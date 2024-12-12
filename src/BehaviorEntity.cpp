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
    } else if (type == "invert") {
        return InvertNode::fromDictionary(data);
    } else if (type == "move") {
        return MoveNode::fromDictionary(data);
    } else if (type == "search_for_tile") {
        return SearchForTileNode::fromDictionary(data);
    } else if (type == "search_for_entity") {
        return SearchForEntityNode::fromDictionary(data);
    } else if (type == "get_property") {
        return GetPropertyNode::fromDictionary(data);
    } else if (type == "set_blackboard") {
        return SetBlackboardNode::fromDictionary(data);
    } else {
        UtilityFunctions::printerr("Unknown node type: ", type);
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
        if (outcome == RUNNING) {
            return RUNNING;
        } else if (outcome == FAILURE) {
            return SUCCESS;
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

std::unique_ptr<InvertNode> InvertNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<InvertNode> node = std::make_unique<InvertNode>();
    Dictionary childData = data.get_or_add("child", Dictionary());
    node->child = BehaviorNode::fromDictionary(childData);
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
    return success ? (isRelative ? SUCCESS : RUNNING) : (failWhenBlocked ? FAILURE : SUCCESS);
}

std::unique_ptr<MoveNode> MoveNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<MoveNode> node = std::make_unique<MoveNode>();

    Dictionary target = data.get_or_add("target", Dictionary());
    node->target = BlackboardValue<Vector2>::fromDictionary(target);

    Dictionary speed = data.get_or_add("speed", 1.0);
    node->speed = BlackboardValue<double>::fromDictionary(speed);

    node->isRelative = data.get_or_add("relative", false);
    node->failWhenBlocked = data.get_or_add("fail_when_blocked", false);
    return node;
}

BehaviorNode::Outcome SearchForTileNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    const Vector2i posI = entity.getPosition().round();
    double radius = this->radius.get(entity.blackboard);
    StringName target = this->target.get(entity.blackboard);

    Vector2i result;
    double closestSq = -1;

    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            Vector2i pos = posI + Vector2i(x, y);
            if (!gameState.isInBounds(pos)) { continue; }
            StringName mat = gameState.getTile(pos).material;
            if (mat != target) { continue;}

            Vector2 diff = pos - entity.getPosition();
            double distSquared = diff.length_squared();
            if (distSquared > radius * radius || (closestSq != -1 && distSquared > closestSq)) { continue; }
            if (Math::is_zero_approx(distSquared)) { continue; }

            if (requireLineOfSight && !entity.hasLineOfSightTo(gameState, pos)) {
                continue;
            }

            closestSq = distSquared;
            result = pos;
        }
    }
    if (closestSq == -1) {
        return FAILURE;
    } else {
        if (!resultKey.is_empty()) {
            entity.blackboard[resultKey] = result;
        }
        return SUCCESS;
    }
}

std::unique_ptr<SearchForTileNode> SearchForTileNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<SearchForTileNode> node = std::make_unique<SearchForTileNode>();

    Dictionary target = data.get_or_add("target", Dictionary());
    node->target = BlackboardValue<StringName>::fromDictionary(target, true);

    Dictionary radius = data.get_or_add("radius", 0);
    node->radius = BlackboardValue<int>::fromDictionary(radius);

    node->requireLineOfSight = data.get_or_add("require_line_of_sight", false);
    node->resultKey = data.get_or_add("result_key", "");
    return node;
}

BehaviorNode::Outcome SearchForEntityNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    double radius = this->radius.get(entity.blackboard);
    StringName target = this->target.get(entity.blackboard);

    Vector2 result;
    double closestSq = -1;

    gameState.processNearbyEntities(entity.getPosition(), radius, [&] (Entity& e) {
        if (e.getType() != target) { return; }

        Vector2 diff = e.getPosition() - entity.getPosition();
        double distSquared = diff.length_squared();
        if (distSquared > radius * radius || (closestSq != -1 && distSquared > closestSq)) { return; }
        if (Math::is_zero_approx(distSquared)) { return; }

        if (requireLineOfSight && !entity.hasLineOfSightTo(gameState, e.getPosition())) {
            return;
        }

        closestSq = distSquared;
        result = e.getPosition();
    });

    if (closestSq == -1) {
        return FAILURE;
    } else {
        if (!resultKey.is_empty()) {
            entity.blackboard[resultKey] = result;
        }
        return SUCCESS;
    }
}

std::unique_ptr<SearchForEntityNode> SearchForEntityNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<SearchForEntityNode> node = std::make_unique<SearchForEntityNode>();

    Dictionary target = data.get_or_add("target", Dictionary());
    node->target = BlackboardValue<StringName>::fromDictionary(target, true);

    Dictionary radius = data.get_or_add("radius", 0);
    node->radius = BlackboardValue<double>::fromDictionary(radius);

    node->requireLineOfSight = data.get_or_add("require_line_of_sight", false);
    node->resultKey = data.get_or_add("result_key", "");
    return node;
}

BehaviorNode::Outcome GetPropertyNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    if (property == StringName("position")) {
        entity.blackboard[resultKey] = entity.getPosition();
    } else if (property == StringName("tile")) {
        entity.blackboard[resultKey] = entity.getCurrentTile(gameState).material;
    } else if (property == StringName("type")) {
        entity.blackboard[resultKey] = entity.getType();
    } else {
        UtilityFunctions::printerr("Unknown property: ", property);
        return FAILURE;
    }
    return SUCCESS;
}

std::unique_ptr<GetPropertyNode> GetPropertyNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<GetPropertyNode> node = std::make_unique<GetPropertyNode>();

    node->property = data.get_or_add("property", "");
    node->resultKey = data.get_or_add("result_key", "");
    return node;
}

BehaviorNode::Outcome SetBlackboardNode::process(BehaviorEntity& entity, double delta, GameState& gameState) {
    entity.blackboard[key.get(entity.blackboard)] = value.get(entity.blackboard);
    return SUCCESS;
}

std::unique_ptr<SetBlackboardNode> SetBlackboardNode::fromDictionary(Dictionary& data) {
    std::unique_ptr<SetBlackboardNode> node = std::make_unique<SetBlackboardNode>();

    Dictionary key = data.get_or_add("key", Dictionary());
    node->key = BlackboardValue<StringName>::fromDictionary(key, true);

    Dictionary value = data.get_or_add("value", Dictionary());
    node->value = BlackboardValue<Variant>::fromDictionary(value);
    return node;
}
