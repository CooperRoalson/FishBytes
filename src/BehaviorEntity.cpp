#include "BehaviorEntity.h"

BehaviorEntity::BehaviorEntity(StringName type, Ref<EntityProperties> properties, Vector2 position) : Entity(type, properties, position) {
    auto* props = Object::cast_to<BehaviorProperties>(properties.ptr());

    blackboard = props->tree->defaultBlackboard.duplicate();

    Array keys = props->defaultBlackboardOverrides.keys();
    for (int i = 0; i < keys.size(); i++) {
        String key = keys[i];
        blackboard[key] = props->defaultBlackboardOverrides[key];
    }
}

void BehaviorEntity::process(double delta, GameState& gameState) {
    auto* props = Object::cast_to<BehaviorProperties>(properties.ptr());

    static bool done = false;
    if (!done) {
        props->tree->root->print();
        done = true;
    }

    BehaviorNode::Outcome outcome = props->tree->root->process(*this, delta, gameState);
    if (outcome != BehaviorNode::RUNNING) {
        dead = true;
    }
}

void BehaviorNode::print(int indent) {
    for (int i = 0; i < indent - 1; i++) {
        UtilityFunctions::printraw("|   ");
    }
    if (indent > 0) {
        UtilityFunctions::printraw("|---");
    }

    UtilityFunctions::printraw(toString(), "\n");
    printChildren(indent + 1);
}

Ref<BehaviorTree> BehaviorTree::parseBehaviorTree(Dictionary& config) {
    Ref<BehaviorTree> tree = memnew(BehaviorTree);
    Dictionary rootData = config.get_or_add("root", Dictionary());

    tree->defaultBlackboard = config.get_or_add("blackboard", Dictionary());
    tree->root = BehaviorNode::fromDictionary(rootData);

    return tree;
}

std::unique_ptr<BehaviorNode> BehaviorNode::fromDictionary(Dictionary& data) {
    String type = data.get_or_add("type", "");
    if (type == "sequence") {
        return SequenceNode::fromDictionary(data);
    } else if (type == "selector") {
        return SelectorNode::fromDictionary(data);
    } else if (type == "repeat_while") {
        return RepeatWhileNode::fromDictionary(data);
    } else if (type == "constant") {
        return ConstantNode::fromDictionary(data);
    } else if (type == "move") {
        return MoveNode::fromDictionary(data);
    }else {
        return std::make_unique<NullNode>();
    }
}

BehaviorNode::Outcome SequenceNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    while (currentChild < children.size()) {
        Outcome outcome = children[currentChild]->process(entity, delta, gameState);
        if (outcome == RUNNING) {
            return RUNNING;
        } else if (outcome == FAILURE) {
            currentChild = 0;
            return FAILURE;
        }
        currentChild++;
    }
    currentChild = 0;
    return SUCCESS;
}

std::unique_ptr<SequenceNode> SequenceNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<SequenceNode> node = std::make_unique<SequenceNode>();
    Array childrenData = data.get_or_add("children", Array());
    for (int i = 0; i < childrenData.size(); i++) {
        Dictionary childData = childrenData[i];
        node->children.push_back(BehaviorNode::fromDictionary(childData));
    }
    return node;
}

BehaviorNode::Outcome SelectorNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    while (currentChild < children.size()) {
        Outcome outcome = children[currentChild]->process(entity, delta, gameState);
        if (outcome == RUNNING) {
            return RUNNING;
        } else if (outcome == SUCCESS) {
            currentChild = 0;
            return SUCCESS;
        }
        currentChild++;
    }
    currentChild = 0;
    return FAILURE;
}

std::unique_ptr<SelectorNode> SelectorNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<SelectorNode> node = std::make_unique<SelectorNode>();
    Array childrenData = data.get_or_add("children", Array());
    for (int i = 0; i < childrenData.size(); i++) {
        Dictionary childData = childrenData[i];
        node->children.push_back(BehaviorNode::fromDictionary(childData));
    }
    return node;
}

BehaviorNode::Outcome RepeatWhileNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    for (int i = 0; i < MAX_LOOPS_PER_FRAME; i++) {
        Outcome outcome = child->process(entity, delta, gameState);
        if (outcome == FAILURE || outcome == RUNNING) {
            return outcome;
        }
    }
    return RUNNING;
}

std::unique_ptr<RepeatWhileNode> RepeatWhileNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<RepeatWhileNode> node = std::make_unique<RepeatWhileNode>();
    Dictionary childData = data.get_or_add("child", Dictionary());
    node->child = BehaviorNode::fromDictionary(childData);
    return node;
}

std::unique_ptr<ConstantNode> ConstantNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<ConstantNode> node = std::make_unique<ConstantNode>();
    String outcome = data.get_or_add("outcome", "SUCCESS");
    node->outcome = outcome == "FAILURE" ? FAILURE : SUCCESS;
    return node;
}

BehaviorNode::Outcome MoveNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    Vector2 dest = target.get(entity.blackboard);
    Vector2 dir = isRelative ? dest : dest - entity.getPosition();
    if (dir.is_zero_approx()) {
        return SUCCESS;
    }
    dir.normalize();
    bool success = entity.move(dir * speed.get(entity.blackboard) * delta, gameState);
    return success ? RUNNING : FAILURE;
}

std::unique_ptr<MoveNode> MoveNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<MoveNode> node = std::make_unique<MoveNode>();

    Dictionary target = data.get_or_add("target", Dictionary());
    node->target = BlackboardValue<Vector2>::fromDictionary(target);

    Dictionary speed = data.get_or_add("speed", 1.0);
    node->speed = BlackboardValue<double>::fromDictionary(speed);

    node->isRelative = data.get_or_add("relative", false);
    return node;
}
