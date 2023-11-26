#include <services/UiRenderer.hpp>
#include <common/DrawQueues.hpp>
#include <common/FileUtils.hpp>

#include <dxgl/Application.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Uniform.hpp>

#include <array>
#include <iostream>
#include <Ultralight/Ultralight.h>
#include <Ultralight/Listener.h>
#include <AppCore/Platform.h>


using namespace services;
using namespace ultralight;

namespace {
    struct VertexData {
        glm::vec2 local_pos{};
        glm::vec2 tex_pos{};
    };

    constexpr std::array<VertexData, 4> quad_vbo_data{
                //  Pos        Tex
        VertexData{{-1, -1},  {0, 1}},
        VertexData{{ 1, -1},  {1, 1}},
        VertexData{{ 1,  1},  {1, 0}},
        VertexData{{-1,  1},  {0, 0}} 
    };

    struct ViewInfo {
        RefPtr<View> view{};
        RefPtr<Bitmap> bitmap{};
        BitmapSurface* bitmap_surface{};
                
        void Resize(glm::ivec2 size) {
            view->Resize(size.x, size.y);
            bitmap_surface->Resize(size.x, size.y);
            bitmap = bitmap_surface->bitmap();
        }
    };
}

class UiRenderer::Pimpl : public ViewListener, LoadListener {
public:
    Config config{};
    ViewConfig view_config{};

    RefPtr<Renderer> renderer{};
    
    const dxgl::Window* main_window{};
    const dxgl::Window* inspector_window{};

    ViewInfo main_view{};
    ViewInfo inspector_view{};

    // TODO: Once rendering is on different thread, this needs to change
    // ui_texture needs to be owned in dxgl::Draw
    dxgl::Vbo screen_vbo{};
    dxgl::Program screen_program{};

    dxgl::Vao ui_vao{};
    dxgl::Texture ui_texture{};
    dxgl::Draw ui_draw{};
    
    std::optional<dxgl::Vao> inspector_vao{}; // Deferred construction
    dxgl::Texture inspector_texture{};
    dxgl::Draw inspector_draw{};

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

        const auto window_size = main_window.GetSize();
        main_view.view = renderer->CreateView(window_size.x, window_size.y, view_config, nullptr);

        main_view.bitmap_surface = dynamic_cast<BitmapSurface*>(main_view.view->surface());
        main_view.bitmap = main_view.bitmap_surface->bitmap();

        main_view.view->set_load_listener(this);
        main_view.view->set_view_listener(this);
        main_view.view->CreateLocalInspectorView();

        InitDraw(main_window, inspector_window);
    }

    void InitDraw(const dxgl::Window& main_window, const dxgl::Window& inspector_window) {
        screen_program = dxgl::ProgramBuilder()
            .Vert("shaders/framebuffer.vert")
            .Frag("shaders/framebuffer.frag")
            .Link();

        dxgl::Uniform::Set(screen_program, "screen_tex", 0);

        screen_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);

        using namespace dxgl;

        auto vao_builder = VaoAttribBuilder()
            .Group(AttribGroup()
                .Vbo(screen_vbo)
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Components(2)
                    .Multiply(2)
                )
            );

        inspector_window.MakeCurrent();
        inspector_vao = dxgl::Vao{}; // Recreate vao in inspector window
        vao_builder.Apply(inspector_vao.emplace());

        main_window.MakeCurrent();
        vao_builder.Apply(ui_vao);

        ui_draw.prim_type = dxgl::PrimType::TriangleFan;
        ui_draw.num_indices = 4;
        ui_draw.program = screen_program;
        ui_draw.vao_view = ui_vao;

        inspector_draw.prim_type = dxgl::PrimType::TriangleFan;
        inspector_draw.num_indices = 4;
        inspector_draw.program = screen_program;
        inspector_draw.vao_view = *inspector_vao;
    }

    RefPtr<View> OnCreateInspectorView(View*, bool, const String&) override {
        auto window_size = inspector_window->GetSize();
        inspector_view.view = renderer->CreateView(window_size.x, window_size.y, view_config, nullptr);
        inspector_view.view->set_load_listener(this);

        inspector_view.bitmap_surface = dynamic_cast<BitmapSurface*>(inspector_view.view->surface());
        inspector_view.bitmap = inspector_view.bitmap_surface->bitmap();

        return inspector_view.view;
    };
    
    void OnBeginLoading(ultralight::View* caller [[maybe_unused]],
                                uint64_t frame_id [[maybe_unused]],
                                bool is_main_frame [[maybe_unused]],
                                const String& url [[maybe_unused]]
    ) override {
        std::cout << "Begin loading of " << url.utf8().data() << std::endl;
    }
    
    void OnFinishLoading(ultralight::View* caller [[maybe_unused]],
                                uint64_t frame_id [[maybe_unused]],
                                bool is_main_frame [[maybe_unused]],
                                const String& url [[maybe_unused]]
    ) override {
        std::cout << "Finished loading of " << url.utf8().data() << std::endl;
    }
};

