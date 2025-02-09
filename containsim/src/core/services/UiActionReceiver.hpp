#pragma once

#include <common/ActionChain.hpp>
#include <RmlUi/Core/Context.h>
#include <dxtl/overloaded.hpp>
#include <dxgl/Application.hpp>
#include <RmlUi_Backends/RmlUi_Platform_GLFW.h>
#include <RmlUi/Core/TextInputHandler.h>

namespace services {
    class UiActionReceiver : public ActionConsumer, public Rml::TextInputHandler {
        Rml::Context* m_context{};
        const dxgl::Window* m_window{};
        bool m_text_input_active{};

    public:
        ActionProducer uncaptured_actions{};

        UiActionReceiver(const dxgl::Window& window) : m_window(&window) { }

        void SetContext(Rml::Context& context) {
            m_context = &context;
        }

        void Consume(Action&& action) override {
            bool consumed = std::visit(dxtl::overloaded {
                [&](const KeyPress& press) {
                    auto dir = press.dir == ButtonDir::Up ? GLFW_RELEASE : GLFW_PRESS;
                    bool c = !RmlGLFW::ProcessKeyCallback(m_context, press.key, dir, press.mods);
                    c |= m_text_input_active;
                    return c;
                },
                [&](const TextInput& text) {
                    bool c = !RmlGLFW::ProcessCharCallback(m_context, text.codepoint);
                    c |= m_text_input_active;
                    return c;
                },
                [&](const MouseMove& move) {
                    bool c = !RmlGLFW::ProcessCursorPosCallback(m_context, m_window->GetGlfwWindow(), move.to.x, move.to.y, 0);
                    c |= m_context->IsMouseInteracting();
                    return c;
                },
                [&](const MouseClick& click) {
                    bool c = !RmlGLFW::ProcessMouseButtonCallback(m_context, click.button, click.dir == ButtonDir::Up ? GLFW_RELEASE : GLFW_PRESS, click.mods);
                    c |= m_context->IsMouseInteracting();
                    return c;
                },
                [&](const ScrollInput& scroll) {
                    bool c = !RmlGLFW::ProcessScrollCallback(m_context, scroll.amount.x, scroll.amount.y, 0);
                    c |= m_context->IsMouseInteracting();
                    return c;
                }
            }, action.data);

            if (!consumed) {
                uncaptured_actions.Send(std::move(action));
            }
        }

        void OnActivate(Rml::TextInputContext* input_context) override {
            m_text_input_active = true;
        }

        void OnDeactivate(Rml::TextInputContext* input_context) override {
            m_text_input_active = false;
        }
    };
}