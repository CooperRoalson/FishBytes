#ifndef BEHAVIORENTITY_H
#define BEHAVIORENTITY_H

#include "godot_includes.h"
#include "GameState.h"
#include "Entities.h"

// Forward declaration
class BehaviorEntity;

class BehaviorNode {
protected:
    BehaviorNode() = default;

    virtual String toString() = 0;
    virtual void printChildren(int indent) {};

public:
    enum Outcome {
        SUCCESS,
        FAILURE,
        RUNNING
    };

    virtual ~BehaviorNode() {}

    virtual Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) = 0;
    void print(int indent = 0);

    static std::unique_ptr<BehaviorNode> fromDictionary(Dictionary& data);
};

struct BehaviorTree : public Resource {
    std::unique_ptr<BehaviorNode> root;
    Dictionary defaultBlackboard;

    static Ref<BehaviorTree> parseBehaviorTree(Dictionary& config);
};

struct BehaviorProperties : EntityProperties {
    Ref<BehaviorTree> tree;
    Dictionary defaultBlackboardOverrides;
};

// *---------- BehaviorEntity ----------*

class BehaviorEntity : public Entity {
public:
    Dictionary blackboard;

    BehaviorEntity(StringName type, Ref<EntityProperties> properties, Vector2 position);

    void process(double delta, GameState& gameState) override;
};

// *---------- BlackboardValue ----------*

template <typename T>
class BlackboardValue {
    String key;
    std::optional<T> value;
    bool isBlackboard;

public:
    static BlackboardValue fromKey(String key) {
        BlackboardValue v{};
        v.isBlackboard = true;
        v.key = key;
        return v;
    }

    static BlackboardValue fromValue(const T& value) {
        BlackboardValue v{};
        v.isBlackboard = false;
        v.value = value;
        return v;
    }

    T get(Dictionary& blackboard) {
        if (isBlackboard) {
            if (!blackboard.has(key)) {
                blackboard[key] = T();
            }
            return blackboard[key];
        } else {
            return value.value();
        }
    }

    String toString() {
        return isBlackboard ? "<blackboard: '" + key + "'>" : String("%s") % Array::make(value.value());
    }

    static BlackboardValue fromDictionary(Dictionary& data, bool isStr = false) {
        if (data.has("key")) {
            return fromKey(data["key"]);
        } else if (data.has("value")) {
            Variant v = data["value"];
            return fromValue(isStr ? v : UtilityFunctions::str_to_var(v));
        } else {
            UtilityFunctions::printerr("BlackboardValue missing data");
            return fromValue(T());
        }
    }
};

// *---------- Behavior Nodes ----------*

class NullNode : public BehaviorNode {
    String toString() override { return "NullNode"; }

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override {
        UtilityFunctions::printerr("A NullNode is being run");
        return FAILURE;
    }
};

class SequenceNode : public BehaviorNode {
    static int sequenceCounter;
    std::vector<std::unique_ptr<BehaviorNode>> children{};
    const int id = 0;

    String toString() override { return "SequenceNode"; }

    void printChildren(int indent) override {
        for (auto& child : children) {
            child->print(indent);
        }
    }

public:
    SequenceNode() : id(sequenceCounter++) {}

    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SequenceNode> fromDictionary(Dictionary& data);
};

class SelectorNode : public BehaviorNode {
    static int selectorCounter;
    std::vector<std::unique_ptr<BehaviorNode>> children{};
    const int id;

    String toString() override { return "SelectorNode"; }

    void printChildren(int indent) override {
        for (auto& child : children) {
            child->print(indent);
        }
    }

public:
    SelectorNode() : id(selectorCounter++) {}

    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SelectorNode> fromDictionary(Dictionary& data);
};

class RepeatWhileNode : public BehaviorNode {
    static constexpr int MAX_LOOPS_PER_FRAME = 10;

    std::unique_ptr<BehaviorNode> child = nullptr;

    String toString() override { return "RepeatWhileNode";}
    void printChildren(int indent) override { child->print(indent);}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<RepeatWhileNode> fromDictionary(Dictionary& data);
};

class ConstantNode : public BehaviorNode {
    Outcome outcome;

    String toString() override { return String("ConstantNode(%s)") % Array::make(outcome == SUCCESS ? "SUCCESS" : "FAILURE");}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override {
        return outcome;
    }

    static std::unique_ptr<ConstantNode> fromDictionary(Dictionary& data);
};

class InvertNode : public BehaviorNode {
    std::unique_ptr<BehaviorNode> child = nullptr;

    String toString() override { return "InvertNode"; }
    void printChildren(int indent) override { child->print(indent); }

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override {
        Outcome outcome = child->process(entity, delta, gameState);
        return outcome == RUNNING ? RUNNING : outcome == SUCCESS ? FAILURE : SUCCESS;
    }

    static std::unique_ptr<InvertNode> fromDictionary(Dictionary& data);
};

class MoveNode : public BehaviorNode {
    BlackboardValue<Vector2> target;
    BlackboardValue<double> speed;
    bool isRelative;
    bool failWhenBlocked;

    String toString() override { return String("MoveNode(%s, %s, %s)") % Array::make(target.toString(), speed.toString(), isRelative ? "true" : "false");}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<MoveNode> fromDictionary(Dictionary& data);
};

class SearchForTileNode : public BehaviorNode {
    BlackboardValue<StringName> target;
    BlackboardValue<int> radius;
    bool requireLineOfSight;
    String resultKey;

    String toString() override { return String("SearchForTileNode(%s, %s, %s)") % Array::make(target.toString(), radius.toString(), requireLineOfSight ? "true" : "false"); }

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SearchForTileNode> fromDictionary(Dictionary& data);
};

class SearchForEntityNode : public BehaviorNode {
    BlackboardValue<StringName> target;
    BlackboardValue<double> radius;
    bool requireLineOfSight;
    String resultKey;

    String toString() override { return String("SearchForEntityNode(%s, %s, %s)") % Array::make(target.toString(), radius.toString(), requireLineOfSight ? "true" : "false"); }

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SearchForEntityNode> fromDictionary(Dictionary& data);
};

class GetPropertyNode : public BehaviorNode {
    StringName property;
    String resultKey;

    String toString() override { return String("GetPropertyNode(%s)") % Array::make(property);}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<GetPropertyNode> fromDictionary(Dictionary& data);
};

class SetBlackboardNode : public BehaviorNode {
    BlackboardValue<StringName> key;
    BlackboardValue<Variant> value;

    String toString() override { return String("SetBlackboardNode(%s, %s)") % Array::make(key.toString(), value.toString());}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SetBlackboardNode> fromDictionary(Dictionary& data);
};

#endif //BEHAVIORENTITY_H
