/****************************************************************************

    File: TextButton.cpp
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
