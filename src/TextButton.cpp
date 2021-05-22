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

#include <asgl/TextButton.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>

#include <cmath>

namespace {

using UString = asgl::TextButton::UString;

} // end of <anonymous> namespace

namespace asgl {

void TextButton::set_string(const UString & str)
    { m_text.set_string(str); }

void TextButton::set_string(UString && str)
    { m_text.set_string(std::move(str)); }

UString TextButton::give_cleared_string()
    { return m_text.give_cleared_string(); }

void TextButton::stylize(const StyleMap & stylemap) {
    Button::stylize(stylemap);
    TextArea::set_required_text_fields(
        m_text, stylemap.find(styles::k_global_font),
        stylemap.find(Frame::to_key(Frame::k_widget_text_style)),
        "TextArea::stylize");
}

const UString & TextButton::string() const { return m_text.string(); }

void TextButton::draw(WidgetRenderer & target) const {
#   if 1
    Button::draw(target);
#   endif
#   if 0
    sf::IntRect outer_bounds(location(), VectorI(width(), height()));
    sf::IntRect inner_bounds( location() + VectorI(1, 1)*padding(), VectorI(width(), height()) - VectorI(2, 2)*padding() );
    target.render_rectangle_pair(inner_bounds, outer_bounds, ItemKey(), this);
#   endif
    m_text.draw_to(target);
}
#if 0
/* private */ void TextButton::issue_auto_resize() {

}
#endif

/* private */ void TextButton::set_location_(int x, int y) {
    Button::set_location_(x, y);
    m_text.set_location(x + padding(), y + padding());
}

/* private */ void TextButton::update_size() {
    set_button_frame_size(m_text.width() + padding()*2, m_text.height() + padding()*2);
#   if 0
    // this is now a pure virtual function
    // (ends up being Widget::update_size()
    Button::update_size();
#   endif
}

} // end if asgl namespace
