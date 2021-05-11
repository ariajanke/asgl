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

#pragma once

#include <asgl/Widget.hpp>
#include <asgl/ImageWidget.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>

#include <common/DrawRectangle.hpp>
#include <asgl/DrawTriangle.hpp>

namespace asgl {

namespace detail {

class SfmlFont;
class SfmlImageResource;

} // end of detail namespace -> into ::asgl

// ---------------------- BEGINNING OF PUBLIC INTERFACE -----------------------

namespace flat_colors {

enum ItemColorEnum {
    k_primary_light,
    k_primary_mid,
    k_primary_dark,

    k_secondary_light,
    k_secondary_mid,
    k_secondary_dark,

    k_text_color,
    k_text_color_dark,

    k_title_text,
    k_widget_text,

    k_color_count
};

} // end of flat_colors namespace -> into ::asgl

class SfmlFlatEngine final : public WidgetRenderer, public ImageLoader {
public:
    void assign_target_and_states(sf::RenderTarget &, sf::RenderStates);

    void stylize(Widget &) const;

    void setup_default_styles();

    ItemKey add_rectangle_style(sf::Color, StyleKey);

    void load_global_font(const std::string & filename);

    static const sf::Texture * dynamic_cast_to_texture(SharedImagePtr);

    static Event convert(const sf::Event &);

    // ----------------------- END OF PUBLIC INTERFACE ------------------------
    //                         (     entire file     )

    using SfmlImageResource = detail::SfmlImageResource;
    using SfmlImageResPtr   = std::shared_ptr<SfmlImageResource>;
    using SfmlRenderItem    = MultiType<DrawRectangle, DrawTriangle, SfmlImageResPtr>;
    using SfmlRenderItemMap = std::map<ItemKey, SfmlRenderItem>;
    using ItemColorEnum     = flat_colors::ItemColorEnum;
    using ItemStyles        = styles::ItemKeysEnum<ItemColorEnum, flat_colors::k_color_count>;

private:
    inline static ItemKey to_item_key(flat_colors::ItemColorEnum e)
        { return ItemStyles::to_key(e); }

    void render_rectangle(const sf::IntRect   &, ItemKey, const void *) final;
    void render_triangle (const TriangleTuple &, ItemKey, const void *) final;
    void render_text(const TextBase &) final;

    void render_rectangle(const sf::IntRect &, DrawRectangle &) const;
    void render_rectangle(const sf::IntRect &, SfmlImageResource &) const;
    void render_triangle(const TriangleTuple &, DrawTriangle &) const;

    SharedImagePtr make_image_resource(const std::string & filename) final;
    SharedImagePtr make_image_resource(SharedImagePtr) final;

    SfmlRenderItem & add_and_verify_unique(ItemKey);

    sf::RenderTarget * m_target_ptr = nullptr;
    sf::RenderStates m_states;

    SfmlRenderItemMap m_items;

    StyleMap m_style_map;
    styles::ItemKeyCreator m_item_key_creator;

    std::shared_ptr<detail::SfmlFont> m_font_handler;
    bool m_first_setup_done = false;
};

namespace detail {

class SfmlImageResource final : public ImageResource {
public:
    int image_width() const override
        { return int(texture.getSize().x); }

    int image_height() const override
        { return int(texture.getSize().y); }

    void set_view_rectangle(sf::IntRect rect) override
        { sprite.setTextureRect(rect); }

    ItemKey item_key() const override { return item; }

    sf::Sprite  sprite;
    sf::Texture texture;
    ItemKey     item;
};

} // end of detail namespace -> into ::asgl

} // end of asgl namespace
