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
#include <asgl/OptionsSlider.hpp>

#include <asgl/Frame.hpp>
#include <asgl/Text.hpp>

#include <common/SfmlVectorTraits.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <cmath>
#include <cassert>

namespace {

using namespace cul::exceptions_abbr;
using SharedImagePtr    = asgl::SharedImagePtr;
using StyleField        = asgl::StyleField;
using SfmlRenderItem    = asgl::SfmlFlatEngine::SfmlRenderItem;
using RoundedBorder     = asgl::SfmlFlatEngine::RoundedBorder;
using SquareBorder      = asgl::SfmlFlatEngine::SquareBorder;
using SfmlRenderItemMap = asgl::SfmlFlatEngine::SfmlRenderItemMap;
using ColorItem         = asgl::SfmlFlatEngine::ColorItem;
using SfmlImageResPtr   = asgl::SfmlFlatEngine::SfmlImageResPtr;
using asgl::WidgetRenderer, asgl::Rectangle, asgl::StyleValue, asgl::Triangle,
      asgl::TextBase, asgl::Widget, asgl::detail::SfmlImageResource,
      asgl::SampleStyleColor, asgl::SampleStyleValue;

const std::array k_palette = [] {
    //using namespace asgl::sfml_items;
    using namespace asgl::sample_style_values;
    using sf::Color;
    std::array<Color, k_color_count> rv;
    // not like this: "= { ... };" I want to see which index to which color

    rv[k_primary_light] = Color(0x51, 0x51, 0x76);
    rv[k_primary_mid  ] = Color(0x18, 0x18, 0x40);
    rv[k_primary_dark ] = Color(0x08, 0x08, 0x22);

    rv[k_secondary_light] = Color(0x77, 0x6A, 0x45);
    rv[k_secondary_mid  ] = Color(0x4B, 0x46, 0x15);
    rv[k_secondary_dark ] = Color(0x30, 0x2C, 0x05);

    rv[k_mono_light] = Color(0xFE, 0xFE, 0xFE);
    rv[k_mono_dark ] = Color(0x40,    0,    0);

    assert(!std::any_of(rv.begin(), rv.end(), [](Color c) { return c == Color(); }));
    return rv;
} ();

template <typename T>
constexpr const int k_item_type_id = asgl::SfmlFlatEngine::SfmlRenderItem::GetTypeId<T>::k_value;

template <typename T>
constexpr const bool is_field_type_t = StyleField::HasType<T>::k_value
    || std::is_same_v<SampleStyleColor, T>
    || std::is_same_v<SampleStyleValue, T>;

class SfmlWidgetRenderer final : public WidgetRenderer {
public:
    SfmlWidgetRenderer(sf::RenderTarget &, sf::RenderStates,
                       SfmlRenderItemMap &);

    void render_rectangle(const Rectangle &, StyleValue, const void *) final;
    void render_triangle (const Triangle  &, StyleValue, const void *) final;
    void render_text(const TextBase &) final;

    void render_rectangle_pair(const Rectangle &, const Rectangle &, StyleValue, const void *) final;

    void render_special(StyleValue, const Widget * instance_pointer) final;

private:
    void render_rectangle(const Rectangle &, ColorItem &) const;
    void render_triangle (const Triangle  &, ColorItem &) const;

    void render_rectangle_pair(const Rectangle &, const Rectangle &, RoundedBorder &) const;
    void render_rectangle_pair(const Rectangle &, const Rectangle &, SquareBorder &) const;
    void render_rectangle_pair(const Rectangle &, const Rectangle &, SfmlImageResource &) const;

    sf::RenderTarget & m_target;
    SfmlRenderItemMap & m_items;
    sf::RenderStates m_states;
};

asgl::Event convert(const sf::Event &);

inline sf::IntRect convert_to_sfml_rectangle(const asgl::Rectangle & rect)
    { return sf::IntRect(rect.left, rect.top, rect.width, rect.height); }

template <typename T>
inline std::enable_if_t<is_field_type_t<T>, StyleField>
    to_field(const T & obj)
    { return StyleField(obj); }

template <>
inline StyleField to_field(const SampleStyleColor & obj)
    { return StyleField(asgl::SfmlFlatEngine::to_item_key(obj)); }

template <>
inline StyleField to_field(const SampleStyleValue & obj)
    { return StyleField(asgl::SfmlFlatEngine::to_item_key(obj)); }

RoundedBorder make_rounded_border(sf::Color back, sf::Color front, int padding);
#if 0
SquareBorder make_square_border(sf::Color back, sf::Color front);
#endif
SfmlRenderItem to_color_item(sf::Color color);

} // end of <anonymous> namespace

