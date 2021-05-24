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
#include <common/DrawTriangle.hpp>
#include <common/SubGrid.hpp>
#include <asgl/DrawTriangle.hpp>

namespace asgl {

using cul::DrawRectangle;
using cul::DrawTriangle;
template <typename T>
using ConstSubGrid = cul::ConstSubGrid<T>;

namespace detail {

class SfmlFont;
class SfmlImageResource;

} // end of detail namespace -> into ::asgl

namespace sfml_items {

enum ItemColorEnum {
    k_primary_light,
    k_primary_mid,
    k_primary_dark,

    k_secondary_light,
    k_secondary_mid,
    k_secondary_dark,

    k_mono_light,
    k_mono_dark,

    k_color_count
};

enum ItemEnum {
    k_title_text,
    k_widget_text,

    k_editable_text_fill,
    k_editable_text_empty,

    k_bordered_regular_widget,
    k_bordered_hover_widget,
    k_bordered_focus_widget,
    k_bordered_hover_and_focus_widget,

    k_item_count
};

}  // end of sfml_items namespace -> into ::asgl

// ---------------------- BEGINNING OF PUBLIC INTERFACE -----------------------

class SfmlFlatEngine final : public WidgetRenderer, public ImageLoader {
public:
    void assign_target_and_states(sf::RenderTarget &, sf::RenderStates);

    void stylize(Widget &) const;

    void setup_default_styles();

    ItemKey add_rectangle_style(sf::Color, StyleKey);

    void load_global_font(const std::string & filename);

    SharedImagePtr make_image_from(ConstSubGrid<sf::Color>);

    static const sf::Texture * dynamic_cast_to_texture(SharedImagePtr);

    static Event convert(const sf::Event &);

    // ----------------------- END OF PUBLIC INTERFACE ------------------------
    //                         (     entire file     )

    class ColorItem {
    public:
        explicit ColorItem(sf::Color);

        void update(const Rectangle &);
        void update(const Triangle  &);

        const DrawRectangle & rectangle() const { return m_rectangle; }
        const DrawTriangle  & triangle () const { return m_triangle ; }

    private:
        DrawRectangle m_rectangle;
        DrawTriangle m_triangle;
    };

    class RoundedBorder {
    public:
        std::vector<sf::Vertex> circle;
        DrawRectangle back_rectangle;
        DrawRectangle front_rectangle;
    };

    class SquareBorder {
    public:
        DrawRectangle back_rectangle;
        DrawRectangle front_rectangle;
    };

    using SfmlImageResource = detail::SfmlImageResource;
    using SfmlImageResPtr   = std::shared_ptr<SfmlImageResource>;
    using SfmlRenderItem    = cul::MultiType<ColorItem, SfmlImageResPtr, RoundedBorder, SquareBorder>;
    using SfmlRenderItemMap = std::map<ItemKey, SfmlRenderItem>;
    using ItemColorEnum     = sfml_items::ItemColorEnum;
    using ItemEnum          = sfml_items::ItemEnum;
    using ColorItemStyles   = styles::ItemKeysEnum<ItemColorEnum, sfml_items::k_color_count>;
    // going to change name... not now until refactoring is done
    using DescItemStyles    = styles::ItemKeysEnum<ItemEnum, sfml_items::k_item_count>;

    static void update_draw_rectangle(DrawRectangle &, const Rectangle &);

    inline static ItemKey to_item_key(sfml_items::ItemColorEnum e)
        { return ColorItemStyles::to_key(e); }

    inline static ItemKey to_item_key(sfml_items::ItemEnum e)
        { return DescItemStyles::to_key(e); }

private:
    void render_rectangle(const Rectangle &, ItemKey, const void *) final;
    void render_triangle (const Triangle  &, ItemKey, const void *) final;
    void render_text(const TextBase &) final;

    void render_rectangle_pair(const Rectangle &, const Rectangle &, ItemKey, const void *) final;

    void render_rectangle(const Rectangle &, ColorItem &) const;
    void render_triangle (const Triangle  &, ColorItem &) const;

    void render_rectangle_pair(const Rectangle &, const Rectangle &, RoundedBorder &) const;
    void render_rectangle_pair(const Rectangle &, const Rectangle &, SquareBorder &) const;
    void render_rectangle_pair(const Rectangle &, const Rectangle &, SfmlImageResource &) const;

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

    ItemKey item_key() const override { return item; }

    sf::Sprite  sprite;
    sf::Texture texture;
    ItemKey     item;
};

} // end of detail namespace -> into ::asgl

} // end of asgl namespace
