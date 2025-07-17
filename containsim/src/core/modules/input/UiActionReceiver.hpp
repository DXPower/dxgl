#pragma once

#include <modules/application/Application.hpp>
#include <common/ActionChain.hpp>
#include <RmlUi/Core/Context.h>
#include <dxtl/overloaded.hpp>
#include <dxgl/Application.hpp>
#include <RmlUi_Backends/RmlUi_Platform_GLFW.h>
#include <RmlUi/Core/TextInputHandler.h>
#include <RmlUi/Core/Element.h>

#include <flecs.h>

namespace input {
    class UiActionReceiver : public ActionConsumer {
        const dxgl::Window* m_window{};
        const application::UiEnv* m_ui_env{};
        Rml::Context* m_context{};

    public:
        ActionProducer uncaptured_actions{};

        UiActionReceiver(flecs::world& world) {
            m_window = &world.query<application::MainWindow>()
                .first().get<dxgl::Window>();

            m_ui_env = &world.get<application::UiEnv>();

            m_context = world.query<application::RmlMainContext>()
                .first().get<application::RmlContextHandle>().context;
        }

        void Consume(Action&& action) override {
            auto IsMousePassthrough = [&] -> bool {
                auto* elem = m_context->GetHoverElement();
                return elem != nullptr && elem->GetTagName() != "body";
            };

            bool consumed = std::visit(dxtl::overloaded {
                [&, this](const KeyPress& press) {
                    auto dir = press.dir == ButtonDir::Up ? GLFW_RELEASE : GLFW_PRESS;
                    bool c = !RmlGLFW::ProcessKeyCallback(m_context, press.key, dir, press.mods);
                    c |= m_ui_env->IsTextInputActive();
                    return c;
                },
                [&, this](const TextInput& text) {
                    bool c = !RmlGLFW::ProcessCharCallback(m_context, text.codepoint);
                    c |= m_ui_env->IsTextInputActive();
                    return c;
                },
                [&](const MouseMove& move) {
                    RmlGLFW::ProcessCursorPosCallback(m_context, m_window->GetGlfwWindow(), move.to.x, move.to.y, 0);
                    return IsMousePassthrough();
                },
                [&](const MouseClick& click) {
                    RmlGLFW::ProcessMouseButtonCallback(m_context, click.button, click.dir == ButtonDir::Up ? GLFW_RELEASE : GLFW_PRESS, click.mods);
                    return IsMousePassthrough();
                },
                [&](const ScrollInput& scroll) {
                    RmlGLFW::ProcessScrollCallback(m_context, scroll.amount.x, scroll.amount.y, 0);
                    return IsMousePassthrough();

                }
            }, action.data);

            if (!consumed) {
                uncaptured_actions.Send(std::move(action));
            }
        }
    };
}