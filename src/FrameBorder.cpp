/****************************************************************************

    File: FrameBorder.cpp
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
#if 0
#include <ksg/FrameBorder.hpp>
#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#endif

#include <asgl/FrameBorder.hpp>
#include <asgl/Frame.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <common/Util.hpp>

#if 0
#include <iostream>
#endif

#include <cassert>

namespace {

using VectorI          = asgl::Widget::VectorI;
using Text             = asgl::Text;
#if 0
using MouseButtonEvent = sf::Event::MouseButtonEvent;
#endif
using InvArg           = std::invalid_argument;
#if 0
void update_title_geometry(VectorI location,
                           const DrawRectangle & title_bar, Text * title);

bool mouse_is_inside(MouseButtonEvent mouse, const DrawRectangle &);
#endif

void update_title_geometry(VectorI location,
                           const sf::IntRect & title_bar, Text * title);

} // end of <anonymous> namespace

namespace asgl {

namespace detail {

// <----------------------- Private types for frame -------------------------->
// implemenation here, to keep vtable out of every translation unit
// (revealed by clang static analysis)

// method used as anchor for LineSeperator
LineSeperator::~LineSeperator() {}

void HorizontalSpacer::set_location_(int x_, int y_)
    { m_location = VectorI(x_, y_); }

VectorI HorizontalSpacer::location() const
    { return m_location; }

int HorizontalSpacer::width() const
    { return m_width; }

void HorizontalSpacer::set_width(int w) {
    assert(w >= 0.f);
    m_width = w;
}

} // end of detail namespace

// ----------------------------------------------------------------------------
// ---                             FrameBorder                              ---
#if 0
/* static */ constexpr const float FrameBorder::k_default_padding;
#endif
VectorI FrameBorder::widget_start() const noexcept {
    VectorI offset(outer_padding(), outer_padding() + title_height());
    if (!m_title.string().empty()) {
        offset.y += outer_padding();
    }
    return location() + offset;
}

VectorI FrameBorder::location() const noexcept
    { return VectorI(m_back.left, m_back.top); }

int FrameBorder::width() const noexcept
    { return m_back.width; }

int FrameBorder::height() const noexcept
    { return m_back.height; }

FrameBorder::EventResponseSignal FrameBorder::process_event
    (const Event & event)
{
    check_should_update_drag(event);

    EventResponseSignal rv;
    rv.should_update_geometry = m_recently_dragged;

    if (
#       if 0
        event.type == sf::Event::MouseButtonPressed
        && mouse_is_inside(event.mouseButton, m_back)
#       endif
           event.is_type<MousePress>()
        && m_back.contains(to_vector(event.as<MousePress>()))
        )
    {
#       if 0
        std::cout << "clicked inside; mouse (" << event.mouseButton.x << ", "
                  << event.mouseButton.y << ") bounds (" << m_back.x() << ", "
                  << m_back.y() << ", " << m_back.width() << ", " << m_back.height()
                  << ")" << std::endl;
#       endif
        assert(m_click_in_frame);
        rv.skip_other_events = (m_click_in_frame() == k_skip_other_events);
    }

    return rv;
}

void FrameBorder::set_location(int x, int y) {
#   if 0
    m_back.set_position(x, y);
#   endif
    m_back.left = x;
    m_back.top  = y;
}

