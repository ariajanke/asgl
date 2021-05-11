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

// !!! KEEP THIS !!!
#if 0
#include <ksg/EditableText.hpp>
#include <ksg/Button.hpp>
#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace {

using VectorF = ksg::EditableText::VectorF;
using UString = ksg::EditableText::UString;

sf::FloatRect to_rect(const DrawRectangle &);

} // end of <anonymous> namespace

namespace ksg {

namespace detail {

void Ellipsis::set_size(float w, float h) {
    m_back.set_size(w, h);
    update_dots();
}

void Ellipsis::set_location(float x, float y) {
    m_back.set_position(x, y);
    update_dots();
}

/* static */ VectorF Ellipsis::point_a_for(int i) {
    float y = (i / 3) ? k_bottom_line : k_top_line;
    switch (i % 3) {
    case 0: return VectorF(k_hmargin + 1*k_inner_step, y);
    case 1: return VectorF(k_hmargin + 4*k_inner_step, y);
    case 2: return VectorF(k_hmargin + 7*k_inner_step, y);
    }
    throw; // reaching here is not possible, but the compiler thinks so
}

/* static */ VectorF Ellipsis::point_b_for(int i) {
    switch (i % 3) {
    case 0: return VectorF(k_hmargin + 0*k_inner_step, 0.5f);
    case 1: return VectorF(k_hmargin + 3*k_inner_step, 0.5f);
    case 2: return VectorF(k_hmargin + 6*k_inner_step, 0.5f);
    }
    throw; // reaching here is not possible, but the compiler thinks so
}

/* static */ VectorF Ellipsis::point_c_for(int i) {
    switch (i % 3) {
    case 0: return VectorF(k_hmargin + 2*k_inner_step, 0.5f);
    case 1: return VectorF(k_hmargin + 5*k_inner_step, 0.5f);
    case 2: return VectorF(k_hmargin + 8*k_inner_step, 0.5f);
    }
    throw; // reaching here is not possible, but the compiler thinks so
}

/* private */ void Ellipsis::update_dots() {
    m_back.set_color(sf::Color(100, 100, 100));

    auto denormalize = [this](VectorF r) {
        r.x *= m_back.width();
        r.y *= m_back.height();
        return r + m_back.position();
    };
    for (int i = 0; i != k_dot_shape_count; ++i) {
        m_dots[i].set_color(sf::Color(30, 40, 10));
        m_dots[i].set_point_a(denormalize(point_a_for(i)));
        m_dots[i].set_point_b(denormalize(point_b_for(i)));
        m_dots[i].set_point_c(denormalize(point_c_for(i)));
    }
}

/* private */ void Ellipsis::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    target.draw(m_back, states);
    for (const auto & tri : m_dots) {
        target.draw(tri);
    }
}

} // end of detail namespace

// ----------------------------------------------------------------------------

/* static */ constexpr const char * const EditableText::k_background_color   ;
/* static */ constexpr const char * const EditableText::k_ellipsis_back_color;

EditableText::EditableText() {}

void EditableText::process_event(const sf::Event & event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        VectorF pos(float(event.mouseButton.x), float(event.mouseButton.y));
        if (to_rect(m_outer).contains(pos)) {
            request_focus();
        }
    }
}

void EditableText::set_location(float x, float y) {
    m_outer.set_position(x, y);
    update_geometry();
}

VectorF EditableText::location() const
    { return m_outer.position(); }

float EditableText::width() const
    { return m_outer.width(); }

float EditableText::height() const
    { return m_outer.height(); }

void EditableText::set_style(const StyleMap & map) {
    using namespace styles;
    if (!set_if_color_found(map, k_background_color, m_inner)) {
        m_inner.set_color(sf::Color::White);
    }
    set_if_color_found(map, Button::k_regular_back_color, m_outer);
    set_if_found(map, Button::k_regular_front_color, m_reg_color);
    set_if_found(map, Button::k_hover_front_color, m_focus_color);
#   if 0
    temp = m_outer.color();
    if (set_if_found(map, , temp)) {
        m_outer.set_color(temp);
    }
    if (auto * color = find<sf::Color>(map, k_background_color)) {
        m_inner.set_color(*color);
    }
    if (auto * color = find<sf::Color>(map, Button::k_regular_back_color)) {
        m_outer.set_color(*color);
    }
    if (auto * color = find<sf::Color>(map, Button::k_regular_front_color)) {
        m_reg_color = *color;
    }
    if (auto * color = find<sf::Color>(map, Button::k_hover_front_color)) {
        m_focus_color = *color;
    }

    if (auto * pad = find<float>(map, k_global_padding)) {
        m_padding = *pad;
    }
    if (auto * text_size = find<float>(map, TextArea::k_text_size)) {
        m_text.set_character_size(int(*text_size));
    }
#   endif
    set_if_found(map, k_global_padding, m_padding);
    m_inner_padding = 2.f;
    set_if_numeric_found(map, TextArea::k_text_size, m_text);
#   if 0
    float text_size = styles::get_unset_value<float>();
    set_if_found(map, TextArea::k_text_size, text_size);
#   endif
    m_text.assign_font(map, k_global_font);
    update_geometry();
}

