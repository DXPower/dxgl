#include <services/UiRenderer.hpp>
#include <common/DrawQueues.hpp>
#include <common/FileUtils.hpp>

#include <dxgl/Application.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Uniform.hpp>
#include <dxtl/overloaded.hpp>

#include <array>
#include <iostream>
#include <glfw/glfw3.h>
#include <Ultralight/Ultralight.h>
#include <Ultralight/Listener.h>
#include <AppCore/Platform.h>


using namespace services;
using namespace ultralight;

static int GLFWModsToUltralightMods(int mods);
static int GLFWKeyCodeToUltralightKeyCode(int key);

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
        std::optional<glm::ivec2> requested_resize{};
                
        void ResizeToRequested() {
            view->Resize(requested_resize->x, requested_resize->y);
            bitmap_surface->Resize(requested_resize->x, requested_resize->y);
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
        view_config.initial_device_scale = main_window.GetScale().x;

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
    auto& view_info = m_pimpl->main_view;

    if (view_info.requested_resize.has_value())
        view_info.ResizeToRequested();
    
    m_pimpl->renderer->Render();
    
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

    if (view_info.requested_resize.has_value())
        view_info.ResizeToRequested();

    void* ui_pixels = view_info.bitmap->LockPixels();

    glm::ivec2 ui_size = { view_info.bitmap->width(), view_info.bitmap->height() };
    dxgl::TextureSource source((unsigned char*) ui_pixels, dxgl::TextureFormat::BGRA, ui_size);

    m_pimpl->inspector_texture = dxgl::Texture(source);
    view_info.bitmap->UnlockPixels();
    
    m_pimpl->inspector_draw.textures.clear();
    m_pimpl->inspector_draw.textures.emplace_back(m_pimpl->inspector_texture);

    draw_queues.QueueViewedDraw(RenderLayer::Ui, m_pimpl->inspector_draw);
}

// void UiRenderer::InputAction(const Action& action) {

// }

void UiRenderer::InputActionDebug(const Action& action) {
    std::visit(dxtl::overloaded{
        [this](const KeyPress& a) {
            ultralight::KeyEvent evt{};

            evt.type = a.dir == ButtonDir::Down || a.dir == ButtonDir::Repeat 
                ? ultralight::KeyEvent::kType_RawKeyDown : ultralight::KeyEvent::kType_KeyUp;

            evt.virtual_key_code = GLFWKeyCodeToUltralightKeyCode(a.key);
            evt.native_key_code = a.scancode;
            evt.modifiers = GLFWModsToUltralightMods(a.mods);
            ultralight::GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);

            m_pimpl->inspector_view.view->FireKeyEvent(evt);

            if (evt.type == ultralight::KeyEvent::kType_RawKeyDown &&
                (a.key == GLFW_KEY_ENTER || a.key == GLFW_KEY_TAB)) {
                // We have to synthesize the Char Event for these keys.
                ultralight::KeyEvent evt;
                evt.type = ultralight::KeyEvent::kType_Char;
                evt.text = a.key == GLFW_KEY_ENTER 
                    ? ultralight::String("\r") 
                    : ultralight::String("\t");
                evt.unmodified_text = evt.text;

                m_pimpl->inspector_view.view->FireKeyEvent(evt);
            }
        },
        [this](const TextInput& a) {
            ultralight::KeyEvent evt{};
            evt.type = ultralight::KeyEvent::kType_Char;

            char32_t c32 = a.codepoint;
            ultralight::String32 text(&c32, 1);

            evt.text = text;
            evt.unmodified_text = std::move(text);

            m_pimpl->inspector_view.view->FireKeyEvent(evt);
        },
        [this](const MouseMove& a) {
            ultralight::MouseEvent evt{};

            evt.type = ultralight::MouseEvent::kType_MouseMoved;
            
            auto scaled = (glm::vec2) a.to / m_pimpl->inspector_window->GetScale();
            evt.x = scaled.x;
            evt.y = scaled.y;

            auto* glfw_window = m_pimpl->inspector_window->GetGlfwWindow();
            if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Left;
            else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Middle;
            else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Right;

            m_pimpl->inspector_view.view->FireMouseEvent(evt);
        },
        [this](const MouseClick& a) {
            ultralight::MouseEvent evt{};

            evt.type = a.dir == ButtonDir::Down 
                ? ultralight::MouseEvent::kType_MouseDown 
                : ultralight::MouseEvent::kType_MouseUp;
            
            auto scaled = (glm::vec2) a.pos / m_pimpl->inspector_window->GetScale();
            evt.x = scaled.x;
            evt.y = scaled.y;

            auto* glfw_window = m_pimpl->inspector_window->GetGlfwWindow();
            if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Left;
            else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Middle;
            else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                evt.button = ultralight::MouseEvent::kButton_Right;

            m_pimpl->inspector_view.view->FireMouseEvent(evt);
        },
        [this](const ScrollInput& a) {
            ultralight::ScrollEvent evt{};
            evt.type = ultralight::ScrollEvent::kType_ScrollByPixel;
            evt.delta_x = a.amount.x;
            evt.delta_y = a.amount.y;

            m_pimpl->inspector_view.view->FireScrollEvent(evt);
        }
    }, action.data);
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
    m_pimpl->main_view.requested_resize = size;
}

