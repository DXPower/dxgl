#include <services/UiContainer.hpp>

#include <Ultralight/Ultralight.h>
#include <Ultralight/Listener.h>
#include <AppCore/Platform.h>

using namespace services;
using namespace ultralight;

class UiContainer::Pimpl : ViewListener {
public:
    RefPtr<Renderer> renderer{};
    Config config{};
    ViewConfig view_config{};
    
    const dxgl::Window* main_window{};
    const dxgl::Window* inspector_window{};

    std::unique_ptr<UiView> main_view{};
    std::unique_ptr<UiView> inspector_view{};

    Pimpl(const dxgl::Window& main_window, const dxgl::Window& inspector_window) {
        this->main_window = &main_window;
        this->inspector_window = &inspector_window;

        auto& platform = Platform::instance();
        platform.set_config(config);
        platform.set_file_system(GetPlatformFileSystem("./res/ui"));
        platform.set_logger(GetDefaultLogger("ultralight.log"));
        platform.set_font_loader(GetPlatformFontLoader());

        renderer = Renderer::Create();
        view_config.is_accelerated = false;
        view_config.is_transparent = true;
        view_config.initial_device_scale = main_window.GetScale().x;

        const auto window_size = main_window.GetSize();
    
        main_view = std::make_unique<UiView>(
            main_window, 
            renderer->CreateView(window_size.x, window_size.y, view_config, nullptr)
        );
        main_view->GetView()->set_view_listener(this);
        main_view->GetView()->CreateLocalInspectorView();

        main_window.MakeCurrent();
    }

    RefPtr<View> OnCreateInspectorView(View*, bool, const String&) override {
        inspector_window->MakeCurrent();

        const auto window_size = inspector_window->GetSize();

        inspector_view = std::make_unique<UiView>(
            *inspector_window,
            renderer->CreateView(window_size.x, window_size.y, view_config, nullptr)
        );

        return inspector_view->GetView();
    };
};

void UiContainer::PimplDeleter::operator()(Pimpl* ptr) const {
    delete ptr;
}

UiContainer::UiContainer(const dxgl::Window& main_window, const dxgl::Window& inspector_window)
    : m_pimpl(new Pimpl(main_window, inspector_window)) { }

UiContainer::~UiContainer() = default;

void UiContainer::Update() {
    m_pimpl->renderer->Update();
}

void UiContainer::Render() {
    m_pimpl->renderer->Render();
}

UiView& UiContainer::GetMainView() {
    return *m_pimpl->main_view;
}

const UiView& UiContainer::GetMainView() const {
    return *m_pimpl->main_view;
}

UiView& UiContainer::GetInspectorView() {
    return *m_pimpl->inspector_view;
}

const UiView& UiContainer::GetInspectorView() const {
    return *m_pimpl->inspector_view;
}