namespace asgl {

void SfmlFlatEngine::stylize(Widget & widget) const {
    if (!m_first_setup_done) {
        throw RtError("SfmlFlatEngine::stylize: cannot stylize without setting "
                      "up the style map first (setup_default_styles must be "
                      "called first).");
    }
    widget.stylize(m_style_map);
}

void SfmlFlatEngine::setup_default_styles() {
    using namespace sample_style_values;
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
        {
        using namespace frame_styles;
        auto add_frame_field = [&stylemap] (FrameStyle style, const StyleField & field)
            { stylemap.add(asgl::to_key(style), field); };
        add_frame_field(k_title_bar_style  , to_field(k_primary_mid));
        add_frame_field(k_widget_body_style, to_field(k_primary_dark));
        add_frame_field(k_border_size_style, to_field(k_chosen_padding));

        add_frame_field(k_title_text_style , to_field(k_title_text));
        add_frame_field(k_widget_text_style, to_field(k_widget_text));
        }
    // button

    auto add_button_field = [&stylemap](Button::ButtonStyleEnum style, const StyleField & field)
        { stylemap.add(Button::to_key(style), field); };
    add_button_field(Button::k_button_padding     , to_field(k_chosen_padding));
    add_button_field(Button::k_regular_style,  to_field(k_bordered_regular_widget));
    add_button_field(Button::k_hover_style,  to_field(k_bordered_hover_widget));
    add_button_field(Button::k_focus_style,  to_field(k_bordered_focus_widget));
    add_button_field(Button::k_hover_and_focus_style,  to_field(k_bordered_hover_and_focus_widget));
    // arrow button
    stylemap.add(ArrowButton::to_key(ArrowButton::k_triangle_style), to_field(k_mono_light));
    // progress bar
    auto add_pbar_field = [&stylemap](ProgressBar::StyleEnum style, const StyleField & field)
        { stylemap.add(ProgressBar::to_key(style), field); };
    add_pbar_field(ProgressBar::k_outer_style  , to_field(k_secondary_dark));
    add_pbar_field(ProgressBar::k_fill_style   , to_field(k_primary_light));
    add_pbar_field(ProgressBar::k_void_style   , to_field(k_mono_dark));
    add_pbar_field(ProgressBar::k_padding_style, to_field(k_chosen_padding));
    // editable text
    auto add_etext_field = [&stylemap] (EditableText::StyleEnum style, const StyleField & field)
        { stylemap.add(EditableText::to_key(style), field); };
    add_etext_field(EditableText::k_cursor_style, to_field(k_mono_dark));
    add_etext_field(EditableText::k_text_background_style, to_field(k_mono_light));
    add_etext_field(EditableText::k_widget_border_style, to_field(k_secondary_mid));
    add_etext_field(EditableText::k_fill_text_style, to_field(k_editable_text_fill));
    add_etext_field(EditableText::k_empty_text_style, to_field(k_editable_text_empty));
    // options slider
    stylemap.add(OptionsSlider::to_key(OptionsSlider::k_back_style ), to_field(k_secondary_dark));
    stylemap.add(OptionsSlider::to_key(OptionsSlider::k_front_style), to_field(k_secondary_mid ));
    }

    m_font_handler->add_font_style(to_item_key(k_title_text ), 22, sf::Color::White);
    m_font_handler->add_font_style(to_item_key(k_widget_text), 18, sf::Color::White);
    // editable text
    m_font_handler->add_font_style(to_item_key(k_editable_text_fill ), 18, sf::Color::Black);
    m_font_handler->add_font_style(to_item_key(k_editable_text_empty), 18, sf::Color(100, 100, 100));

