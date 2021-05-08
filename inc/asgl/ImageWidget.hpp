/****************************************************************************

    File: ImageWidget.hpp
    Author: Aria Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include <asgl/Widget.hpp>

#include <memory>
#if 0
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <common/MultiType.hpp>

#include <ksg/Widget.hpp>
#endif
namespace asgl {

struct ImageResource {
    virtual ~ImageResource();
    virtual int image_width() const = 0;
    virtual int image_height() const = 0;
    virtual void set_view_rectangle(sf::IntRect) = 0;
    virtual ItemKey item_key() const = 0;
};

using SharedImagePtr = std::shared_ptr<ImageResource>;

struct ImageLoader {
    virtual ~ImageLoader();

    virtual SharedImagePtr make_image_resource(const std::string & filename) = 0;
    virtual SharedImagePtr make_image_resource(SharedImagePtr) = 0;
};

class ImageWidget final : public Widget, public ImageResource {
public:
    // this is fine I think, having the engine own all the "items" seems to
    // have been my goal originally with this redesign
    SharedImagePtr load_image(ImageLoader &, const std::string &);

    void set_image(SharedImagePtr);

    void copy_image_from(ImageLoader &, const ImageWidget &);

    void copy_image_from(ImageLoader &, SharedImagePtr);

    void stylize(const StyleMap &) override {}

#   if 0
    using TextureMultiType =
        MultiType<const sf::Texture *, std::shared_ptr<const sf::Texture>, sf::Texture>;

    bool load_from_file(const char * filename) noexcept;

    void load_from_image(const sf::Image & image);

    void set_texture(const sf::Texture & texture_,
                     const sf::IntRect & trect_ = sf::IntRect());

    void set_texture_shared_pointer(std::shared_ptr<const sf::Texture>);

    /** Assigns a pointer to the texture, with the client being responsible for
     *  ownership.
     *  @note the given pointed to object must live at least as long as this
     *        widget, or bad things will happen!
     */
    void assign_texture(const sf::Texture *);

    void reset_texture_rectangle(const sf::IntRect & trect_);

    void set_location(float x, float y) override;
#   endif
    VectorI location() const override;

    int width() const override;

    int height() const override;

    void set_size(int w, int h);

    int image_width() const override;

    int image_height() const override;

    void set_view_rectangle(sf::IntRect) override;

private:
    void process_event(const Event &) override {}

    void on_geometry_update() override {}

    ItemKey item_key() const override;

    ImageResource & verify_image_present();

    const ImageResource & verify_image_present() const;

    void set_location_(int x, int y) override;

    void draw_(WidgetRenderer &) const override;

#   if 0
    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    void check_invarients() const;
#   endif
    SharedImagePtr m_image;
    sf::IntRect m_bounds;
#   if 0
    ItemKey m_image_item_key;

    void update_size_post_load();

    TextureMultiType m_texture_storage;
    sf::Sprite   m_spt;
    sf::Vector2f m_size;
#   endif

};

} // end of ksg namespace
