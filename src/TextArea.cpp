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

#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>

#include <cmath>

namespace {

using RtError = std::runtime_error;
using VectorI = asgl::Widget::VectorI;
using UString = asgl::TextArea::UString;

} // end of <anonymous> namespace

namespace asgl {

VectorI TextArea::location() const
    { return m_draw_text.location(); }

int TextArea::width() const
    { return m_draw_text.width(); }

int TextArea::height() const
    { return m_draw_text.height(); }

void TextArea::stylize(const StyleMap & stylemap) {
    set_required_text_fields(
        m_draw_text, stylemap.find(styles::k_global_font),
        stylemap.find(Frame::to_key(Frame::k_widget_text_style)),
        "TextArea::stylize");
}

void TextArea::set_string(const UString & str)
    { m_draw_text.set_string(str); }

void TextArea::set_string(UString && str)
    { m_draw_text.set_string(std::move(str)); }

UString TextArea::give_cleared_string()
    { return m_draw_text.give_cleared_string(); }

void TextArea::set_limiting_line(int x_limit) {
    m_draw_text.set_limiting_line(x_limit);
    flag_needs_whole_family_geometry_update();
}

void TextArea::set_viewport(const sf::IntRect & rect) {
    m_draw_text.set_viewport(rect);
    flag_needs_whole_family_geometry_update();
}

void TextArea::reset_viewport() {
    m_draw_text.reset_viewport();
    flag_needs_whole_family_geometry_update();
}

const sf::IntRect & TextArea::viewport() const
    { return m_draw_text.viewport(); }

void TextArea::draw(WidgetRenderer & target) const
    { m_draw_text.draw_to(target); }

/* static */ void TextArea::set_required_text_fields
    (Text & text, const StyleField * font, const StyleField * style_key,
     const char * full_call)
{
    using FontPtr = std::weak_ptr<const Font>;
    auto make_error = [full_call](const char * what)
        { return RtError(std::string(full_call) + ": " + what); };
    if (!font) {
        throw make_error("no font found for style keys.");
    } else if (!font->is_type<FontPtr>()) {
        throw make_error("font item is not a font type (was the wrong key used?)");
    } else if (font->as<FontPtr>().expired()) {
        throw make_error("font pointer has expired (was the engine deleted?)");
    }
    if (!style_key) {
        throw make_error("no text style key found.");
    } else if (!style_key->is_type<ItemKey>()) {
        throw make_error("text style is not an item key.");
    } else if (style_key->as<ItemKey>() == ItemKey()) {
        throw make_error("text style item key is the default (null) key.");
    }
    const auto & font_ = *font->as<FontPtr>().lock();
    text.set_font(font_);
    text.stylize(style_key->as<ItemKey>());
}

/* private */ void TextArea::set_location_(int x, int y)
    { m_draw_text.set_location(float(x), float(y)); }

/* private */ void TextArea::issue_auto_resize() {}

/* private */ void TextArea::update_geometry() {
    m_draw_text.set_location(m_draw_text.location().x, m_draw_text.location().y);
}

} // end of asgl namespace
