/****************************************************************************

    Copyright 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#include <asgl/sfml/SfmlEngine.hpp>

#include "SfmlFontAndText.hpp"

// use most controls
#include <asgl/Button.hpp>
#include <asgl/ArrowButton.hpp>
#include <asgl/TextButton.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/ProgressBar.hpp>
#include <asgl/EditableText.hpp>

#include <asgl/Frame.hpp>
#include <asgl/Text.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace {

using SharedImagePtr = asgl::SharedImagePtr;
using RtError        = std::runtime_error;
using InvArg         = std::invalid_argument;
using StyleField     = asgl::StyleField;
using SfmlRenderItem = asgl::SfmlFlatEngine::SfmlRenderItem;

template <typename T>
constexpr const int k_item_type_id = asgl::SfmlFlatEngine::SfmlRenderItem::GetTypeId<T>::k_value;

asgl::Event convert(const sf::Event &);

template <typename T>
inline StyleField to_field(const T & obj)
    { return StyleField(obj); }

template <>
inline StyleField to_field<asgl::flat_colors::ItemColorEnum>
    (const asgl::flat_colors::ItemColorEnum & obj)
{ return StyleField(asgl::SfmlFlatEngine::ItemStyles::to_key(obj)); }

inline SfmlRenderItem to_item(sf::Color color) {
    return asgl::SfmlFlatEngine::SfmlRenderItem(asgl::SfmlFlatEngine::ColorItem(color));
}

} // end of <anonymous> namespace

namespace asgl {

void SfmlFlatEngine::assign_target_and_states
    (sf::RenderTarget & target, sf::RenderStates states)
{
    m_target_ptr = &target;
    m_states = states;
}

void SfmlFlatEngine::stylize(Widget & widget) const {
    if (!m_first_setup_done) {
        throw RtError("SfmlFlatEngine::stylize: cannot stylize without setting "
                      "up the style map first (setup_default_styles must be "
                      "called first).");
    }
    widget.stylize(m_style_map);
}

void SfmlFlatEngine::setup_default_styles() {
    using namespace flat_colors;
    static constexpr const int k_chosen_padding = 5;
    if (m_first_setup_done) return;
    if (m_style_map.has_same_map_pointer(StyleMap())) {
        m_style_map = StyleMap::construct_new_map();
    }
    {
    auto & stylemap = m_style_map;
    stylemap.add(styles::k_global_padding, to_field(k_chosen_padding));
    stylemap.add(styles::k_global_font   , to_field( std::weak_ptr<const Font>( m_font_handler ) ));
    // frame
    auto add_frame_field = [&stylemap] (Frame::StyleEnum style, const StyleField & field)
        { stylemap.add(Frame::to_key(style), field); };
    add_frame_field(Frame::k_title_bar_style  , to_field(k_primary_mid));
#   if 0
    add_frame_field(Frame::k_title_size_style , to_field(22));
    add_frame_field(Frame::k_title_color_style, to_field(k_text_color));
#   endif
    add_frame_field(Frame::k_widget_body_style, to_field(k_primary_dark));
    add_frame_field(Frame::k_border_size_style, to_field(k_chosen_padding));

    add_frame_field(Frame::k_title_text_style , to_field(k_title_text));
    add_frame_field(Frame::k_widget_text_style, to_field(k_widget_text));
    // button
    auto add_button_field = [&stylemap](Button::ButtonStyleEnum style, const StyleField & field)
        { stylemap.add(Button::to_key(style), field); };
    add_button_field(Button::k_button_padding     , to_field(k_chosen_padding));
    add_button_field(Button::k_regular_front_style, to_field(k_secondary_mid));
    add_button_field(Button::k_regular_back_style , to_field(k_secondary_dark));
    add_button_field(Button::k_hover_back_style   , to_field(k_secondary_mid));
    add_button_field(Button::k_hover_front_style  , to_field(k_secondary_light));
    // arrow button
    stylemap.add(ArrowButton::to_key(ArrowButton::k_triangle_style), to_field(k_text_color));
    // progress bar
    auto add_pbar_field = [&stylemap](ProgressBar::StyleEnum style, const StyleField & field)
        { stylemap.add(ProgressBar::to_key(style), field); };
    add_pbar_field(ProgressBar::k_outer_style  , to_field(k_secondary_dark));
    add_pbar_field(ProgressBar::k_fill_style   , to_field(k_primary_light));
    add_pbar_field(ProgressBar::k_void_style   , to_field(k_text_color_dark));
    add_pbar_field(ProgressBar::k_padding_style, to_field(k_chosen_padding));
    // editable text
    auto add_etext_field = [&stylemap] (EditableText::StyleEnum style, const StyleField & field)
        { stylemap.add(EditableText::to_key(style), field); };
    add_etext_field(EditableText::k_cursor_style, to_field(k_text_color_dark));
    add_etext_field(EditableText::k_text_background_style, to_field(k_text_color));
    add_etext_field(EditableText::k_widget_border_style, to_field(k_secondary_mid));
    add_etext_field(EditableText::k_fill_text_style, to_field(k_editable_text_fill));
    add_etext_field(EditableText::k_empty_text_style, to_field(k_editable_text_empty));
#   if 0
    stylemap.add(TextArea::to_key(TextArea::k_text_size ), StyleField(18));
    stylemap.add(TextArea::to_key(TextArea::k_text_color), StyleField(sf::Color::White));
#   endif
    }

    m_font_handler->add_font_style(to_item_key(k_title_text ), 22, sf::Color::White);
    m_font_handler->add_font_style(to_item_key(k_widget_text), 18, sf::Color::White);
    // editable text
    m_font_handler->add_font_style(to_item_key(k_editable_text_fill ), 18, sf::Color::Black);
    m_font_handler->add_font_style(to_item_key(k_editable_text_empty), 18, sf::Color(100, 100, 100));

    using sf::Color;
    m_items[to_item_key(k_primary_light  )] = to_item(Color(0x51, 0x51, 0x76));
    m_items[to_item_key(k_primary_mid    )] = to_item(Color(0x18, 0x18, 0x40));
    m_items[to_item_key(k_primary_dark   )] = to_item(Color(0x08, 0x08, 0x22));
    m_items[to_item_key(k_secondary_light)] = to_item(Color(0x77, 0x6A, 0x45));
    m_items[to_item_key(k_secondary_mid  )] = to_item(Color(0x4B, 0x46, 0x15));
    m_items[to_item_key(k_secondary_dark )] = to_item(Color(0x30, 0x2C, 0x05));
    m_items[to_item_key(k_text_color     )] = to_item(Color::White);
    m_items[to_item_key(k_text_color_dark)] = to_item(Color(0x40,    0,    0));

    m_first_setup_done = true;
}

ItemKey SfmlFlatEngine::add_rectangle_style(sf::Color color, StyleKey stylekey) {
    auto item_key = m_item_key_creator.make_key();
    m_items[item_key] = to_item(color);
    m_style_map.add(stylekey, StyleField(item_key));
    return item_key;
}

void SfmlFlatEngine::load_global_font(const std::string & filename) {
    m_font_handler = std::make_shared<detail::SfmlFont>();
    m_font_handler->load_font(filename);
    setup_default_styles();
}

/* static */ const sf::Texture * SfmlFlatEngine::dynamic_cast_to_texture
    (SharedImagePtr ptr)
{
    auto downcasted = std::dynamic_pointer_cast<SfmlImageResource>(ptr);
    if (!downcasted) return nullptr;
    return &downcasted->texture;
}