void UiRenderer::ResizeInspector(glm::ivec2 size) {
    m_pimpl->inspector_view.requested_resize = size;
}


int GLFWModsToUltralightMods(int mods) {
    int result = 0;
    if (mods & GLFW_MOD_ALT)
        result |= KeyEvent::kMod_AltKey;
    if (mods & GLFW_MOD_CONTROL)
        result |= KeyEvent::kMod_CtrlKey;
    if (mods & GLFW_MOD_SUPER)
        result |= KeyEvent::kMod_MetaKey;
    if (mods & GLFW_MOD_SHIFT)
        result |= KeyEvent::kMod_ShiftKey;
    return result;
}

int GLFWKeyCodeToUltralightKeyCode(int key) {
    using namespace ultralight::KeyCodes;

    switch (key) {
        case GLFW_KEY_SPACE: return GK_SPACE;
        case GLFW_KEY_APOSTROPHE: return GK_OEM_7;
        case GLFW_KEY_COMMA: return GK_OEM_COMMA;
        case GLFW_KEY_MINUS: return GK_OEM_MINUS;
        case GLFW_KEY_PERIOD: return GK_OEM_PERIOD;
        case GLFW_KEY_SLASH: return GK_OEM_2;
        case GLFW_KEY_0: return GK_0;
        case GLFW_KEY_1: return GK_1;
        case GLFW_KEY_2: return GK_2;
        case GLFW_KEY_3: return GK_3;
        case GLFW_KEY_4: return GK_4;
        case GLFW_KEY_5: return GK_5;
        case GLFW_KEY_6: return GK_6;
        case GLFW_KEY_7: return GK_7;
        case GLFW_KEY_8: return GK_8;
        case GLFW_KEY_9: return GK_9;
        case GLFW_KEY_SEMICOLON: return GK_OEM_1;
        case GLFW_KEY_EQUAL: return GK_OEM_PLUS;
        case GLFW_KEY_A: return GK_A;
        case GLFW_KEY_B: return GK_B;
        case GLFW_KEY_C: return GK_C;
        case GLFW_KEY_D: return GK_D;
        case GLFW_KEY_E: return GK_E;
        case GLFW_KEY_F: return GK_F;
        case GLFW_KEY_G: return GK_G;
        case GLFW_KEY_H: return GK_H;
        case GLFW_KEY_I: return GK_I;
        case GLFW_KEY_J: return GK_J;
        case GLFW_KEY_K: return GK_K;
        case GLFW_KEY_L: return GK_L;
        case GLFW_KEY_M: return GK_M;
        case GLFW_KEY_N: return GK_N;
        case GLFW_KEY_O: return GK_O;
        case GLFW_KEY_P: return GK_P;
        case GLFW_KEY_Q: return GK_Q;
        case GLFW_KEY_R: return GK_R;
        case GLFW_KEY_S: return GK_S;
        case GLFW_KEY_T: return GK_T;
        case GLFW_KEY_U: return GK_U;
        case GLFW_KEY_V: return GK_V;
        case GLFW_KEY_W: return GK_W;
        case GLFW_KEY_X: return GK_X;
        case GLFW_KEY_Y: return GK_Y;
        case GLFW_KEY_Z: return GK_Z;
        case GLFW_KEY_LEFT_BRACKET: return GK_OEM_4;
        case GLFW_KEY_BACKSLASH: return GK_OEM_5;
        case GLFW_KEY_RIGHT_BRACKET: return GK_OEM_6;
        case GLFW_KEY_GRAVE_ACCENT: return GK_OEM_3;
        case GLFW_KEY_WORLD_1: return GK_UNKNOWN;
        case GLFW_KEY_WORLD_2: return GK_UNKNOWN;
        case GLFW_KEY_ESCAPE: return GK_ESCAPE;
        case GLFW_KEY_ENTER: return GK_RETURN;
        case GLFW_KEY_TAB: return GK_TAB;
        case GLFW_KEY_BACKSPACE: return GK_BACK;
        case GLFW_KEY_INSERT: return GK_INSERT;
        case GLFW_KEY_DELETE: return GK_DELETE;
        case GLFW_KEY_RIGHT: return GK_RIGHT;
        case GLFW_KEY_LEFT: return GK_LEFT;
        case GLFW_KEY_DOWN: return GK_DOWN;
        case GLFW_KEY_UP: return GK_UP;
        case GLFW_KEY_PAGE_UP: return GK_PRIOR;
        case GLFW_KEY_PAGE_DOWN: return GK_NEXT;
        case GLFW_KEY_HOME: return GK_HOME;
        case GLFW_KEY_END: return GK_END;
        case GLFW_KEY_CAPS_LOCK: return GK_CAPITAL;
        case GLFW_KEY_SCROLL_LOCK: return GK_SCROLL;
        case GLFW_KEY_NUM_LOCK: return GK_NUMLOCK;
        case GLFW_KEY_PRINT_SCREEN: return GK_SNAPSHOT;
        case GLFW_KEY_PAUSE: return GK_PAUSE;
        case GLFW_KEY_F1: return GK_F1;
        case GLFW_KEY_F2: return GK_F2;
        case GLFW_KEY_F3: return GK_F3;
        case GLFW_KEY_F4: return GK_F4;
        case GLFW_KEY_F5: return GK_F5;
        case GLFW_KEY_F6: return GK_F6;
        case GLFW_KEY_F7: return GK_F7;
        case GLFW_KEY_F8: return GK_F8;
        case GLFW_KEY_F9: return GK_F9;
        case GLFW_KEY_F10: return GK_F10;
        case GLFW_KEY_F11: return GK_F11;
        case GLFW_KEY_F12: return GK_F12;
        case GLFW_KEY_F13: return GK_F13;
        case GLFW_KEY_F14: return GK_F14;
        case GLFW_KEY_F15: return GK_F15;
        case GLFW_KEY_F16: return GK_F16;
        case GLFW_KEY_F17: return GK_F17;
        case GLFW_KEY_F18: return GK_F18;
        case GLFW_KEY_F19: return GK_F19;
        case GLFW_KEY_F20: return GK_F20;
        case GLFW_KEY_F21: return GK_F21;
        case GLFW_KEY_F22: return GK_F22;
        case GLFW_KEY_F23: return GK_F23;
        case GLFW_KEY_F24: return GK_F24;
        case GLFW_KEY_F25: return GK_UNKNOWN;
        case GLFW_KEY_KP_0: return GK_NUMPAD0;
        case GLFW_KEY_KP_1: return GK_NUMPAD1;
        case GLFW_KEY_KP_2: return GK_NUMPAD2;
        case GLFW_KEY_KP_3: return GK_NUMPAD3;
        case GLFW_KEY_KP_4: return GK_NUMPAD4;
        case GLFW_KEY_KP_5: return GK_NUMPAD5;
        case GLFW_KEY_KP_6: return GK_NUMPAD6;
        case GLFW_KEY_KP_7: return GK_NUMPAD7;
        case GLFW_KEY_KP_8: return GK_NUMPAD8;
        case GLFW_KEY_KP_9: return GK_NUMPAD9;
        case GLFW_KEY_KP_DECIMAL: return GK_DECIMAL;
        case GLFW_KEY_KP_DIVIDE: return GK_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY: return GK_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT: return GK_SUBTRACT;
        case GLFW_KEY_KP_ADD: return GK_ADD;
        case GLFW_KEY_KP_ENTER: return GK_RETURN;
        case GLFW_KEY_KP_EQUAL: return GK_OEM_PLUS;
        case GLFW_KEY_LEFT_SHIFT: return GK_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return GK_CONTROL;
        case GLFW_KEY_LEFT_ALT: return GK_MENU;
        case GLFW_KEY_LEFT_SUPER: return GK_LWIN;
        case GLFW_KEY_RIGHT_SHIFT: return GK_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL: return GK_CONTROL;
        case GLFW_KEY_RIGHT_ALT: return GK_MENU;
        case GLFW_KEY_RIGHT_SUPER: return GK_RWIN;
        case GLFW_KEY_MENU: return GK_UNKNOWN;
        default: return GK_UNKNOWN;
    }
}