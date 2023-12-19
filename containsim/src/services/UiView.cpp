#include <services/UiView.hpp>
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

namespace {
    int GLFWModsToUltralightMods(int mods);
    int GLFWKeyCodeToUltralightKeyCode(int key);
    
    struct DrawData {
        dxgl::Vbo screen_vbo{};
        dxgl::Program screen_program{};
    };

    std::optional<DrawData> draw_data{};
}

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

class UiView::Pimpl : public ViewListener, LoadListener {
public:
    const dxgl::Window* window{};

    RefPtr<View> view{};
    RefPtr<Bitmap> bitmap{};
    BitmapSurface* bitmap_surface{};
    std::optional<glm::ivec2> requested_resize{};

    // TODO: Once rendering is on different thread, this needs to change
    // ui_texture needs to be owned in dxgl::Draw
    dxgl::Texture ui_texture{};
    dxgl::Vao ui_vao{};
    dxgl::Draw ui_draw{};
    
    Pimpl(const dxgl::Window& window, const RefPtr<View>& view) {
        this->window = &window;
        this->view = view;

        bitmap_surface = dynamic_cast<BitmapSurface*>(view->surface());
        bitmap = bitmap_surface->bitmap();

        view->set_load_listener(this);

        InitDraw();
    }

    void InitDraw() {
        using namespace dxgl;
        if (!draw_data.has_value()) {

            draw_data.emplace();

            draw_data->screen_program = dxgl::ProgramBuilder()
                .Vert("shaders/framebuffer.vert")
                .Frag("shaders/framebuffer.frag")
                .Link();
            dxgl::Uniform::Set(draw_data->screen_program, "screen_tex", 0);

            draw_data->screen_vbo.Upload(quad_vbo_data, dxgl::BufferUsage::Static);
        }

        VaoAttribBuilder()
            .Group(AttribGroup()
                .Vbo(draw_data->screen_vbo)
                .Attrib(Attribute()
                    .Type(AttribType::Float)
                    .Components(2)
                    .Multiply(2)
                )
            )
            .Apply(ui_vao);

        ui_draw.prim_type = dxgl::PrimType::TriangleFan;
        ui_draw.num_indices = 4;
        ui_draw.program = draw_data->screen_program;
        ui_draw.vao_view = ui_vao;
    }

    void MaybeResizeToRequested() {
        if (!requested_resize.has_value())
            return;

        view->Resize(requested_resize->x, requested_resize->y);
        bitmap_surface->Resize(requested_resize->x, requested_resize->y);
        bitmap = bitmap_surface->bitmap();
    }

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

void UiView::PimplDeleter::operator()(Pimpl* ptr) const {
    delete ptr;
}

UiView::UiView(const dxgl::Window& window, const RefPtr<View>& view) {
    m_pimpl.reset(new Pimpl(window, view));
}

UiView::~UiView() = default;

void UiView::Render(DrawQueues& draw_queues) const {
    m_pimpl->MaybeResizeToRequested();
    
    auto* ui_pixels = static_cast<unsigned char*>(m_pimpl->bitmap->LockPixels());
    
    glm::ivec2 ui_size = { m_pimpl->bitmap->width(), m_pimpl->bitmap->height() };
    dxgl::TextureSource source(ui_pixels, dxgl::TextureFormat::BGRA, ui_size);

    m_pimpl->ui_texture = dxgl::Texture(source);
    m_pimpl->bitmap->UnlockPixels();
    
    m_pimpl->ui_draw.textures.clear();
    m_pimpl->ui_draw.textures.emplace_back(m_pimpl->ui_texture);

    draw_queues.QueueViewedDraw(RenderLayer::Ui, m_pimpl->ui_draw);
}

void UiView::PushAction(Action&& action) {
    std::visit(dxtl::overloaded{
        [this](const KeyPress& a) {
            ultralight::KeyEvent evt{};

            evt.type = a.dir == ButtonDir::Down || a.dir == ButtonDir::Repeat 
                ? ultralight::KeyEvent::kType_RawKeyDown : ultralight::KeyEvent::kType_KeyUp;

            evt.virtual_key_code = GLFWKeyCodeToUltralightKeyCode(a.key);
            evt.native_key_code = a.scancode;
            evt.modifiers = GLFWModsToUltralightMods(a.mods);
            ultralight::GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);

            m_pimpl->view->FireKeyEvent(evt);

            if (evt.type == ultralight::KeyEvent::kType_RawKeyDown &&
                (a.key == GLFW_KEY_ENTER || a.key == GLFW_KEY_TAB)) {
                // We have to synthesize the Char Event for these keys.
                ultralight::KeyEvent synth_evt;
                synth_evt.type = ultralight::KeyEvent::kType_Char;
                synth_evt.text = a.key == GLFW_KEY_ENTER 
                    ? ultralight::String("\r") 
                    : ultralight::String("\t");
                synth_evt.unmodified_text = synth_evt.text;

                m_pimpl->view->FireKeyEvent(synth_evt);
            }
        },
        [this](const TextInput& a) {
            ultralight::KeyEvent evt{};
            evt.type = ultralight::KeyEvent::kType_Char;

            char32_t c32 = a.codepoint;
            ultralight::String32 text(&c32, 1);

            evt.text = text;
            evt.unmodified_text = text;

            m_pimpl->view->FireKeyEvent(evt);
        },
        [this](const MouseMove& a) {
            ultralight::MouseEvent evt{};

            evt.type = ultralight::MouseEvent::kType_MouseMoved;
            
            auto scaled = (glm::vec2) a.to / m_pimpl->window->GetScale();
            evt.x = (int) scaled.x;
            evt.y = (int) scaled.y;

            auto* glfw_window = m_pimpl->window->GetGlfwWindow();

            if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Left;
            } else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Middle;
            } else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Right;
            }

