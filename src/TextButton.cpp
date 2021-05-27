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

namespace {

using std::get;
static constexpr const int k_big    = 0;
static constexpr const int k_small = 1;

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
        stylemap.find(asgl::to_key(frame_styles::k_widget_text_style)),
        "TextArea::stylize");
}

const UString & TextButton::string() const { return m_text.string(); }

void TextButton::draw(WidgetRenderer & target) const {
    Button::draw(target);
    m_text.draw_to(target);
}

/* private */ void TextButton::set_location_(int x, int y) {
    Button::set_location_(x, y);
    auto inpad = inner_padding();
    m_text.set_location(x + padding() + get<k_small>(inpad)
                       ,y + padding() + get<k_big  >(inpad));
}

/* private */ void TextButton::update_size() {
    auto inpad = inner_padding();
    set_button_frame_size
        (m_text.width () + padding()*2 + get<k_small>(inpad)*2
        ,m_text.height() + padding()*2 + get<k_small>(inpad) + get<k_big>(inpad));
}

std::tuple<int, int> TextButton::inner_padding() const {
    int full_pad = std::max(1, m_text.height() / 4);
    return std::make_tuple(full_pad - (full_pad / 2), full_pad / 2);
}

} // end if asgl namespace