    using sf::Color;
    m_items[to_item_key(k_primary_light  )] = to_color_item(k_palette[k_primary_light  ]);
    m_items[to_item_key(k_primary_mid    )] = to_color_item(k_palette[k_primary_mid    ]);
    m_items[to_item_key(k_primary_dark   )] = to_color_item(k_palette[k_primary_dark   ]);
    m_items[to_item_key(k_secondary_light)] = to_color_item(k_palette[k_secondary_light]);
    m_items[to_item_key(k_secondary_mid  )] = to_color_item(k_palette[k_secondary_mid  ]);
    m_items[to_item_key(k_secondary_dark )] = to_color_item(k_palette[k_secondary_dark ]);
    m_items[to_item_key(k_mono_light     )] = to_color_item(k_palette[k_mono_light     ]);
    m_items[to_item_key(k_mono_dark      )] = to_color_item(k_palette[k_mono_dark      ]);

    auto make_button_item =  [](SampleStyleColor back, SampleStyleColor fore) {
        return SfmlRenderItem( make_rounded_border( k_palette[back], k_palette[fore], k_chosen_padding ) );
    };

    m_items[to_item_key(k_bordered_regular_widget)]
        = make_button_item(k_secondary_dark, k_secondary_mid);
    m_items[to_item_key(k_bordered_hover_widget)]
        = make_button_item(k_secondary_mid, k_secondary_dark);
    m_items[to_item_key(k_bordered_focus_widget)]
        = make_button_item(k_secondary_light, k_secondary_mid);
    m_items[to_item_key(k_bordered_hover_and_focus_widget)]
        = make_button_item(k_secondary_light, k_secondary_light);

    m_first_setup_done = true;
}

StyleValue SfmlFlatEngine::add_rectangle_style(sf::Color color, StyleKey stylekey) {
    auto item_key = m_item_key_creator.make_key();
    m_items[item_key] = to_color_item(color);
    m_style_map.add(stylekey, StyleField(item_key));
    return item_key;
}

void SfmlFlatEngine::load_global_font(const std::string & filename) {
    m_font_handler = std::make_shared<detail::SfmlFont>();
    m_font_handler->load_font(filename);
    setup_default_styles();
}

SharedImagePtr SfmlFlatEngine::make_image_from(ConstSubGrid<sf::Color> data) {
    sf::Image img;
    img.create(unsigned(data.width()), unsigned(data.height()));
    for (Vector r; r != data.end_position(); r = data.next(r)) {
        img.setPixel(unsigned(r.x), unsigned(r.y), data(r));
    }
    auto image_res = std::make_shared<SfmlImageResource>();
    image_res->item = m_item_key_creator.make_key();
    image_res->texture.loadFromImage(img);
    image_res->sprite.setTexture(image_res->texture);

    m_items[image_res->item] = SfmlRenderItem(image_res);
    return image_res;
}

void SfmlFlatEngine::draw
    (const Widget & widget, sf::RenderTarget & target, sf::RenderStates states)
{
    // a renderer would be better described as an aggregate of some kind...
    // be it an additional member or exist for this stack frame only...
    SfmlWidgetRenderer widren(target, states, m_items);
    widget.draw(widren);
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

void SfmlFlatEngine::ColorItem::update(const Rectangle & rect) {
    SfmlFlatEngine::update_draw_rectangle(m_rectangle, rect);
}

void SfmlFlatEngine::ColorItem::update(const Triangle & trituple) {
    using std::get;
    using cul::convert_to;
    m_triangle.set_point_a(convert_to<sf::Vector2f>(get<0>(trituple)));
    m_triangle.set_point_b(convert_to<sf::Vector2f>(get<1>(trituple)));
    m_triangle.set_point_c(convert_to<sf::Vector2f>(get<2>(trituple)));
}

/* static */ void SfmlFlatEngine::update_draw_rectangle
    (DrawRectangle & rectangle, const Rectangle & irect)
{
    rectangle = DrawRectangle(
        float(irect.left), float(irect.top), float(irect.width), float(irect.height),
        rectangle.color());
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

/* private */ SfmlRenderItem & SfmlFlatEngine::add_and_verify_unique(StyleValue key) {
    auto p = std::make_pair(key, SfmlRenderItem());
    auto gv = m_items.insert(p);
    if (gv.second) {
        return gv.first->second;
    }
    throw InvArg("Cannot insert dupelicate item key.");
}

} // end of asgl namespace

namespace {

using asgl::keys::KeyEventImpl;
using asgl::mouse::MouseEventImpl;
using asgl::MouseMove;
using asgl::gamepad::Button;
using asgl::gamepad::Axis;

KeyEventImpl convert(const sf::Event::KeyEvent &);
MouseEventImpl convert(const sf::Event::MouseButtonEvent &);
MouseMove convert(const sf::Event::MouseMoveEvent &);
Button convert(const sf::Event::JoystickButtonEvent &);
Axis convert(const sf::Event::JoystickMoveEvent &);

SfmlWidgetRenderer::SfmlWidgetRenderer
    (sf::RenderTarget & target, sf::RenderStates states, SfmlRenderItemMap & items):
    m_target(target),
    m_items(items),
    m_states(states)
{}

void SfmlWidgetRenderer::render_rectangle
    (const Rectangle & rect, StyleValue itemkey, const void *)
{
    auto itr = m_items.find(itemkey);
    if (itr == m_items.end())
        return;
    switch (itr->second.type_id()) {
#   if 0
    case k_item_type_id<SfmlImageResPtr>:
#   endif
    case k_item_type_id<ColorItem>:
        return render_rectangle(rect, itr->second.as<ColorItem>());
#   if 0
    case k_item_type_id<DrawTriangle>:
        throw RtError("SfmlFlatEngine::render_rectangle: Assigned ItemKey "
                      "belonging to a triangle and attempted to draw it as a "
                      "rectangle.");
#   endif
    default:
        throw RtError("SfmlFlatEngine::render_rectangle: (not) Bad branch.");
    }
}

void SfmlWidgetRenderer::render_triangle
    (const Triangle & tuple, StyleValue itemkey, const void *)
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
    default:
        throw RtError("SfmlFlatEngine::render_rectangle: (not) Bad branch.");
    }
}