void FrameBorder::stylize(const StyleMap & smap) {
    using namespace styles;
    auto make_missing_error = [](const char * field_name) {
        return InvArg("FrameBorder::stylize: missing require field \""
                      + std::string(field_name) + "\".");
    };
    if (auto * font_field = smap.find(k_global_font)) {
        m_title.assign_font(font_field->as<std::shared_ptr<const sf::Font>>());
    } else {
        throw make_missing_error("global font");
    }
    m_outer_padding = Widget::Helpers::verify_padding(smap.find(k_global_padding), "FrameBorder::stylize");

    if (auto * field = smap.find(Frame::to_key(Frame::k_title_size_style))) {
        m_title.set_character_size(field->as<int>());
    } else {
        throw make_missing_error("title size");
    }

    if (auto * field = smap.find(Frame::to_key(Frame::k_title_color_style))) {
#       if 0
        auto a = field->as_pointer<int>();
        auto b = field->as_pointer<std::shared_ptr<const sf::Font>>();
        auto c = field->as_pointer<styles::AutomaticSize>();
        auto d = field->as_pointer<ItemKey>();
        auto e = field->is_valid();

        m_title.set_color(field->as<sf::Color>());
#       endif
        m_title.set_color(sf::Color::White);
    } else {
        throw make_missing_error("title color");
    }
    if (auto * field = smap.find(Frame::to_key(Frame::k_title_bar_style))) {
        m_title_bar_style = Widget::Helpers::verify_item_key_field
            (*field, "FrameBorder::stylize", "k_title_bar_style");
    } else {
        throw make_missing_error("title bar");
    }
#   if 0
    set_if_present(m_title, smap, k_global_font, Frame::k_title_size,
                   Frame::k_title_color);
    set_if_color_found(smap, Frame::k_background_color, m_back);
    set_if_color_found(smap, Frame::k_title_bar_color, m_title_bar);
    set_if_color_found(smap, Frame::k_widget_body_color, m_widget_body);
    if (set_if_found(smap, Frame::k_border_size, m_outer_padding))
        {}
    else if (set_if_found(smap, k_global_padding, m_outer_padding))
        {}
    else if (std::equal_to<float>()(m_outer_padding, get_unset_value<float>())) {
        m_outer_padding = k_default_padding;
    }
#   endif
}

void FrameBorder::set_size(int w, int h) {
    if (w < 0 || h < 0) {
        throw InvArg("FrameBorder::set_size: size values must be non-negative integers.");
    }
    m_back.width  = w;
    m_back.height = h;
#   if 0
    m_back.set_size(w, h);
#   endif
}

void FrameBorder::set_title(const UString & title_text) {
    m_title.set_string(title_text);
    if (title_text.empty()) {
        ignore_drag_events();
    } else {
        watch_for_drag_events();
    }
}

void FrameBorder::set_title_size(int font_size)
    { m_title.set_character_size(font_size); }

void FrameBorder::reset_register_click_event()
    { m_click_in_frame = do_default_click_event; }

void FrameBorder::update_geometry() {
    const auto loc = location();
    auto w = m_back.width;
    auto h = m_back.height;
    const float k_title_bar_height = title_height();
    const float k_title_bar_pad = m_title.is_visible() ? outer_padding() : 0.f;
    if (m_title.is_visible()) {
        m_title_bar.left = loc.x - outer_padding();
        m_title_bar.top  = loc.y - outer_padding();
        m_title_bar.width = w - outer_padding()*2;
        m_title_bar.height = h - outer_padding()*2;

        update_title_geometry(loc, m_title_bar, &m_title);
    }
    m_widget_body.left = loc.x + outer_padding();
    m_widget_body.top  = loc.y + k_title_bar_height + outer_padding() + k_title_bar_pad;

    m_widget_body.width  = w - outer_padding()*2;
    m_widget_body.height = h - (k_title_bar_height + outer_padding()*2 + k_title_bar_pad);
#   if 0
    const auto loc = location();
    auto w = m_back.width();
    auto h = m_back.height();
    const float k_title_bar_height = title_height();
    const float k_title_bar_pad = m_title.is_visible() ? outer_padding() : 0.f;
    if (m_title.is_visible()) {
        m_title_bar.set_position(loc.x + outer_padding(), loc.y + outer_padding());
        m_title_bar.set_size(w - outer_padding()*2.f, k_title_bar_height);
        update_title_geometry(loc, m_title_bar, &m_title);
    }
    m_widget_body.set_position
        (loc.x + outer_padding(),
         loc.y + k_title_bar_height + outer_padding() + k_title_bar_pad);
    auto wid_body_wid = w - outer_padding()*2.f;
    auto wid_body_hei = h - (k_title_bar_height + outer_padding()*2.f + k_title_bar_pad);
    m_widget_body.set_size(wid_body_wid, wid_body_hei);
    assert(is_real(wid_body_hei) && is_real(wid_body_wid));
#   endif
}

