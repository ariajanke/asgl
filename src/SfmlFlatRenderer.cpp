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

#include <asgl/SfmlFlatRenderer.hpp>

// use most controls
#include <asgl/Button.hpp>
#include <asgl/ArrowButton.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/TextArea.hpp>
#include <asgl/ProgressBar.hpp>

#include <asgl/Frame.hpp>
#include <asgl/Text.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

namespace {

using SharedImagePtr = asgl::SharedImagePtr;
using RtError        = std::runtime_error;
using InvArg         = std::invalid_argument;

template <typename T>
constexpr const int k_item_type_id = asgl::SfmlRenderItem::GetTypeId<T>::k_value;

} // end of <anonymous> namespace

namespace asgl {

void SfmlFlatEngine::assign_target_and_states
    (sf::RenderTarget & target, sf::RenderStates states)
{
    m_target_ptr = &target;
    m_states = states;
}

void SfmlFlatEngine::stylize(Widget & widget) const {
    widget.stylize(m_style_map);
}

template <typename T>
StyleField to_field(const T & obj) {
    return StyleField(obj);
}

template <>
StyleField to_field<flat_colors::ItemColorEnum>
    (const flat_colors::ItemColorEnum & obj)
{
    return StyleField(SfmlFlatEngine::ItemStyles::to_key(obj));
}

static SfmlRenderItem to_field(sf::Color color) {
    DrawRectangle drect;
    drect.set_color(color);
    return SfmlRenderItem(drect);
}

void SfmlFlatEngine::setup_default_styles() {
    using namespace flat_colors;
    static constexpr const int k_chosen_padding = 3;
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
#   if 0
    stylemap.add(TextArea::to_key(TextArea::k_text_size ), StyleField(18));
    stylemap.add(TextArea::to_key(TextArea::k_text_color), StyleField(sf::Color::White));
#   endif
    }

    m_font_handler->add_font_style(to_item_key(k_title_text ), 22, sf::Color::White);
    m_font_handler->add_font_style(to_item_key(k_widget_text), 18, sf::Color::White);

    using sf::Color;
    m_items[to_item_key(k_primary_light  )] = to_field(Color(0x51, 0x51, 0x76));
    m_items[to_item_key(k_primary_mid    )] = to_field(Color(0x18, 0x18, 0x40));
    m_items[to_item_key(k_primary_dark   )] = to_field(Color(0x08, 0x08, 0x22));
    m_items[to_item_key(k_secondary_light)] = to_field(Color(0x77, 0x6A, 0x45));
    m_items[to_item_key(k_secondary_mid  )] = to_field(Color(0x4B, 0x46, 0x15));
    m_items[to_item_key(k_secondary_dark )] = to_field(Color(0x30, 0x2C, 0x05));
    m_items[to_item_key(k_text_color     )] = [](){
        DrawTriangle dtri;
        dtri.set_color(sf::Color::White);
        return SfmlRenderItem(dtri);
    }();// to_field(Color(0xFF, 0xFF, 0xFF));
    m_items[to_item_key(k_text_color_dark)] = to_field(Color(0x40,    0,    0));

    m_first_setup_done = true;
}

ItemKey SfmlFlatEngine::add_rectangle_style(sf::Color color, StyleKey stylekey) {
    auto item_key = m_item_key_creator.make_key();
    m_items[item_key] = to_field(color);
    m_style_map.add(stylekey, StyleField(item_key));
    return item_key;
}

/* static */ const sf::Texture * SfmlFlatEngine::dynamic_cast_to_texture
    (SharedImagePtr ptr)
{
    auto downcasted = std::dynamic_pointer_cast<SfmlImageResource>(ptr);
    if (!downcasted) return nullptr;
    return &downcasted->texture;
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
    case k_item_type_id<DrawRectangle>:
        return render_rectangle(rect, itr->second.as<DrawRectangle>());
    case k_item_type_id<DrawTriangle>:
        throw RtError("SfmlFlatEngine::render_rectangle: Assigned ItemKey "
                      "belonging to a triangle and attempted to draw it as a "
                      "rectangle.");
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
    case k_item_type_id<DrawRectangle>:
        throw RtError("SfmlFlatEngine::render_rectangle: Assigned ItemKey "
                      "belonging to a rectangle and attempted to draw it as a "
                      "triangle.");
    case k_item_type_id<DrawTriangle>:
        render_triangle(tuple, itr->second.as<DrawTriangle>());
        break;
    default:
        throw RtError("SfmlFlatEngine::render_rectangle: Bad branch.");
    }
}

/* private */ void SfmlFlatEngine::render_text(const TextBase & text_base) {
    const auto * dc_text = dynamic_cast<const SfmlTextN *>(&text_base);
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
    (const sf::IntRect & rect, DrawRectangle & drect) const
{
    drect = DrawRectangle(
        float(rect.left ), float(rect.top   ),
        float(rect.width), float(rect.height), drect.color());
    m_target_ptr->draw(drect, m_states);
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
    (const TriangleTuple & trituple, DrawTriangle & triangle) const
{
    using std::get;
    triangle.set_point_a(sf::Vector2f(std::get<0>(trituple)));
    triangle.set_point_b(sf::Vector2f(std::get<1>(trituple)));
    triangle.set_point_c(sf::Vector2f(std::get<2>(trituple)));
    m_target_ptr->draw(triangle, m_states);
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
