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

using namespace cul::exceptions_abbr;
using asgl::StyleMap;
using std::get;
using std::make_tuple;

} // end of <anonymous> namespace

namespace asgl {

void Button::process_event(const Event & evnt) {
    switch (evnt.type_id()) {
    case k_event_id_of<MouseRelease>:
        if (/*m_is_highlighted */ m_is_hovered && is_contained_in(evnt.as<MouseRelease>(), m_back)) {
            press();
        }
        break;
    case k_event_id_of<MouseMove>:
        if (is_contained_in(evnt.as<MouseMove>(), m_back)) {
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

    Helpers::handle_required_fields("Button::stylize", {
        make_tuple(&get<k_regular_idx>(m_items), "regular style",
                   smap.find(get<k_regular_idx>(m_styles), to_key(k_regular_style))),
        make_tuple(&get<k_hover_idx>(m_items), "hover style",
                   smap.find(get<k_hover_idx>(m_styles), to_key(k_hover_style))),
        make_tuple(&get<k_focus_idx>(m_items), "focus style",
                   smap.find(get<k_focus_idx>(m_styles), to_key(k_focus_style))),
        make_tuple(&get<k_hover_and_focus_idx>(m_items), "hover and focus style",
                   smap.find(get<k_hover_and_focus_idx>(m_styles), to_key(k_hover_and_focus_style))),
    });
}

void Button::set_press_event(BlankFunctor && func)
    { m_press_functor = std::move(func); }

void Button::press() { m_press_functor(); }

Vector Button::location() const { return top_left_of(m_back); }

Size Button::size() const { return size_of(m_back); }

void Button::draw_frame(WidgetRenderer & target) const
    { draw_to(target, m_front, m_back, get_active_item()); }

void Button::set_visible(bool b) { m_is_visible = b; }

bool Button::is_visible() const noexcept { return m_is_visible; }

/* protected */ Button::Button() {}

/* protected */ void Button::set_button_frame_size
    (int width_, int height_)
{
    Helpers::verify_non_negative(width_ , "Button::set_button_frame_size", "width" );
    Helpers::verify_non_negative(height_, "Button::set_button_frame_size", "height");
    set_size_of(m_back, width_, height_);
    flag_needs_whole_family_geometry_update();
}

/* protected */ void Button::deselect() {
    m_is_hovered = false;
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

/* protected */ void Button::highlight() {
    m_is_hovered = true;
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

/* protected */ void Button::process_focus_event(const Event & event) {
    if (event.is_type<KeyRelease>()) {
        if (event.as<KeyRelease>().key == keys::k_enter) {
            m_press_functor();
        }
    }
}

/* protected */ void Button::notify_focus_gained()
    { m_is_focused = true; }

/* protected */ void Button::notify_focus_lost()
    { m_is_focused = false; }

/* protected */ void Button::set_location_(int x, int y) {
    set_top_left_of(m_back, x, y);

    if (m_back.width > padding()*2) {
        m_front.left  = m_back.left  + padding();
        m_front.width = m_back.width - padding()*2;
    }
    if (m_back.height > padding()*2) {
        m_front.top    = m_back.top    + padding();
        m_front.height = m_back.height - padding()*2;
    }
}

/* protected */ bool Button::is_visible_for_focus_advance() const
    { return is_visible(); }

/* private */ StyleValue Button::get_active_item() const {
    if (m_is_focused) {
        return m_is_hovered ? get<k_hover_and_focus_idx>(m_items) : get<k_focus_idx>(m_items);
    } else {
        return m_is_hovered ? get<k_hover_idx>(m_items) : get<k_regular_idx>(m_items);
    }
}

/* private static */ Button::KeyTuple<StyleKey> Button::default_styles() {
    KeyTuple<StyleKey> def;
    get<k_regular_idx>(def) = to_key(k_regular_style);
    get<k_hover_idx>(def) = to_key(k_hover_style);
    get<k_focus_idx>(def) = to_key(k_focus_style);
    get<k_hover_and_focus_idx>(def) = to_key(k_hover_and_focus_style);
    return def;
}

} // end of asgl namespace
