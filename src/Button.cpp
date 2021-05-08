/****************************************************************************

    File: Button.cpp
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
#include <ksg/Button.hpp>
#include <ksg/Frame.hpp>
#endif
#include <asgl/Button.hpp>
#include <asgl/Frame.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

using StyleMap = asgl::StyleMap;
using InvArg   = std::invalid_argument;
using RtError  = std::runtime_error;
using VectorI  = asgl::Widget::VectorI;
#if 0
bool is_in_drect(int x, int y, const DrawRectangle & drect) {
    return (x >= drect.x() && y >= drect.y()
            && x <= drect.x() + drect.width()
            && y <= drect.y() + drect.height());
}
#endif
#if 0
bool is_click_in(const sf::Event::MouseButtonEvent & mouse,
              const DrawRectangle & drect)
{ return is_in_drect(mouse.x, mouse.y, drect); }

bool is_mouse_in(const sf::Event::MouseMoveEvent & mouse,
              const DrawRectangle & drect)
{ return is_in_drect(mouse.x, mouse.y, drect); }
#endif
} // end of <anonymous> namespace

namespace asgl {
#if 0
/* static */ constexpr const char * const Button::k_hover_back_color;
/* static */ constexpr const char * const Button::k_hover_front_color;
/* static */ constexpr const char * const Button::k_regular_back_color;
/* static */ constexpr const char * const Button::k_regular_front_color;
#endif
void Button::process_event(const /*sf::*/Event & evnt) {
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

/* private */ void Button::set_location_(int x, int y) {
#   if 0
    float old_x = location().x, old_y = location().y;
    m_outer.set_position(x, y);
    m_inner.set_position(x + std::max(m_padding, 0.f), y + std::max(m_padding, 0.f));

    set_button_frame_size(width(), height());
    on_location_changed(old_x, old_y);
#   endif
    m_back.left = x;
    m_back.top  = y;
}

void Button::stylize(const StyleMap & smap) {
#   if 0
    using namespace styles;

    set_if_found(smap, k_hover_back_color     , m_hover.back );
    set_if_found(smap, k_hover_front_color    , m_hover.front);
    set_if_found(smap, k_regular_back_color   , m_reg.back   );
    set_if_found(smap, k_regular_front_color  , m_reg.front  );
    set_if_found(smap, k_global_padding       , m_padding    );

    m_outer.set_color(m_reg.back);
    m_inner.set_color(m_reg.front);
#   endif
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
}

void Button::set_press_event(BlankFunctor && func) {
    m_press_functor = std::move(func);
}

void Button::press() {
    if (m_press_functor)
        m_press_functor();
}

VectorI Button::location() const
    { return VectorI(m_back.left, m_back.top); }

void Button::set_size(int width_, int height_) {
    Helpers::verify_non_negative(width_ , "Button::set_size", "width" );
    Helpers::verify_non_negative(height_, "Button::set_size", "height");
#   if 0
    if (width_ <= 0 || height_ <= 0) {
        throw InvArg("asgl::Button::set_size: width and height must be "
                     "positive real numbers (which excludes zero)."    );
    }
#   endif

    float old_width = width(), old_height = height();

    set_button_frame_size(width_, height_);
    set_size_back(width_, height_);
    on_size_changed(old_width, old_height);
    set_needs_geometry_update_flag();
}

int Button::width() const { return m_back.width; }

int Button::height() const { return m_back.height; }

/* protected */ Button::Button() {}
#if 0
/* protected */ void Button::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    target.draw(m_outer);
    target.draw(m_inner);
}
#endif

/* protected */ void Button::draw_(WidgetRenderer & target) const {
    draw_to(target, m_back , m_active_items.back );
    if (m_front.width <= 0 || m_front.height <= 0) return;
    draw_to(target, m_front, m_active_items.front);
}

/* protected */ void Button::on_geometry_update() {
    if (m_back.width > m_padding*2) {
        m_front.left  = m_back.left  + m_padding;
        m_front.width = m_back.width - m_padding*2;
    }
    if (m_back.height > m_padding*2) {
        m_front.top    = m_back.top    + m_padding;
        m_front.height = m_back.height - m_padding*2;
    }
}

/* protected */ void Button::on_size_changed(int, int) { }

/* protected */ void Button::on_location_changed(int, int) { }

/* protected */ void Button::set_size_back(int, int) { }

/* protected */ void Button::set_button_frame_size
    (int width_, int height_)
{
    Helpers::verify_non_negative(width_ , "Button::set_button_frame_size", "width" );
    Helpers::verify_non_negative(height_, "Button::set_button_frame_size", "height");
    m_back.width  = width_;
    m_back.height = height_;
#   if 0
    m_outer.set_size(width_, height_);
    m_inner.set_size(std::max(width_  - m_padding*2.f, 0.f),
                     std::max(height_ - m_padding*2.f, 0.f));
#   endif
}

/* protected */ void Button::deselect() {
    m_is_highlighted = false;
    m_active_items.front = m_regular_items.front;
    if (has_focus()) {
        m_active_items.back = m_hover_items.front;
    } else {
        m_active_items.front = m_regular_items.back;
    }
#   if 0
    m_inner.set_color(m_reg.front);
    if (has_focus()) {
        m_outer.set_color(m_hover.front);
    } else {
        m_outer.set_color(m_reg.back);
    }
#   endif
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
#   if 0
    m_inner.set_color(m_hover.front);
    if (has_focus()) {
        m_outer.set_color(m_hover.front);
    } else {
        m_outer.set_color(m_hover.back);
    }
#   endif
    set_needs_redraw_flag();
}

/* private */ void Button::process_focus_event(const Event & event) {
#   if 0
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Return) {
            m_press_functor();
        }
    }
#   endif
    if (event.is_type<KeyRelease>()) {
        if (event.as<KeyRelease>().key == keys::k_enter) {
            m_press_functor();
        }
    }
}

/* private */ void Button::notify_focus_gained() {
#   if 0
    m_outer.set_color(m_hover.front);
#   endif
    m_active_items.back = m_hover_items.front;
}

/* private */ void Button::notify_focus_lost() {
#   if 0
    m_outer.set_color(m_reg.back);
#   endif
    m_active_items.back = m_regular_items.back;
}

} // end of ksg namespace