int FrameBorder::title_width_accommodation() const noexcept
    { return m_title.string().empty() ? 0 : m_title.width(); }

int FrameBorder::width_available_for_widgets() const noexcept
    { return m_widget_body.width; }

void FrameBorder::set_border_size(float pixels) {
    if (pixels < 0.f) {
        throw std::invalid_argument(
            "FrameBorder::set_border_size: border size must be a "
            "non-negative real number.");
    }
    m_outer_padding = pixels;
}

void FrameBorder::draw(WidgetRenderer & renderer) const {
    if (m_title.string().empty()) return;
    renderer.render_rectangle(m_back, m_title_bar_style, this);
    renderer.render_text     (m_title);
}

/* private */ void FrameBorder::update_drag_position
    (int drect_x, int drect_y)
{
    set_location(float(drect_x), float(drect_y));
    // save and later send signal for geo update to frame, not the most clean
    // solution, just the least worst given the circumstances
    m_recently_dragged = true;
}
#if 0
/* private */ void FrameBorder::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    target.draw(m_back, states);
    target.draw(m_title_bar, states);
    target.draw(m_widget_body, states);

    if (!m_title.string().empty())
        target.draw(m_title, states);
}
#endif
/* private */ int FrameBorder::title_height() const noexcept
    { return m_title.string().empty() ? 0 : m_title.character_size()*2; }

/* private */ void FrameBorder::check_should_update_drag
    (const Event & event)
{
    m_recently_dragged = false;
    switch (event.type_id()) {
    case k_mouse_press_id:
        mouse_click(event.as<MousePress>().x, event.as<MousePress>().y, m_back);
        break;
    case k_mouse_release_id:
        drag_release();
        break;
    case k_mouse_move_id:
        mouse_move(event.as<MouseMove>().x, event.as<MouseMove>().y);
        break;
    default: break;
    }
#   if 0
    switch (event.type) {
    case sf::Event::MouseButtonPressed:
        mouse_click(event.mouseButton.x, event.mouseButton.y, m_title_bar);
        break;
    case sf::Event::MouseButtonReleased:
        drag_release();
        break;
    case sf::Event::MouseMoved:
        mouse_move(event.mouseMove.x, event.mouseMove.y);
        break;
    default: break;
    }
#   endif
}

/* private static */ FrameBorder::ClickResponse
    FrameBorder::do_default_click_event()
    { return k_continue_other_events; }

/* private */ int FrameBorder::outer_padding() const noexcept
    { return std::max(0, m_outer_padding); }

} // end of ksg namespace

namespace {

void update_title_geometry(VectorI location,
                           const sf::IntRect & title_bar, Text * title)
{
    title->set_limiting_dimensions(title_bar.width, title_bar.height);
    VectorI title_offset(
        (title_bar.width  - title->width ()) / 2.f,
        (title_bar.height - title->height()) / 2.f);
    title->set_location(sf::Vector2f(location + title_offset));
}
#if 0
void update_title_geometry(VectorF location, const DrawRectangle & title_bar,
                           Text * title)
{
    title->set_limiting_dimensions(title_bar.width(), title_bar.height());
    VectorF title_offset(
        (title_bar.width () - title->width ()) / 2.f,
        (title_bar.height() - title->height()) / 2.f);
    title->set_location(location + title_offset);
}

bool mouse_is_inside(MouseButtonEvent mouse, const DrawRectangle & drect) {
    return mouse.x >= drect.x() && mouse.x <= drect.x() + drect.width () &&
           mouse.y >= drect.y() && mouse.y <= drect.y() + drect.height();
}
#endif
} // end of <anonymous> namespace