void SfmlWidgetRenderer::render_text(const TextBase & text_base) {
    const auto * dc_text = dynamic_cast<const asgl::detail::SfmlText *>(&text_base);
    if (!dc_text) return;
    m_target.draw(*dc_text, m_states);
}

void SfmlWidgetRenderer::render_rectangle_pair
    (const Rectangle & first, const Rectangle & second, StyleValue key, const void *)
{
    auto itr = m_items.find(key);
    if (itr == m_items.end()) return;
    switch (itr->second.type_id()) {
    case k_item_type_id<SfmlImageResPtr>: {
        auto ptr = itr->second.as<SfmlImageResPtr>();
        if (!ptr) { throw RtError("somehow null"); }
        return render_rectangle_pair(first, second, *ptr);
    }
    case k_item_type_id<ColorItem>:
        render_rectangle(first, itr->second.as<ColorItem>());
        return render_rectangle(second, itr->second.as<ColorItem>());
    case k_item_type_id<RoundedBorder>:
        return render_rectangle_pair(first, second, itr->second.as<RoundedBorder>());
    case k_item_type_id<SquareBorder>:
        return render_rectangle_pair(first, second, itr->second.as<SquareBorder>());
    default: throw RtError("bad branch");
    }
}

void SfmlWidgetRenderer::render_special
    (StyleValue key, const Widget * instance_pointer)
{
    if (key != asgl::SfmlFlatEngine::to_item_key(asgl::sample_style_values::k_special_draw_item)) {
        throw InvArg("SfmlFlatEngine::render_special: this function should "
                     "only be called with the special draw item key.");
    }
    const auto * as_drawable = dynamic_cast<const sf::Drawable *>(instance_pointer);
    if (!as_drawable) {
        throw InvArg("SfmlFlatEngine::render_special: special rendering "
                     "expects that ");
    }
    m_target.draw(*as_drawable, m_states);
}

/* private */ void SfmlWidgetRenderer::render_rectangle
    (const Rectangle & rect, ColorItem & color_item) const
{
    color_item.update(rect);
    m_target.draw(color_item.rectangle(), m_states);
}

/* private */ void SfmlWidgetRenderer::render_triangle
    (const Triangle & trituple, ColorItem & color_item) const
{
    color_item.update(trituple);
    m_target.draw(color_item.triangle(), m_states);
}

