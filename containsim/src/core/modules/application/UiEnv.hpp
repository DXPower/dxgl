#pragma once

#include <dxgl/Application.hpp>
#include <modules/application/ApplicationEvents.hpp>

#include <RmlUi_Backends/RmlUi_Backend.h>
#include <RmlUi_Backends/RmlUi_Renderer_GL3.h>
#include <RmlUi_Backends/RmlUi_Platform_GLFW.h>
#include <RmlUi/Core/TextInputHandler.h>
#include <RmlUi/Core/Context.h>

#include <flecs.h>

namespace application {
    struct RmlMainContext { };
    struct RmlContextHandle {
        Rml::Context* context{};
    };
    
    struct UiTextInputHandler final : Rml::TextInputHandler {
        bool m_text_input_active{};

        void OnActivate(Rml::TextInputContext* input_context [[maybe_unused]]) override {
            m_text_input_active = true;
        }

        void OnDeactivate(Rml::TextInputContext* input_context [[maybe_unused]]) override {
            m_text_input_active = false;
        }
    };

    class UiEnv {
        // TODO: Pimpl this
        RenderInterface_GL3 m_rml_renderer{};
        SystemInterface_GLFW m_rml_system{};
        UiTextInputHandler m_text_input_handler{};

    public:
        UiEnv(flecs::entity main_window_e);

        Rml::RenderInterface& GetRenderInterface() { return m_rml_renderer; }
        const Rml::RenderInterface& GetRenderInterface() const { return m_rml_renderer; }

        bool IsTextInputActive() const {
            return m_text_input_handler.m_text_input_active;
        };

    private:
        void OnWindowResize(const WindowSizeChangedEvent& e);
    };
}