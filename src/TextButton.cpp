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

/* private */ void TextButton::issue_auto_resize() {
    set_size(int(std::round( m_text.width () )) + padding()*2,
             int(std::round( m_text.height() )) + padding()*2);
}

/* private */ void TextButton::draw_(WidgetRenderer & target) const {
    Button::draw_(target);
    m_text.draw_to(target);
}

/* private */ void TextButton::on_geometry_update() {
    Button::on_geometry_update();
    m_text.set_location(float( location().x + padding() ),
                        float( location().y + padding() ));
}

} // end if asgl namespace
