#pragma once

#include <common/Logging.hpp>
#include <common/DeducingThisHelper.hpp>
#include <dxfsm/dxfsm.hpp>
#include <string>
#include <vector>
#include <boost/bimap.hpp>
#include <unordered_map>

struct MeceId {
    int id{};
    std::string name{};

    constexpr bool operator==(const MeceId& rhs) const {
        return id == rhs.id;
    }
};

template<>
struct std::hash<MeceId> {
    std::size_t operator()(const MeceId& o) const {
        return std::hash<decltype(o.id)>{}(o.id);
    }
};

class MeceIdRegistry {
    boost::bimap<int, std::string> m_ids{};

public:
    int AddId(int id, std::string name) {
        auto [it, ins] = m_ids.insert(decltype(m_ids)::value_type(id, std::move(name)));

        if (!ins) {
            throw std::runtime_error("Attempt to register an id that already exists");
        }

        return id;
    }

    int AddId(std::string name) {
        return AddId(static_cast<int>(m_ids.size()), std::move(name));
    }

    int GetOrAddId(const std::string& name) {
        auto it = m_ids.right.find(name);

        if (it != m_ids.right.end()) {
            return it->second;
        } else {
            return AddId(name);
        }
    }

    auto& GetIds() const { return m_ids; }
};

class MeceSubStates : public MeceIdRegistry {
public:
    constexpr static int Inactive = 0;
    constexpr static int Idle = 1;

    MeceSubStates() {
        AddId("Inactive");
        AddId("Idle");
    }
};

class MeceSubEvents : public MeceIdRegistry {
public:
    constexpr static int EnteringSubFsm = 0;
    constexpr static int ExitingSubFsm = 1;

    MeceSubEvents() {
        AddId("EnteringSubFsm");
        AddId("ExitingSubFsm");
    }
};

class MeceFsm;

// MECE = "Mutually Exclusive, Collectively Exhaustive"
class MeceSubFsm {
public:
    using State = dxfsm::State<int>;
    using Event = dxfsm::Event<int>;
    using FSM = dxfsm::FSM<int, int>;

private:
    MeceFsm* m_parent{};
    FSM m_fsm{};
    MeceSubStates m_state_info{};
    MeceSubEvents m_event_info{};

    std::string m_name{};
    int m_id{};
protected:
    logging::Logger m_logger;

public:
    MeceSubFsm(const std::string& name);
    virtual ~MeceSubFsm() = default;

    bool IsActive() const;
    void MakeActive();
    void MakeInactive();
    int  GetCurState() const;
    std::string_view GetName() const;
    int GetId() const;

    auto& GetStateInfo(this auto&& self) {
        return self.m_state_info;
    }

    auto& GetEventInfo(this auto&& self) {
        return self.m_event_info;
    }

    template<typename Self>
    auto& GetFsm(this Self&& self) {
        return self.m_fsm;
    }
    
protected:
    void InitializeDefaultStates();
    void AddExitTransitionsToAllStates();

    virtual void OnStateChanged(
        const FSM&, 
        std::optional<State> from [[maybe_unused]],
        State to [[maybe_unused]],
        const Event&) { };

    virtual State StateIdle(FSM& fsm, int state_id) = 0;
    virtual State StateInactive(FSM& fsm, int state_id);

    template<typename F>
    requires std::is_same_v<std::invoke_result_t<F, FSM&, int>, State>
    State AddState(std::string name, F&& coro) {
        int new_id = m_state_info.AddId(std::move(name));
        return coro(m_fsm, new_id);
    }

private:
    void SetParent(MeceFsm& parent);
    void SetId(int id);
    void LogTransition(std::optional<State> from, State to, const Event& e);

    friend class MeceFsm;
};

class MeceFsm final {
    logging::Logger m_logger;
    std::vector<std::unique_ptr<MeceSubFsm>> m_subs{};
    std::unordered_map<int, MeceSubFsm*> m_id_to_sub{};
    // TODO: Enable transparent comparison of std::string_view
    std::unordered_map<std::string, MeceSubFsm*> m_name_to_sub{};
    MeceSubFsm* m_active_sub{};

public:
    MeceFsm(std::string name);
    MeceFsm(const MeceFsm&) = delete;
    MeceFsm& operator=(const MeceFsm&) = delete;

    int AddSubFsm(std::unique_ptr<MeceSubFsm>&& sub_fsm);

    template<typename Self>
    auto GetSubFsm(this Self&& self, int id) -> PropagateConst<Self, MeceSubFsm>* {
        auto it = self.m_id_to_sub.find(id);

        if (it != self.m_id_to_sub.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }
    
    template<typename Self>
    auto GetSubFsm(this Self&& self, std::string_view id) -> PropagateConst<Self, MeceSubFsm>* {
        auto it = self.m_name_to_sub.find(std::string(id));

        if (it != self.m_name_to_sub.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    template<typename Self>
    auto GetActiveSubFsm(this Self&& self) -> PropagateConst<Self, MeceSubFsm>* {
        return self.m_active_sub;
    }

private:
    void NotifySubFsmMadeActive(MeceSubFsm& sub);
    void NotifySubFsmMadeInactive(MeceSubFsm& sub);

    friend class MeceSubFsm;
};
