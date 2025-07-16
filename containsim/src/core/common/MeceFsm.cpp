#include <common/MeceFsm.hpp>
#include <cassert>

MeceSubFsm::MeceSubFsm(const std::string& name)
    : m_name(name)
    , m_logger(logging::CreateLogger(name)) { }

void MeceSubFsm::InitializeDefaultStates() {
    auto inactive_state = StateInactive(m_fsm, MeceSubStates::Inactive);
    StateIdle(m_fsm, MeceSubStates::Idle);
    
    m_fsm.SetCurrentState(inactive_state);

    m_fsm.AddTransition(MeceSubStates::Idle, MeceSubEvents::ExitingSubFsm, MeceSubStates::Inactive);
    m_fsm.AddTransition(MeceSubStates::Inactive, MeceSubEvents::EnteringSubFsm, MeceSubStates::Idle);

    m_fsm.SetTransitionObserver([this](const FSM& fsm, std::optional<State> from, State to, const Event& e) {
        LogTransition(from, to, e);

        // Detect if we are just sending the enter/exit event
        if (e == MeceSubEvents::EnteringSubFsm) {
            m_parent->NotifySubFsmMadeActive(*this);
        } else if (e == MeceSubEvents::ExitingSubFsm) {
            m_parent->NotifySubFsmMadeInactive(*this);
        }

        // Detect if we enter the active state for the first time or if we enter inactive state
        if (to.Id() == MeceSubStates::Idle && (!from.has_value() || from->Id() == MeceSubStates::Inactive)) {
            m_parent->NotifySubFsmMadeActive(*this);
        } else if (to.Id() == MeceSubStates::Inactive) {
            m_parent->NotifySubFsmMadeInactive(*this);
        }

        OnStateChanged(fsm, from, to, e);
    });
}

void MeceSubFsm::AddExitTransitionsToAllStates() {
    for (State s : m_fsm.GetStates()) {
        if (s.Id() == MeceSubStates::Inactive)
            continue;

        m_fsm.AddTransition(s.Id(), MeceSubEvents::ExitingSubFsm, MeceSubStates::Inactive);
    }
}

bool MeceSubFsm::IsActive() const {
    auto s = m_fsm.GetCurrentState();
    return s.value().Id() != MeceSubStates::Inactive;
}

void MeceSubFsm::MakeActive() {
    // Do nothing if already active
    if (IsActive())
        return;

    m_fsm.InsertEvent(MeceSubEvents::EnteringSubFsm);
    m_parent->NotifySubFsmMadeActive(*this);
}

void MeceSubFsm::MakeInactive() {
    // Do nothing if not already active
    if (!IsActive())
        return;

    m_fsm.SetCurrentState(MeceSubStates::Inactive);
    m_fsm.InsertEvent(MeceSubEvents::ExitingSubFsm);
    m_parent->NotifySubFsmMadeInactive(*this);
}

int MeceSubFsm::GetCurState() const {
    auto s = m_fsm.GetCurrentState();

    if (s.has_value()) {
        return s->Id();
    } else {
        return -1;
    }
}

std::string_view MeceSubFsm::GetName() const {
    return m_name;
}

int MeceSubFsm::GetId() const {
    return m_id;
}

auto MeceSubFsm::StateInactive(FSM& fsm, int state_id [[maybe_unused]]) -> State {
    while (true) {
        co_await fsm.IgnoreEvent();
    }
}

void MeceSubFsm::SetParent(MeceFsm& parent) {
    m_parent = &parent;
}

void MeceSubFsm::SetId(int id) {
    m_id = id;
}


void MeceSubFsm::LogTransition(std::optional<State> from, State to, const Event& e) {
    if (!m_logger.should_log(spdlog::level::info))
        return;

    std::string e_str{};

    if (!e.Empty()) {
        const auto& events = m_event_info.GetIds();
        auto it = events.left.find(e.GetId());

        if (it != events.left.end()) {
            e_str = std::format("(Event: '{}')", it->second);
        } else {
            e_str = "(Unknown event)";
        }
    }

    const auto& states = m_state_info.GetIds();
    std::string from_str{};

    if (from.has_value()) {
        auto it = states.left.find(from->Id());

        if (it != states.left.end()) {
            from_str = std::format("from state '{}' ", it->second);
        } else {
            from_str = "from an unknown state ";
        }
    }

    std::string to_str{};
    {
        auto it = states.left.find(to.Id());

        if (it != states.left.end()) {
            to_str = std::format("state '{}'", it->second);
        } else {
            to_str = std::format("an unknown state");
        }
    }

    m_logger.info("Transitioning {}to {}. {}", from_str, to_str, e_str);
}

MeceFsm::MeceFsm(std::string name) : m_logger(logging::CreateLogger(std::move(name))) { }

int MeceFsm::AddSubFsm(std::unique_ptr<MeceSubFsm>&& sub_fsm) {
    auto new_id = static_cast<int>(m_subs.size());

    sub_fsm->SetParent(*this);
    sub_fsm->SetId(new_id);
    
    m_id_to_sub.emplace(new_id, sub_fsm.get());
    m_name_to_sub.emplace(sub_fsm->GetName(), sub_fsm.get());
    m_subs.push_back(std::move(sub_fsm));

    return new_id;
}

void MeceFsm::NotifySubFsmMadeActive(MeceSubFsm& sub) {
    // Filter out repeated attempts
    if (m_active_sub == &sub)
        return;

    if (m_active_sub != nullptr) {
        m_active_sub->MakeInactive();
    }

    assert(m_active_sub == nullptr);
    m_active_sub = &sub;
}

void MeceFsm::NotifySubFsmMadeInactive(MeceSubFsm& sub) {
    if (m_active_sub != &sub) {
        m_logger.warn("Attempt to make sub-FSM '{}' inactive when it wasn't active", sub.GetName());
    }

    m_active_sub = nullptr;
}