/* static */ Event SfmlFlatEngine::convert(const sf::Event & event)
    { return ::convert(event); }

SfmlFlatEngine::ColorItem::ColorItem(sf::Color color) {
    m_triangle .set_color(color);
    m_rectangle.set_color(color);
}

void SfmlFlatEngine::ColorItem::update(const sf::IntRect & rect) {
    m_rectangle = DrawRectangle(
        float(rect.left), float(rect.top), float(rect.width), float(rect.height),
        m_rectangle.color());
}
void SfmlFlatEngine::ColorItem::update(const TriangleTuple & trituple) {
    using std::get;
    m_triangle.set_point_a(sf::Vector2f(std::get<0>(trituple)));
    m_triangle.set_point_b(sf::Vector2f(std::get<1>(trituple)));
    m_triangle.set_point_c(sf::Vector2f(std::get<2>(trituple)));
}

/* private */ void SfmlFlatEngine::render_rectangle
    (const sf::IntRect & rect, ItemKey itemkey, const void *)
{
    auto itr = m_items.find(itemkey);
    if (itr == m_items.end())
        return;
    switch (itr->second.type_id()) {
    case k_item_type_id<SfmlImageResPtr>:
        return render_rectangle(rect, *itr->second.as<SfmlImageResPtr>());
    case k_item_type_id<ColorItem>:
        return render_rectangle(rect, itr->second.as<ColorItem>());
#   if 0
    case k_item_type_id<DrawTriangle>:
        throw RtError("SfmlFlatEngine::render_rectangle: Assigned ItemKey "
                      "belonging to a triangle and attempted to draw it as a "
                      "rectangle.");
#   endif
    default:
        throw RtError("SfmlFlatEngine::render_rectangle: Bad branch.");
    }
}

