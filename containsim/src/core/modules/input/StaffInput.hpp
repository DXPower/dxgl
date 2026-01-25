#pragma once

#include <modules/application/EventManager.hpp>
#include <dxfsm/dxfsm.hpp>
#include <common/ActionChain.hpp>
#include <common/Logging.hpp>
#include <common/EventCommandable.hpp>

#include <modules/rendering/Camera.hpp>
#include <common/MeceFsm.hpp>

#include <modules/core/Staff.hpp>

namespace input {
    class StaffInput final 
        : public EventCommandable<StaffInput>
        , public MeceSubFsm
    {
    public:
        struct StateId {
            inline static int AddStaff{};
        };

        struct EventId {
            inline static int ExitMode{};
            inline static int SelectStaffType{};
            inline static int Action{};
        };
        
        ActionProducer uncaptured_actions{};

    private:
        application::EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};

    public:
        StaffInput(application::EventManager& em, const rendering::Camera& cam);

        void SelectAddStaff();
        void SelectViewStaff();
        void SelectStaffType(core::StaffType);
        void ExitMode();

    private:
        State StateIdle(FSM& fsm, int) override;
        State StateDemarcation(FSM& fsm, int);

        std::optional<glm::vec2> ScreenToWorldPos(glm::vec2 screen_pos) const;
        
        void OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) override;
    };

    using StaffInputCommand = Command<StaffInput>;
}