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

#include <asgl/Button.hpp>

namespace {

using StyleMap = asgl::StyleMap;
using InvArg   = std::invalid_argument;
using RtError  = std::runtime_error;
using VectorI  = asgl::Widget::VectorI;

} // end of <anonymous> namespace

namespace asgl {

void Button::process_event(const Event & evnt) {
    switch (evnt.type_id()) {
    case k_mouse_release_id:
        if (m_is_highlighted && m_back.contains(to_vector(evnt.as<MouseRelease>()))) {
            press();
        }
        break;
    case k_mouse_move_id:
        if (m_back.contains(to_vector(evnt.as<MouseMove>()))) {
            highlight();
        } else {
            deselect();
        }
        break;
    // missing events
    // mouse left, lost focus, resize
    // should all deselect this control
    default:
        break;
    }
#   if 0
    switch (evnt.type) {
    case sf::Event::MouseButtonReleased:
        if (m_is_highlighted && is_click_in(evnt.mouseButton, m_outer)) {
            press();
        }
        break;
    case sf::Event::MouseMoved:
        if (is_mouse_in(evnt.mouseMove, m_outer))
            highlight();
        else
            deselect();
        break;
    case sf::Event::MouseLeft: case sf::Event::LostFocus:
    case sf::Event::Resized:
        deselect();
        break;
    default: break;
    }
#   endif
}

void Button::stylize(const StyleMap & smap) {
    m_padding = Helpers::verify_padding
        (smap.find(to_key(k_button_padding), styles::k_global_padding),
         "Button::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("Button::stylize", {
        make_tuple(&m_regular_items.front, "regular front style",
                   smap.find(m_regular_keys.front, to_key(k_regular_front_style))),
        make_tuple(&m_regular_items.back, "regular back style",
                   smap.find(m_regular_keys.back, to_key(k_regular_back_style))),
        // hover
        make_tuple(&m_hover_items.front, "hover front style",
                   smap.find(m_hover_keys.front, to_key(k_hover_front_style))),
        make_tuple(&m_hover_items.back, "hover back style",
                   smap.find(m_hover_keys.back, to_key(k_hover_back_style))),
    });
    m_active_items = m_regular_items;
}

void Button::set_press_event(BlankFunctor && func)
    { m_press_functor = std::move(func); }

void Button::press() { m_press_functor(); }

VectorI Button::location() const
    { return VectorI(m_back.left, m_back.top); }

void Button::set_size(int width_, int height_) {
    set_button_frame_size(width_, height_);
    set_needs_geometry_update_flag();
}

int Button::width() const { return m_back.width; }

int Button::height() const { return m_back.height; }

void Button::on_geometry_update() {
    if (m_back.width > padding()*2) {
        m_front.left  = m_back.left  + padding();
        m_front.width = m_back.width - padding()*2;
    }
    if (m_back.height > padding()*2) {
        m_front.top    = m_back.top    + padding();
        m_front.height = m_back.height - padding()*2;
    }
}

void Button::draw(WidgetRenderer & target) const {
    draw_to(target, m_back , m_active_items.back );
    if (m_front.width <= 0 || m_front.height <= 0) return;
    draw_to(target, m_front, m_active_items.front);
}

/* protected */ Button::Button() {}

/* protected */ void Button::set_button_frame_size
    (int width_, int height_)
{
    Helpers::verify_non_negative(width_ , "Button::set_button_frame_size", "width" );
    Helpers::verify_non_negative(height_, "Button::set_button_frame_size", "height");
    m_back.width  = width_;
    m_back.height = height_;
}

/* protected */ void Button::deselect() {
    m_is_highlighted = false;
    m_active_items.front = m_regular_items.front;
    if (has_focus()) {
        m_active_items.back = m_hover_items.front;
    } else {
        m_active_items.back = m_regular_items.back;
    }

    set_needs_redraw_flag();
}

/* protected */ void Button::highlight() {
    m_is_highlighted = true;
    m_active_items.front = m_hover_items.front;
    if (has_focus()) {
        m_active_items.back = m_hover_items.front;
    } else {
        m_active_items.back = m_hover_items.back;
    }

    set_needs_redraw_flag();
}

/* private */ void Button::process_focus_event(const Event & event) {
    if (event.is_type<KeyRelease>()) {
        if (event.as<KeyRelease>().key == keys::k_enter) {
            m_press_functor();
        }
    }
}

/* private */ void Button::notify_focus_gained() {
    m_active_items.back = m_hover_items.front;
}

/* private */ void Button::notify_focus_lost() {
    m_active_items.back = m_regular_items.back;
}

/* private */ void Button::set_location_(int x, int y) {
    m_back.left = x;
    m_back.top  = y;
}

} // end of asgl namespace