UiRenderer::UiRenderer(const dxgl::Window& main_window, const dxgl::Window& inspector_window) {
    m_pimpl = std::make_unique<Pimpl>(main_window, inspector_window);
}

UiRenderer::~UiRenderer() = default;

void UiRenderer::Update() {
    m_pimpl->renderer->Update();
}

void UiRenderer::Render(DrawQueues& draw_queues) const {
    m_pimpl->renderer->Render();

    auto& view_info = m_pimpl->main_view;
    
    void* ui_pixels = view_info.bitmap->LockPixels();
    
    glm::ivec2 ui_size = { view_info.bitmap->width(), view_info.bitmap->height() };
    dxgl::TextureSource source((unsigned char*) ui_pixels, dxgl::TextureFormat::BGRA, ui_size);

    m_pimpl->ui_texture = dxgl::Texture(source);
    view_info.bitmap->UnlockPixels();
    
    m_pimpl->ui_draw.textures.clear();
    m_pimpl->ui_draw.textures.emplace_back(m_pimpl->ui_texture);

    draw_queues.QueueViewedDraw(RenderLayer::Ui, m_pimpl->ui_draw);
}

void UiRenderer::RenderDebug(DrawQueues& draw_queues) const {
    auto& view_info = m_pimpl->inspector_view;

    void* ui_pixels = view_info.bitmap->LockPixels();

    glm::ivec2 ui_size = { view_info.bitmap->width(), view_info.bitmap->height() };
    dxgl::TextureSource source((unsigned char*) ui_pixels, dxgl::TextureFormat::BGRA, ui_size);

    m_pimpl->inspector_texture = dxgl::Texture(source);
    view_info.bitmap->UnlockPixels();
    
    m_pimpl->inspector_draw.textures.clear();
    m_pimpl->inspector_draw.textures.emplace_back(m_pimpl->inspector_texture);

    draw_queues.QueueViewedDraw(RenderLayer::Ui, m_pimpl->inspector_draw);
}

void UiRenderer::LoadHtml(std::string_view path) {
    std::string html = LoadFileToString(path);
    m_pimpl->main_view.view->LoadHTML(html.c_str());
}

void UiRenderer::LoadCss(std::string_view path) {
    std::string css = LoadFileToString(path);
    m_pimpl->config.user_stylesheet = css.c_str();
    
    Platform::instance().set_config(m_pimpl->config);
}

void UiRenderer::LoadUrl(dxtl::cstring_view path) {
    m_pimpl->main_view.view->LoadURL(path.c_str());
}

void UiRenderer::Resize(glm::ivec2 size) {
    m_pimpl->main_view.Resize(size);
}

void UiRenderer::ResizeInspector(glm::ivec2 size) {
    m_pimpl->inspector_view.Resize(size);
}