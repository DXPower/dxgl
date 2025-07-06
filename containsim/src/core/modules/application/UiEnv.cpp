#include <modules/application/UiEnv.hpp>
#include <modules/application/EventManager.hpp>
#include <modules/application/WindowComponents.hpp>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

using namespace application;

UiEnv::UiEnv(flecs::entity main_window_e) {
    const auto& world = main_window_e.world();
    const auto& main_window = main_window_e.get<dxgl::Window>();

    Rml::SetRenderInterface(&m_rml_renderer);
    Rml::SetSystemInterface(&m_rml_system);

    const auto window_size = main_window.GetSize();
    m_rml_renderer.SetViewport(window_size.x, window_size.y);

    Rml::Initialise();

    auto* rml_context = Rml::CreateContext(
        "main",
        Rml::Vector2i(window_size.x, window_size.y),
        nullptr,
        &m_text_input_handler
    ); 

    if (rml_context == nullptr) {
        throw std::runtime_error("Failed to create RmlUi context");
    }

    auto rml_context_e = world.entity()
        .set<RmlContextHandle>({rml_context})
        .add<RmlMainContext>();

    rml_context->SetDensityIndependentPixelRatio(main_window.GetScale().x);
    Rml::Debugger::Initialise(rml_context);

    if (!Rml::LoadFontFace("res/fonts/LatoLatin-Regular.ttf", true)) {
        throw std::runtime_error("Failed to load RmlUi font face");
    }
    
    world.observer<WindowSize>()
        // .term_at(0).src(main_window_e)
        .event(flecs::OnSet)
        .each([rml_context_e, &main_window, this](flecs::entity, const WindowSize& size) {
            auto& rml_context_handle = rml_context_e.get_mut<RmlContextHandle>();
            rml_context_handle.context->SetDimensions(Rml::Vector2i(size.value.x, size.value.y));
            rml_context_handle.context->SetDensityIndependentPixelRatio(main_window.GetScale().x);

            m_rml_renderer.SetViewport(size.value.x, size.value.y);
        });
}