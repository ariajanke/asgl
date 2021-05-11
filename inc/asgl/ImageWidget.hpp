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

#include <memory>

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

    VectorI location() const override;

    int width() const override;

    int height() const override;

    void set_size(int w, int h);

    int image_width() const override;

    int image_height() const override;

    void set_view_rectangle(sf::IntRect) override;

    void draw(WidgetRenderer &) const override;

private:
    void process_event(const Event &) override {}

    void on_geometry_update() override {}

    ItemKey item_key() const override;

    ImageResource & verify_image_present();

    const ImageResource & verify_image_present() const;

    void set_location_(int x, int y) override;

    SharedImagePtr m_image;
    sf::IntRect m_bounds;
};

} // end of asgl namespace
