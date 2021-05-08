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

#include <cmath>

namespace {

using UString = asgl::TextButton::UString;

} // end of <anonymous> namespace

namespace asgl {

void TextButton::set_string(const UString & str)
    { m_text.set_string(str); }

void TextButton::set_string(UString && str)
    { m_text.set_string(std::move(str)); }

UString TextButton::take_cleared_string()
    { return m_text.take_cleared_string(); }

void TextButton::stylize(const StyleMap & stylemap) {
    Button::stylize(stylemap);

    TextArea::set_required_text_fields(
        m_text, stylemap.find(styles::k_global_font),
        stylemap.find(TextArea::to_key(TextArea::k_text_color)),
        stylemap.find(TextArea::to_key(TextArea::k_text_size)),
        "TextButton::stylize");
}

const UString & TextButton::string() const { return m_text.string(); }

/* private */ void TextButton::set_size_back(int, int) {

}

/* private */ void TextButton::issue_auto_resize() {
    set_size(int(std::round( m_text.width () )) + padding()*2,
             int(std::round( m_text.height() )) + padding()*2);
}

/* private */ void TextButton::on_size_changed(int, int) {

}

/* private */ void TextButton::draw_(WidgetRenderer & target) const {
    Button::draw_(target);
    target.render_text(m_text);
}

/* private */ void TextButton::on_geometry_update() {
    m_text.set_location(float( location().x + padding() ),
                        float( location().y + padding() ));
}

} // end if asgl namespace

#if 0
#include <ksg/TextButton.hpp>
#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace ksg {

/* static */ constexpr const char * const TextButton::k_text_color;
/* static */ constexpr const char * const TextButton::k_text_size ;

TextButton::TextButton() {}

void TextButton::swap_string(UString & str) {
    m_text.set_string(std::move(str));
    update_string_position();
}

void TextButton::set_string(const UString & str) {
    UString t(str);
    swap_string(t);
}

void TextButton::set_style(const StyleMap & smap) {
    set_if_present(m_text, smap, styles::k_global_font, k_text_size, k_text_color);
    Button::set_style(smap);
    update_string_position();
}

void TextButton::set_location(float x, float y) {
    Button::set_location(x, y);
    update_string_position();
}

void TextButton::issue_auto_resize() {
    if (width() != 0.f || height() != 0.f) return;
    update_text_geometry(std::numeric_limits<float>::infinity(),
                         std::numeric_limits<float>::infinity());
    set_button_frame_size(m_text.width () + padding()*4.f,
                          m_text.line_height() + padding()*4.f);
    update_string_position();
}

/* private */ void TextButton::set_size_back(float w, float h) {
    assert(w > 0.f && h > 0.f);
    update_text_geometry(w, h);
}

/* private */ void TextButton::update_text_geometry(float w, float h) {
    // it is permissible that w and/or h be set to zero
    assert(w > 0.f && h > 0.f);
    m_text.set_limiting_dimensions(w, h);

    w = std::min(2.f*padding() + m_text.width (), w);
    h = std::min(2.f*padding() + m_text.height(), h);
}

/* private */ void TextButton::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    Button::draw(target, states);
    target.draw(m_text);
}

/* private */ void TextButton::on_size_changed(float, float) {
    m_text.set_limiting_dimensions
        (std::max(width()  - 2.f*padding(), 0.f),
         std::max(height() - 2.f*padding(), 0.f));
}

/* private */ void TextButton::update_string_position() {
    if (m_text.width() == 0.f || m_text.height() == 0.f) return;

    float width_diff  = width()  - padding()*2.f - m_text.width();
    float height_diff = height() - padding()*2.f - m_text.height();
    VectorF offset(std::max(0.f, width_diff/2.f),
                   std::max(0.f, height_diff/2.f));
    m_text.set_location(location().x + padding() + offset.x,
                        location().y + padding() + offset.y);
}

} // end of ksg namespace
#endif