            m_pimpl->view->FireMouseEvent(evt);
        },
        [this](const MouseClick& a) {
            ultralight::MouseEvent evt{};

            evt.type = a.dir == ButtonDir::Down 
                ? ultralight::MouseEvent::kType_MouseDown 
                : ultralight::MouseEvent::kType_MouseUp;
            
            auto scaled = (glm::vec2) a.pos / m_pimpl->window->GetScale();
            evt.x = (int) scaled.x;
            evt.y = (int) scaled.y;

            auto* glfw_window = m_pimpl->window->GetGlfwWindow();
            if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Left;
            } else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Middle;
            } else if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                evt.button = ultralight::MouseEvent::kButton_Right;
            }

            m_pimpl->view->FireMouseEvent(evt);
        },
        [this](const ScrollInput& a) {
            ultralight::ScrollEvent evt{};
            evt.type = ultralight::ScrollEvent::kType_ScrollByPixel;
            evt.delta_x = (int) a.amount.x;
            evt.delta_y = (int) a.amount.y;

            m_pimpl->view->FireScrollEvent(evt);
        }
    }, action.data);
}

InputLayer UiView::TestMouse(glm::dvec2 pos) const {
    auto& bitmap = *m_pimpl->bitmap;

    if (pos.x < 0 || pos.x >= bitmap.width() || pos.y < 0 || pos.y >= bitmap.height())
        return InputLayer::Offscreen;

    const uint8_t* pixels = static_cast<uint8_t*>(bitmap.LockPixels());
    
    const auto pixel_offset = (bitmap.row_bytes() * (uint32_t) pos.y) + (bitmap.bpp() * (uint32_t) pos.x);
    const uint8_t* pixel = pixels + pixel_offset;
    uint8_t alpha = pixel[3];

    m_pimpl->bitmap->UnlockPixels();

    if (alpha == 0) {
        return InputLayer::Game;
    } else {
        return InputLayer::Ui;
    }
}

void UiView::LoadHtml(std::string_view path) {
    std::string html = LoadFileToString(path);
    m_pimpl->view->LoadHTML(html.c_str());
}

void UiView::LoadUrl(dxtl::cstring_view path) {
    m_pimpl->view->LoadURL(path.c_str());
}

void UiView::Resize(glm::ivec2 size) {
    m_pimpl->requested_resize = size;
}

RefPtr<View> UiView::GetView() {
    return m_pimpl->view;
}

namespace {
    int GLFWModsToUltralightMods(int mods) {
        int result = 0;
        if ((mods & GLFW_MOD_ALT) != 0)
            result |= KeyEvent::kMod_AltKey;
        if ((mods & GLFW_MOD_CONTROL) != 0)
            result |= KeyEvent::kMod_CtrlKey;
        if ((mods & GLFW_MOD_SUPER) != 0)
            result |= KeyEvent::kMod_MetaKey;
        if ((mods & GLFW_MOD_SHIFT) != 0)
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
            case GLFW_KEY_WORLD_1:
            case GLFW_KEY_WORLD_2:
            case GLFW_KEY_MENU:
            default: return GK_UNKNOWN;
        }
    }
}