/* private */ void SfmlFlatEngine::render_triangle
    (const TriangleTuple & tuple, ItemKey itemkey, const void *)
{
    auto itr = m_items.find(itemkey);
    if (itr == m_items.end())
        return;
    switch (itr->second.type_id()) {
    case k_item_type_id<SfmlImageResPtr>:
        throw RtError("SfmlFlatEngine::render_rectangle: Assigned ItemKey "
                      "belonging to a rectangle and attempted to draw it as a "
                      "triangle.");
    case k_item_type_id<ColorItem>:
        return render_triangle(tuple, itr->second.as<ColorItem>());
#   if 0
    case k_item_type_id<DrawTriangle>:
        render_triangle(tuple, itr->second.as<DrawTriangle>());
        break;
#   endif
    default:
        throw RtError("SfmlFlatEngine::render_rectangle: Bad branch.");
    }
}

/* private */ void SfmlFlatEngine::render_text(const TextBase & text_base) {
    const auto * dc_text = dynamic_cast<const detail::SfmlText *>(&text_base);
    if (!dc_text) return;
#   if 0
    auto & drect = m_items.find(to_item_key(flat_colors::k_text_color_dark))->second.as<DrawRectangle>();
    drect = DrawRectangle(dc_text->location().x, dc_text->location().y,
                          dc_text->width(), dc_text->height(), drect.color());
    m_target_ptr->draw(drect, m_states);
#   endif
    m_target_ptr->draw(*dc_text, m_states);
}


/* private */ void SfmlFlatEngine::render_rectangle
    (const sf::IntRect & rect, ColorItem & color_item) const
{
    color_item.update(rect);
    m_target_ptr->draw(color_item.rectangle(), m_states);
#   if 0
    drect = DrawRectangle(
        float(rect.left ), float(rect.top   ),
        float(rect.width), float(rect.height), drect.color());
    m_target_ptr->draw(drect, m_states);
#   endif
}

/* private */ void SfmlFlatEngine::render_rectangle
    (const sf::IntRect & rect, SfmlImageResource & res) const
{
    auto & spt = res.sprite;
    spt.setPosition(float(rect.left), float(rect.top));
    spt.setScale(float( rect.width  ) / float(spt.getTextureRect().width ),
                 float( rect.height ) / float(spt.getTextureRect().height));
    m_target_ptr->draw(spt, m_states);
}

/* private */ void SfmlFlatEngine::render_triangle
    (const TriangleTuple & trituple, ColorItem & color_item) const
{
    color_item.update(trituple);
    m_target_ptr->draw(color_item.triangle(), m_states);
#   if 0
    using std::get;
    triangle.set_point_a(sf::Vector2f(std::get<0>(trituple)));
    triangle.set_point_b(sf::Vector2f(std::get<1>(trituple)));
    triangle.set_point_c(sf::Vector2f(std::get<2>(trituple)));
    m_target_ptr->draw(triangle, m_states);
#   endif
}


/* private */ SharedImagePtr SfmlFlatEngine::make_image_resource
    (const std::string & filename)
{
    auto rv = std::make_shared<SfmlImageResource>();
    if (!rv->texture.loadFromFile(filename)) {
        throw RtError("SfmlFlatEngine::make_image_resource: Cannot load "
                      "texture from file \"" + filename + "\".");
    }
    rv->item = m_item_key_creator.make_key();
    add_and_verify_unique(rv->item) = SfmlRenderItem( rv );
    rv->sprite.setTexture(rv->texture);
    return rv;
}

