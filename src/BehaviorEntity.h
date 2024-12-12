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

    T& get(Dictionary& blackboard) {
        if (isBlackboard) {
            if (!blackboard.has(key)) {
                blackboard[key] = T();
            }
            return (T&) blackboard[key];
        } else {
            return value.value();
        }
    }

    String toString() {
        return isBlackboard ? "<blackboard: '" + key + "'>" : String("%s") % Array::make(value.value());
    }

    static BlackboardValue fromDictionary(Dictionary& data) {
        if (data.has("key")) {
            return fromKey(data["key"]);
        } else  if (!data.has("value")) {
            UtilityFunctions::printerr("BlackboardValue missing data");
            return fromValue(T());
        } else {
            return fromValue(UtilityFunctions::str_to_var(data["value"]));
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
    std::vector<std::unique_ptr<BehaviorNode>> children{};
    int currentChild = 0;

    String toString() override { return "SequenceNode"; }

    void printChildren(int indent) override {
        for (auto& child : children) {
            child->print(indent);
        }
    }

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<SequenceNode> fromDictionary(Dictionary& data);
};

class SelectorNode : public BehaviorNode {
    std::vector<std::unique_ptr<BehaviorNode>> children{};
    int currentChild = 0;

    String toString() override { return "SelectorNode"; }

    void printChildren(int indent) override {
        for (auto& child : children) {
            child->print(indent);
        }
    }

public:
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

class MoveNode : public BehaviorNode {
    BlackboardValue<Vector2> target;
    BlackboardValue<double> speed;
    bool isRelative;

    String toString() override { return String("MoveNode(%s, %s)") % Array::make(target.toString(), speed.toString());}

public:
    Outcome process(BehaviorEntity& entity, double delta, GameState& gameState) override;
    static std::unique_ptr<MoveNode> fromDictionary(Dictionary& data);
};


#endif //BEHAVIORENTITY_H
