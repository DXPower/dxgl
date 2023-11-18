#include <services/UiRenderer.hpp>
#include <common/DrawQueues.hpp>
#include <common/FileUtils.hpp>

#include <dxgl/Application.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Uniform.hpp>

#include <array>
#include <Ultralight/Ultralight.h>
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
}
class UiRenderer::Pimpl {
public:
    Config config{};
    ViewConfig view_config{};

    RefPtr<Renderer> renderer{};
    RefPtr<View> view{};
    BitmapSurface* bitmap_surface{};
    RefPtr<Bitmap> bitmap{};

    // TODO: Once rendering is on different thread, this needs to change
    // ui_texture needs to be owned in dxgl::Draw
    dxgl::Program ui_program{};
    dxgl::Draw ui_draw{};
    dxgl::Texture ui_texture{};

    Pimpl() {
        auto& platform = Platform::instance();
        platform.set_config(config);
        platform.set_file_system(GetPlatformFileSystem("./res/ui"));
        platform.set_logger(GetDefaultLogger("ultralight.log"));
        platform.set_font_loader(GetPlatformFontLoader());

        renderer = Renderer::Create();
        view_config.is_accelerated = false;
        view_config.is_transparent = true;

        const auto window_size = dxgl::Application::GetWindowSize();
        view = renderer->CreateView(window_size.x, window_size.y, view_config, nullptr);

        bitmap_surface = dynamic_cast<BitmapSurface*>(view->surface());
        bitmap = bitmap_surface->bitmap();

        InitDraw();
    }

    void InitDraw() {
        ui_program = dxgl::ProgramBuilder()
            .Vert("shaders/framebuffer.vert")
            .Frag("shaders/framebuffer.frag")
            .Link();

        dxgl::Uniform::Set(ui_program, "screen_tex", 0);

        ui_draw.prim_type = dxgl::PrimType::TriangleFan;
        ui_draw.num_indices = 4;

        ui_draw.program = ui_program;
        ui_draw.vbo_storage.emplace_back().Upload(quad_vbo_data, dxgl::BufferUsage::Static);
        ui_draw.vao_storage.emplace();

        using namespace dxgl;

        VaoAttribBuilder()
            .Group(AttribGroup()
                .Vbo(ui_draw.vbo_storage.back())
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Components(2)
                    .Multiply(2)
                )
            )
            .Apply(*ui_draw.vao_storage);
    }
};

UiRenderer::UiRenderer() {
    m_pimpl = std::make_unique<Pimpl>();
}

UiRenderer::~UiRenderer() = default;

void UiRenderer::Update() {
    m_pimpl->renderer->Update();
}

void UiRenderer::Render(DrawQueues& draw_queues) const {
    m_pimpl->renderer->Render();

    void* ui_pixels = m_pimpl->bitmap->LockPixels();
    
    glm::ivec2 ui_size = { m_pimpl->bitmap->width(), m_pimpl->bitmap->height() };
    dxgl::TextureSource source((unsigned char*) ui_pixels, dxgl::TextureFormat::BGRA, ui_size);

    m_pimpl->ui_texture = dxgl::Texture(source);
    m_pimpl->bitmap->UnlockPixels();
    
    m_pimpl->ui_draw.textures.clear();
    m_pimpl->ui_draw.textures.emplace_back(m_pimpl->ui_texture);

    draw_queues.QueueViewedDraw(RenderLayer::Ui, m_pimpl->ui_draw);
}

void UiRenderer::LoadHtml(std::string_view path) {
    std::string html = LoadFileToString(path);
    m_pimpl->view->LoadHTML(html.c_str());
}

void UiRenderer::LoadCss(std::string_view path) {
    std::string css = LoadFileToString(path);
    m_pimpl->config.user_stylesheet = css.c_str();
    
    Platform::instance().set_config(m_pimpl->config);
}

void UiRenderer::LoadUrl(dxtl::cstring_view path) {
    m_pimpl->view->LoadURL(path.c_str());
}