/* private */ SharedImagePtr SfmlFlatEngine::make_image_resource
    (SharedImagePtr ptr)
{
    if (!ptr) return nullptr;
    const auto * texture_ptr = dynamic_cast_to_texture(ptr);
    if (!texture_ptr) {
        throw InvArg("SfmlFlatEngine::make_image_resource: Source image type "
                     "is not the same as the type used by this engine.");
    }
    auto rv = std::make_shared<SfmlImageResource>();
    rv->item  = m_item_key_creator.make_key();
    add_and_verify_unique(rv->item) = SfmlRenderItem( rv );
    rv->texture = *texture_ptr;
    rv->sprite.setTexture(rv->texture);
    return rv;
}

/* private */ SfmlRenderItem & SfmlFlatEngine::add_and_verify_unique(ItemKey key) {
    auto p = std::make_pair(key, SfmlRenderItem());
    auto gv = m_items.insert(p);
    if (gv.second) {
        return gv.first->second;
    }
    throw std::invalid_argument("Cannot insert dupelicate item key.");
}

} // end of asgl namespace

namespace {

using asgl::keys::KeyEventImpl;
using asgl::mouse::MouseEventImpl;
using asgl::MouseMove;

KeyEventImpl convert(const sf::Event::KeyEvent &);
MouseEventImpl convert(const sf::Event::MouseButtonEvent &);
MouseMove convert(const sf::Event::MouseMoveEvent &);

asgl::Event convert(const sf::Event & sfevent) {
    using namespace asgl;
    switch (sfevent.type) {
    case sf::Event::KeyPressed:
        return Event(KeyPress(::convert(sfevent.key)));
    case sf::Event::KeyReleased:
        return Event(KeyRelease(::convert(sfevent.key)));
    case sf::Event::MouseButtonPressed:
        return Event(MousePress(::convert(sfevent.mouseButton)));
    case sf::Event::MouseButtonReleased:
        return Event(MouseRelease(::convert(sfevent.mouseButton)));
    case sf::Event::MouseMoved:
        return Event(MouseMove(::convert(sfevent.mouseMove)));
    case sf::Event::TextEntered:
        return Event(KeyTyped{sfevent.text.unicode});
    default: return Event();
    }
}

// ----------------------------------------------------------------------------

using asgl::Key;
using asgl::MouseButton;

Key convert(sf::Keyboard::Key);
MouseButton convert(sf::Mouse::Button);

KeyEventImpl convert(const sf::Event::KeyEvent & ke) {
    KeyEventImpl kei;
    kei.alt    = ke.alt;
    kei.key    = convert(ke.code);
    kei.ctrl   = ke.control;
    kei.shift  = ke.shift;
    kei.system = ke.system;
    return kei;
}

MouseEventImpl convert(const sf::Event::MouseButtonEvent & mbe) {
    MouseEventImpl mei;
    mei.x = mbe.x;
    mei.y = mbe.y;
    mei.button = convert(mbe.button);
    return mei;
}

// ----------------------------------------------------------------------------

Key convert(sf::Keyboard::Key k) {
    using namespace asgl::keys;
    using Ky = sf::Keyboard;
    switch (k) {
    case Ky::Unknown: return k_count;
    case Ky::A: return k_a; case Ky::B: return k_b; case Ky::C: return k_c;
    case Ky::D: return k_d; case Ky::E: return k_e; case Ky::F: return k_f;
    case Ky::G: return k_g; case Ky::H: return k_h; case Ky::I: return k_i;
    case Ky::J: return k_j; case Ky::K: return k_k; case Ky::L: return k_l;
    case Ky::M: return k_m; case Ky::N: return k_n; case Ky::O: return k_o;
    case Ky::P: return k_p; case Ky::Q: return k_q; case Ky::R: return k_r;
    case Ky::S: return k_s; case Ky::T: return k_t; case Ky::U: return k_u;
    case Ky::V: return k_v; case Ky::W: return k_w; case Ky::X: return k_x;
    case Ky::Y: return k_y; case Ky::Z: return k_z;
    case Ky::Num0: return k_zero ; case Ky::Num1: return k_one  ;
    case Ky::Num2: return k_two  ; case Ky::Num3: return k_three;
    case Ky::Num4: return k_four ; case Ky::Num5: return k_five ;
    case Ky::Num6: return k_six  ; case Ky::Num7: return k_seven;
    case Ky::Num8: return k_eight; case Ky::Num9: return k_nine ;
    case Ky::Escape   : return k_escape        ;
    case Ky::LControl : return k_left_control  ;
    case Ky::LShift   : return k_left_shift    ;
    case Ky::LAlt     : return k_left_alt      ;
    case Ky::LSystem  : return k_left_system   ;
    case Ky::RControl : return k_right_control ;
    case Ky::RShift   : return k_right_shift   ;
    case Ky::RAlt     : return k_right_alt     ;
    case Ky::RSystem  : return k_right_system  ;
    case Ky::Menu     : return k_menu          ;
    case Ky::LBracket : return k_left_bracket  ;
    case Ky::RBracket : return k_right_bracket ;
    case Ky::SemiColon: return k_semicolon     ;
    case Ky::Comma    : return k_comma         ;
    case Ky::Period   : return k_period        ;
    case Ky::Quote    : return k_quote         ;
    case Ky::Slash    : return k_foreslash     ;
    case Ky::BackSlash: return k_backslash     ;
    case Ky::Tilde    : return k_tilde         ;
    case Ky::Equal    : return k_equal         ;
    case Ky::Dash     : return k_dash          ;
    case Ky::Space    : return k_space         ;
    case Ky::Return   : return k_enter         ;
    case Ky::BackSpace: return k_backspace     ;
    case Ky::Tab      : return k_tab           ;
    case Ky::PageUp   : return k_pageup        ;
    case Ky::PageDown : return k_pagedown      ;
    case Ky::End      : return k_end           ;
    case Ky::Home     : return k_home          ;
    case Ky::Insert   : return k_insert        ;
    case Ky::Delete   : return k_delete        ;
    case Ky::Add      : return k_plus          ;
    case Ky::Subtract : return k_minus         ;
    case Ky::Multiply : return k_multiplication;
    case Ky::Divide   : return k_division      ;
    case Ky::Left     : return k_left ; case Ky::Right    : return k_right;
    case Ky::Up       : return k_up   ; case Ky::Down     : return k_down ;
    case Ky::Numpad0: return k_pad_zero ;
    case Ky::Numpad1: return k_pad_one  ;
    case Ky::Numpad2: return k_pad_two  ;
    case Ky::Numpad3: return k_pad_three;
    case Ky::Numpad4: return k_pad_four ;
    case Ky::Numpad5: return k_pad_five ;
    case Ky::Numpad6: return k_pad_six  ;
    case Ky::Numpad7: return k_pad_seven;
    case Ky::Numpad8: return k_pad_eight;
    case Ky::Numpad9: return k_pad_nine ;
    case Ky::F1 : return k_f1 ; case Ky::F2 : return k_f2 ;
    case Ky::F3 : return k_f3 ; case Ky::F4 : return k_f4 ;
    case Ky::F5 : return k_f5 ; case Ky::F6 : return k_f6 ;
    case Ky::F7 : return k_f7 ; case Ky::F8 : return k_f8 ;
    case Ky::F9 : return k_f9 ; case Ky::F10: return k_f10;
    case Ky::F11: return k_f11; case Ky::F12: return k_f12;
    case Ky::F13: case Ky::F14: case Ky::F15:
        return k_count;
    case Ky::Pause: return k_pause;
    case Ky::KeyCount: return k_count;
    }
    throw std::runtime_error("impossible branch");
}

MouseButton convert(sf::Mouse::Button b) {
    using namespace asgl::mouse;
    using Mb = sf::Mouse;
    switch (b) {
    case Mb::Left  : return k_left  ;
    case Mb::Right : return k_right ;
    case Mb::Middle: return k_middle;
    default        : return k_count ;
    }
}

MouseMove convert(const sf::Event::MouseMoveEvent & mme) {
    MouseMove rv;
    rv.x = mme.x;
    rv.y = mme.y;
    return rv;
}

} // end of <anonymous> namespace
