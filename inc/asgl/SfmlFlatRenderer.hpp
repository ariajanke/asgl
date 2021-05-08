#pragma once

#include <asgl/Widget.hpp>
#include <asgl/ImageWidget.hpp>

#include <SFML/Graphics/Sprite.hpp>

#include <common/DrawRectangle.hpp>
#include <asgl/DrawTriangle.hpp>

namespace asgl {

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

using SfmlImageResPtr = std::shared_ptr<SfmlImageResource>;
using SfmlRenderItem = MultiType<DrawRectangle, DrawTriangle, SfmlImageResPtr>;
using SfmlRenderItemMap = std::map<ItemKey, SfmlRenderItem>;

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
    k_color_count
};

} // end of flat_colors namespace

class SfmlFlatEngine final :
    public WidgetRenderer, public ImageLoader
{
public:
    using ItemColorEnum = flat_colors::ItemColorEnum;
    using ItemStyles = styles::ItemKeysEnum<ItemColorEnum, flat_colors::k_color_count>;

    void assign_target_and_states(sf::RenderTarget &, sf::RenderStates);

    void stylize(Widget &) const;

    void setup_default_styles();

    void add_rectangle_style(sf::Color, StyleKey);

    void add_global_font(std::shared_ptr<const sf::Font>);

    static const sf::Texture * dynamic_cast_to_texture(SharedImagePtr);

private:
    inline static ItemKey to_item_key(flat_colors::ItemColorEnum e)
        { return ItemStyles::to_key(e); }

    void render_rectangle(const sf::IntRect   &, ItemKey, const void *) final;
    void render_triangle (const TriangleTuple &, ItemKey, const void *) final;
    void render_text     (const Text &) final;

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
};

} // end of asgl namespace