void EditableText::set_width(float w) {
    m_outer.set_width(w);
    update_geometry();
}

void EditableText::set_text(const UString & str) {
    m_text.set_string(str);
    update_geometry();
}

void EditableText::set_string(const UString & ustr) {
    m_text.set_string(ustr);
    update_geometry();
}

void EditableText::set_cursor_position(int) {

}

int EditableText::character_count() const
    { return m_text.character_size(); }

const UString & EditableText::text() const
    { return m_text.string(); }

const UString & EditableText::string() const
    { return m_text.string(); }

void EditableText::set_character_size(int size)
    { m_text.set_character_size(size); }

void EditableText::set_character_filter(CharFilterFunc && f)
    { m_filter_func = std::move(f); }

void EditableText::set_text_change_event(BlankFunc && f)
    { m_change_text_func = std::move(f); }

float EditableText::max_text_width() const
    { return width() - padding()*2.f - inner_padding()*2.f; }

void EditableText::process_focus_event(const sf::Event & event) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode < 10) return;

        bool needed_ellipsis = need_ellipsis();
        UString new_string;
        new_string.reserve(1 + m_text.string().size());
        new_string = m_text.string();
        new_string.push_back(event.text.unicode);

        if (m_filter_func(new_string)) {
            m_text.set_string(new_string);
            update_cursor();
            if (need_ellipsis() != needed_ellipsis) {
                update_geometry();
            }
            m_change_text_func();
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::BackSpace && !m_text.string().empty()) {
            auto new_string = m_text.string();
            new_string.pop_back();
            m_text.set_string(new_string);
            update_cursor();
        }
    }
}

void EditableText::notify_focus_gained()
    { m_outer.set_color(m_focus_color); }

void EditableText::notify_focus_lost()
    { m_outer.set_color(m_reg_color); }

/* private */ void EditableText::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    target.draw(m_outer, states);
    target.draw(m_inner, states);
    target.draw(m_text , states);
    if (need_ellipsis()) { target.draw(m_ellipsis, states); }
    if (has_focus    ()) { target.draw(m_cursor  , states); }
}

/* private */ void EditableText::update_geometry() {
    if (width() == 0.f || !m_text.has_font_assigned()) {
        return;
    }

    auto total_height = padding()*2.f + inner_padding()*2.f + m_text.line_height();
    m_outer.set_height(total_height);
    m_inner.set_size(width() - padding()*2.f, total_height - padding()*2.f);
    m_text.set_limiting_height(m_text.line_height());    
    m_text.set_limiting_width(
        max_text_width() - (need_ellipsis() ? m_ellipsis.width() : 0.f));

    auto inner_loc = location() + padding()*VectorF(1.f, 1.f);
    m_inner.set_position(inner_loc);
    m_text .set_location(inner_loc + inner_padding()*VectorF(1.f, 1.f));

    m_ellipsis.set_location(inner_loc.x, inner_loc.y);
    m_ellipsis.set_size(m_inner.height() * 1.5f, m_inner.height());

    update_cursor();
}

/* private */ void EditableText::update_cursor() {
    m_cursor.set_position(m_text.character_location(m_text.string().size()));
    m_cursor.set_size    (m_text.line_height() / 3.f, m_text.line_height());
    m_cursor.set_color   (sf::Color::Black);
}

/* private */ float EditableText::padding() const noexcept
    { return std::max(0.f, m_padding); }

/* private */ float EditableText::inner_padding() const noexcept
    { return std::max(0.f, m_inner_padding); }

} // end of ksg namespace

namespace {

sf::FloatRect to_rect(const DrawRectangle & drect) {
    sf::FloatRect rv;
    rv.top    = drect.y     ();
    rv.left   = drect.x     ();
    rv.width  = drect.width ();
    rv.height = drect.height();
    return rv;
}

} // end of <anonymous> namespace
#endif