/* private */ void SfmlWidgetRenderer::render_rectangle_pair
    (const Rectangle & front, const Rectangle & back, RoundedBorder & obj) const
{
    using asgl::SfmlFlatEngine;
    SfmlFlatEngine::update_draw_rectangle
        (obj.back_rectangle, Rectangle(front.left, back.top, front.width, back.height));
    m_target.draw(obj.back_rectangle, m_states);

    SfmlFlatEngine::update_draw_rectangle
        (obj.back_rectangle, Rectangle(back.left, front.top, back.width, front.height));
    m_target.draw(obj.back_rectangle, m_states);

    auto draw_circle_at = [&obj, this](float x, float y) {
        auto nstates = m_states;
        nstates.transform.translate(x, y);
        m_target.draw(obj.circle.data(), obj.circle.size(), sf::PrimitiveType::Triangles, nstates);
    };
    // tl, tr, bl, br
    draw_circle_at( float(front.left              ), float(front.top               ) );
    draw_circle_at( float(front.left + front.width), float(front.top               ) );
    draw_circle_at( float(front.left              ), float(front.top + front.height) );
    draw_circle_at( float(front.left + front.width), float(front.top + front.height) );

    SfmlFlatEngine::update_draw_rectangle(obj.front_rectangle, front);
    m_target.draw(obj.front_rectangle, m_states);
}

/* private */ void SfmlWidgetRenderer::render_rectangle_pair
    (const Rectangle & front, const Rectangle & back, SquareBorder & obj) const
{
    using asgl::SfmlFlatEngine;
    SfmlFlatEngine::update_draw_rectangle(obj.back_rectangle, front);
    m_target.draw(obj.back_rectangle, m_states);

    SfmlFlatEngine::update_draw_rectangle(obj.front_rectangle, back);
    m_target.draw(obj.front_rectangle, m_states);
}

/* private */ void SfmlWidgetRenderer::render_rectangle_pair
    (const Rectangle & bounds, const Rectangle & txrect, SfmlImageResource & obj) const
{
    obj.sprite.setTextureRect(convert_to_sfml_rectangle(txrect));
    obj.sprite.setPosition(float(bounds.left), float(bounds.top));
    obj.sprite.setScale(float( bounds.width ) / float(txrect.width ),
                        float( bounds.height) / float(txrect.height));
    m_target.draw(obj.sprite, m_states);
}

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
    case sf::Event::JoystickButtonPressed:
        return Event(ButtonPress  (convert(sfevent.joystickButton)));
    case sf::Event::JoystickButtonReleased:
        return Event(ButtonRelease(convert(sfevent.joystickButton)));
    case sf::Event::JoystickMoved:
        return Event(AxisMove(convert(sfevent.joystickMove)));
    default: return Event();
    }
}

RoundedBorder make_rounded_border(sf::Color back, sf::Color front, int padding) {
    assert(padding >= 0);
    RoundedBorder border;
    border.back_rectangle.set_color(back);
    border.front_rectangle.set_color(front);

    auto to_v = [padding](float f)
        { return sf::Vector2f(std::cos(f), std::sin(f))*float(std::max(0, padding - 1)); };
    border.circle.reserve(padding*3*3);
    static constexpr const float k_full_rot = 3.14159265f * 2.f;
    float t    = 0.f;
    float step = k_full_rot / float(padding*3);
    sf::Vertex vtx;
    vtx.color = back;
    for (int i = 0; i != padding*3; ++i) {
        auto next_t = std::min(k_full_rot, t + step);
        vtx.position = sf::Vector2f();
        border.circle.push_back(vtx);
        vtx.position = to_v(t);
        border.circle.push_back(vtx);
        vtx.position = to_v(next_t);
        border.circle.push_back(vtx);
        t = next_t;
    }
    return border;
}
#if 0
// I'd like to add this later to the sample rendering options...
SquareBorder make_square_border(sf::Color back, sf::Color front) {
    SquareBorder border;
    border.back_rectangle.set_color(back);
    border.front_rectangle.set_color(front);
    return border;
}
#endif
SfmlRenderItem to_color_item(sf::Color color) {
     return SfmlRenderItem( asgl::SfmlFlatEngine::ColorItem(color) );
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
    throw RtError("impossible branch");
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

Button convert(const sf::Event::JoystickButtonEvent & jbe) {
    Button rv;
    rv.button = jbe.button;
    rv.gamepad_id = jbe.joystickId;
    return rv;
}

Axis convert(const sf::Event::JoystickMoveEvent & jme) {
    Axis rv;
    rv.gamepad_id = jme.joystickId;
    rv.axis_id    = jme.axis;
    rv.position   = double(jme.position / 100.f);
    return rv;
}

} // end of <anonymous> namespace
