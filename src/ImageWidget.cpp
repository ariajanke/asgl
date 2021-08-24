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

#include <asgl/ImageWidget.hpp>

namespace {

using namespace cul::exceptions_abbr;

} // end of <anonymous> namespace

namespace asgl {

ImageResource::~ImageResource() {}

ImageLoader::~ImageLoader() {}

// ----------------------------------------------------------------------------

SharedImagePtr ImageWidget::load_image
    (ImageLoader & loader, const std::string & filename)
{ return (m_image = loader.make_image_resource(filename)); }

void ImageWidget::set_image(SharedImagePtr resptr) {
    m_image = resptr;
    m_image_rect = Rectangle(0, 0, m_image->image_width(), m_image->image_height());
    // (don't delete quite yet?)
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

void ImageWidget::copy_image_from(ImageLoader & loader, const ImageWidget & rhs)
    { copy_image_from(loader, rhs.m_image); }

void ImageWidget::copy_image_from(ImageLoader & loader, SharedImagePtr resptr)
    { set_image(loader.make_image_resource(resptr)); }

Vector ImageWidget::location() const
    { return Vector(m_bounds.left, m_bounds.top); }

Size ImageWidget::size() const { return size_of(m_bounds); }

void ImageWidget::set_size(Size sz) { set_size(sz.width, sz.height); }

void ImageWidget::set_size(int w, int h) {
    Helpers::verify_non_negative(w, "ImageWidget::set_size", "width" );
    Helpers::verify_non_negative(h, "ImageWidget::set_size", "height");
    m_bounds.width  = w;
    m_bounds.height = h;
}

int ImageWidget::image_width() const
    { return verify_image_present().image_width(); }

int ImageWidget::image_height() const
    { return verify_image_present().image_height(); }

void ImageWidget::set_view_rectangle(Rectangle rect)
    { m_image_rect = rect; }

void ImageWidget::draw(WidgetRenderer & target) const
    { draw_to(target, m_bounds, m_image_rect, item_key()); }

/* private */ StyleValue ImageWidget::item_key() const
    { return verify_image_present().item_key(); }

/* private */ ImageResource & ImageWidget::verify_image_present() {
    const auto & const_this = *this;
    return const_cast<ImageResource &>(const_this.verify_image_present());
}

/* private */ const ImageResource & ImageWidget::verify_image_present() const {
    if (m_image) return *m_image;
    throw RtError("ImageWidget: call failed: image resource pointer has not been set.");
}

/* private */ void ImageWidget::set_location_(int x, int y) {
    m_bounds.left = x;
    m_bounds.top  = y;
}

} // end of asgl namespace
