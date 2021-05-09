/****************************************************************************

    File: TextArea.cpp
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

#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>

#include <cmath>

namespace {

using RtError = std::runtime_error;
using VectorI = asgl::Widget::VectorI;
using UString = asgl::TextArea::UString;

} // end of <anonymous> namespace

namespace asgl {

VectorI TextArea::location() const {
    return VectorI( int(std::round(m_draw_text.location().x)),
                    int(std::round(m_draw_text.location().y)) );
}

int TextArea::width() const
    { return int(std::round(m_draw_text.width())); }

int TextArea::height() const
    { return int(std::round(m_draw_text.height())); }

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

// most of the demos I think, let text set its own size
void TextArea::set_max_width(int w)
    { m_draw_text.set_limiting_line(w);/* m_draw_text.set_limiting_width(float(w)); */}

void TextArea::set_max_height(int /*h*/)
    { /*m_draw_text.set_limiting_height(float(h));*/ }

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

/* private */ void TextArea::draw_(WidgetRenderer & target) const {
    m_draw_text.draw_to(target);
}

/* private */ void TextArea::issue_auto_resize() {}

/* private */ void TextArea::on_geometry_update() {
    m_draw_text.set_location(m_draw_text.location().x, m_draw_text.location().y);
}

} // end of asgl namespace
