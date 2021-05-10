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

#include <asgl/FrameBorder.hpp>
#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <common/Util.hpp>

#if 1
#include <iostream>
#endif

#include <cassert>

namespace {

using VectorI = asgl::Widget::VectorI;
using Text    = asgl::Text;
using InvArg  = std::invalid_argument;

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

    if (   event.is_type<MousePress>()
        && m_back.contains(to_vector(event.as<MousePress>()))
        )
    {
#       if 0
        auto loc = to_vector(event.as<MousePress>());
        std::cout << "clicked inside; mouse (" << loc.x << ", "
                  << loc.y << ") bounds (" << m_back.left << ", "
                  << m_back.top << ", " << m_back.width << ", " << m_back.height
                  << ")" << std::endl;
#       endif
        assert(m_click_in_frame);
        rv.skip_other_events = (m_click_in_frame() == k_skip_other_events);
    }

    return rv;
}

void FrameBorder::set_location(int x, int y) {
    m_back.left = x;
    m_back.top  = y;
}

void FrameBorder::stylize(const StyleMap & smap) {
    using namespace styles;
    TextArea::set_required_text_fields(
        m_title, smap.find(styles::k_global_font),
        smap.find(Frame::to_key(Frame::k_title_text_style)),
        "FrameBorder::stylize");

    using std::make_tuple;
    Widget::Helpers::handle_required_fields("FrameBorder::stylize", {
        make_tuple(&m_back_style, "frame border",
                   smap.find(Frame::to_key(Frame::k_title_bar_style))),
        make_tuple(&m_title_bar_style, "title bar",
                   smap.find(Frame::to_key(Frame::k_title_bar_style))),
        make_tuple(&m_widget_body_style, "widget body",
                   smap.find(Frame::to_key(Frame::k_widget_body_style))),
    });
}

void FrameBorder::set_size(int w, int h) {
    if (w < 0 || h < 0) {
        throw InvArg("FrameBorder::set_size: size values must be non-negative integers.");
    }
    m_back.width  = w;
    m_back.height = h;
}

void FrameBorder::set_title(const UString & title_text) {
    m_title.set_string(title_text);
    if (title_text.empty()) {
        ignore_drag_events();
    } else {
        watch_for_drag_events();
    }
}

void FrameBorder::set_title_size(int font_size) {
    (void)font_size;
    throw std::runtime_error("");
}

void FrameBorder::reset_register_click_event()
    { m_click_in_frame = do_default_click_event; }

void FrameBorder::update_geometry() {
    auto title_is_visible = [this] () { return !m_title.string().empty(); };
    const auto loc = location();
    auto w = m_back.width;
    auto h = m_back.height;
    const float k_title_bar_height = title_height();
    const float k_title_bar_pad = title_is_visible() ? outer_padding() : 0.f;
    if (title_is_visible()) {
        m_title_bar.left = loc.x - outer_padding();
        m_title_bar.top  = loc.y - outer_padding();
        m_title_bar.width = w - outer_padding()*2;
        m_title_bar.height = title_height();// h - outer_padding()*2;

        update_title_geometry(loc, m_title_bar, &m_title);
    }

    m_widget_body.left = loc.x + outer_padding();
    m_widget_body.top  = loc.y + k_title_bar_height + outer_padding() + k_title_bar_pad;

    m_widget_body.width  = w - outer_padding()*2;
    m_widget_body.height = h - (k_title_bar_height + outer_padding()*2 + k_title_bar_pad);
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
    auto title_is_visible = [this] () { return !m_title.string().empty(); };
    renderer.render_rectangle(m_back, m_back_style, this);
    renderer.render_rectangle(m_widget_body, m_widget_body_style, this);

    if (title_is_visible()) {
        renderer.render_rectangle(m_title_bar, m_title_bar_style, this);
        m_title.draw_to(renderer);
    }
}

/* private */ void FrameBorder::update_drag_position
    (int drect_x, int drect_y)
{
    set_location(float(drect_x), float(drect_y));
    // save and later send signal for geo update to frame, not the most clean
    // solution, just the least worst given the circumstances
    m_recently_dragged = true;
}

/* private */ int FrameBorder::title_height() const noexcept
    { return m_title.string().empty() ? 0 : /*m_title.character_size()*2*/(m_title.height() * 3) / 2; }

/* private */ void FrameBorder::check_should_update_drag
    (const Event & event)
{
    m_recently_dragged = false;
    switch (event.type_id()) {
    case k_mouse_press_id:
        mouse_click(event.as<MousePress>().x, event.as<MousePress>().y, m_title_bar);
        break;
    case k_mouse_release_id:
        drag_release();
        break;
    case k_mouse_move_id:
        mouse_move(event.as<MouseMove>().x, event.as<MouseMove>().y);
        break;
    default: break;
    }
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
    VectorI title_offset(
        (title_bar.width  - title->width ()) / 2,
        (title_bar.height - title->height()) / 2);
    title->set_location(location + title_offset);
}

} // end of <anonymous> namespace
