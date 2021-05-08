/****************************************************************************

    File: OptionsSlider.cpp
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

#include <asgl/OptionsSlider.hpp>
#include <asgl/TextArea.hpp>

#include <cmath>

namespace {

using VectorI = asgl::Widget::VectorI;
using UString = asgl::Text::UString;

} // end of <anonymous> namespace

namespace asgl {

OptionsSlider::OptionsSlider()
    { set_arrow_events(); }

OptionsSlider::OptionsSlider(const OptionsSlider &)
    { set_arrow_events(); }

OptionsSlider::~OptionsSlider() {}

OptionsSlider & OptionsSlider::operator = (const OptionsSlider & rhs) {
    if (this != &rhs) {
        OptionsSlider temp(rhs);
        swap(temp);
    }
    return *this;
}

OptionsSlider & OptionsSlider::operator = (OptionsSlider && rhs) {
    if (this != &rhs) swap(rhs);
    return *this;
}

void OptionsSlider::process_event(const Event & evnt) {
    m_left_arrow.process_event(evnt);
    m_right_arrow.process_event(evnt);
}

VectorI OptionsSlider::location() const
    { return m_left_arrow.location(); }

int OptionsSlider::width() const
    { return m_inner_bounds.width + m_left_arrow.width()*2; }

int OptionsSlider::height() const { return m_inner_bounds.height; }

void OptionsSlider::stylize(const StyleMap & stylemap) {
    m_left_arrow.stylize(stylemap);
    m_right_arrow.stylize(stylemap);
    TextArea::set_required_text_fields(
        m_text, stylemap.find(styles::k_global_font),
        stylemap.find(TextArea::to_key(TextArea::k_text_color)),
        stylemap.find(TextArea::to_key(TextArea::k_text_size)),
        "OptionsSlider::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("OptionsSlider::stylize", {
        make_tuple(&m_front, "regular front style",
                   stylemap.find(m_front_style, Button::to_key(Button::k_regular_front_style))),
        make_tuple(&m_back, "regular back style",
                   stylemap.find(m_back_style, Button::to_key(Button::k_regular_back_style))),
    });
}
#if 0
void OptionsSlider::set_interior_size(int w, int h){
    Helpers::verify_non_negative(w, "OptionsSlider::set_interior_size", "width" );
    Helpers::verify_non_negative(h, "OptionsSlider::set_interior_size", "height");
    m_inner_bounds.width  = w;
    m_inner_bounds.height = h;
    set_needs_geometry_update_flag();
}
#endif
#if 0
void OptionsSlider::swap_options(std::vector<UString> & options){}
#endif
void OptionsSlider::set_options(const std::vector<UString> & options) {
    m_options = options;
    set_needs_geometry_update_flag();
}

void OptionsSlider::set_options(std::vector<UString> && options) {
    m_options = std::move(options);
    set_needs_geometry_update_flag();
}

void OptionsSlider::select_option(std::size_t index) {
    using Dir = ArrowButton::Direction;
    if (index == m_selected_index) return;
    m_text.set_string( m_options.at(index) );
    m_selected_index = index;
    if (!m_wrap_enabled) {
        const auto last_idx = m_options.size() - 1;
        m_left_arrow.set_direction( index == 0 ? Dir::k_none : Dir::k_left );
        m_right_arrow.set_direction( index == last_idx ? Dir::k_none : Dir::k_right );
    }
    set_needs_geometry_update_flag();
}

std::size_t OptionsSlider::selected_option_index() const
    { return m_selected_index; }

const UString & OptionsSlider::selected_option() const
    { return m_text.string(); }

std::size_t OptionsSlider::options_count() const
    { return m_options.size(); }

void OptionsSlider::set_option_change_event(BlankFunctor && func) {
    m_press_func = std::move(func);
}

void OptionsSlider::swap(OptionsSlider & rhs) {
    using std::swap;
    swap(m_left_arrow , rhs.m_left_arrow );
    swap(m_right_arrow, rhs.m_right_arrow);

    swap(m_padding, rhs.m_padding);
    swap(m_back   , rhs.m_back   );
    swap(m_front  , rhs.m_front  );

    swap(m_back_style   , rhs.m_back_style   );
    swap(m_front_style  , rhs.m_front_style  );
    swap(m_padding_style, rhs.m_padding_style);

    swap(m_inner_bounds, rhs.m_inner_bounds);

    swap(m_text          , rhs.m_text          );
    swap(m_options       , rhs.m_options       );
    swap(m_selected_index, rhs.m_selected_index);
    swap(m_press_func    , rhs.m_press_func    );
    swap(m_wrap_enabled  , rhs.m_wrap_enabled  );
}

/* private */ void OptionsSlider::draw_(WidgetRenderer & target) const {
    draw_to(target, m_inner_bounds, m_back);
    auto front = m_inner_bounds;
    front.top += m_padding;
    front.height -= m_padding;
    draw_to(target, front, m_front);
    m_left_arrow.draw(target);
    m_right_arrow.draw(target);
}

/* private */ void OptionsSlider::issue_auto_resize() {}

/* private */ void OptionsSlider::iterate_children_(ChildWidgetIterator & itr) {
    itr.on_child(m_left_arrow);
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::iterate_children_const_
    (ChildWidgetIterator & itr) const
{
    itr.on_child(m_left_arrow);
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::on_geometry_update() {
    {
    float width_ = 0.f;
    for (const auto & str : m_options) {
        auto gv = m_text.measure_text(str);
        width_  = std::max(width_ , gv.width);
    }

    float height_ = m_text.line_height() + 2.f*float(m_padding);
    m_inner_bounds.width  = int(std::round( width_ ));
    m_inner_bounds.height = int(std::round( height_));
    }

    m_left_arrow .set_size(m_inner_bounds.height, m_inner_bounds.height);
    m_right_arrow.set_size(m_inner_bounds.height, m_inner_bounds.height);

    // center text
    m_text.set_location(
        float(m_left_arrow.location().x) + std::max(0.f, float(m_inner_bounds.width) - m_text.width()),
        float(m_left_arrow.location().y) + float(m_padding));

    // reposition right arrow
    m_right_arrow.set_location(
        m_left_arrow.location().x + m_inner_bounds.width,
        m_left_arrow.location().y);

    // update internal geometry of left and right arrows
    m_left_arrow .on_geometry_update();
    m_right_arrow.on_geometry_update();
}

/* private */ void OptionsSlider::set_location_(int x, int y) {
    m_left_arrow.set_location(x, y);
}

/* private */ void OptionsSlider::set_arrow_events() {
    using Dir = ArrowButton::Direction;
    m_left_arrow .set_direction(Dir::k_left );
    m_right_arrow.set_direction(Dir::k_right);

    m_left_arrow.set_press_event([this]() {
        if (m_options.empty() || m_selected_index == 0) return;
        select_option( --m_selected_index );
        m_press_func();

    });
    m_right_arrow.set_press_event([this]() {
        auto last_idx = m_options.size() - 1;
        if (m_options.empty() || m_selected_index == last_idx) return;
        select_option( ++m_selected_index );
        m_press_func();
    });
}

} // end of asgl namespace

#if 0
#include <ksg/OptionsSlider.hpp>
#include <ksg/Frame.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/TextArea.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <iostream>

#include <cassert>

namespace ksg {

using VectorF = OptionsSlider::VectorF;
using UString = OptionsSlider::UString;

OptionsSlider::OptionsSlider() {
    m_right_arrow.set_press_event([this]() {
        std::size_t next_index;
        if (selected_option_index() != options_count() - 1) {
            next_index = selected_option_index() + 1;
        } else if (m_wrap_enabled) {
            next_index = 0;
        } else {
            return;
        }
        select_option(next_index);
        m_press_func();
    });

    m_left_arrow.set_press_event([this]()  {
        std::size_t next_index;
        if (selected_option_index() != 0) {
            next_index = selected_option_index() - 1;
        } else if (m_wrap_enabled) {
            next_index = m_options.size() - 1;
        } else {
            return;
        }
        select_option(next_index);
        m_press_func();
    });
}

void OptionsSlider::process_event(const sf::Event & evnt) {
    m_left_arrow .process_event(evnt);
    m_right_arrow.process_event(evnt);
}

void OptionsSlider::set_location(float x, float y) {
    m_left_arrow.set_location(x, y);
    if (is_horizontal()) {
        m_back .set_position(x + m_left_arrow.width(), y);
        m_front.set_position(x + m_left_arrow.width(), y + padding());
        m_right_arrow.set_location(x + m_left_arrow.width() + m_back.width(),
                                   y);
    } else {
        assert(is_vertical());
        m_back.set_position(x, y + m_left_arrow.height());
        m_front.set_position(x + padding(), y + m_left_arrow.height());
        m_right_arrow.set_location(x,
                                   y + m_left_arrow.height() + m_back.height());
    }
    recenter_text();
}

VectorF OptionsSlider::location() const
    { return m_left_arrow.location(); }

float OptionsSlider::width() const
    { return m_size.x; }

float OptionsSlider::height() const
    { return m_size.y; }

void OptionsSlider::set_style(const StyleMap & smap) {
    using namespace styles;
    m_left_arrow.set_style(smap);
    m_right_arrow.set_style(smap);

    set_if_present(m_text, smap, k_global_font,
                   TextButton::k_text_size, TextButton::k_text_color);
#   if 0
    if (!set_if_found(smap, k_global_padding, m_padding)) {
        m_padding = 2.f;
    }
#   endif
    set_if_color_found(smap, Button::k_regular_front_color, m_front);
    set_if_color_found(smap, Button::k_regular_back_color , m_back );

    // setting style should not invoke any kind of geometry update
}

void OptionsSlider::set_interior_size(float w, float h) {
    if (w == 0.f || h == 0.f) return;
#   if 0
    if (m_padding > w || m_padding > h) {
        // have to get right of padding, constraints too tight!
        m_padding = 0.f;
    }
#   endif
    float arrow_size = h;//std::min(w, h);
    m_left_arrow .set_size(arrow_size, arrow_size);
    m_right_arrow.set_size(arrow_size, arrow_size);

    m_back .set_size(w + padding()*2.f, h);
    m_front.set_size(w + padding()*2.f, h - padding()*2.f);

    m_text.set_limiting_dimensions(m_front.width(), m_front.height());
    // do I need padding around here?
    m_size = VectorF(w + arrow_size*2.f + padding()*2.f, h);
    set_location(location().x, location().y);
}

void OptionsSlider::swap_options(std::vector<UString> & options) {
    m_options.swap(options);
    select_option(0);
}

void OptionsSlider::set_options(const std::vector<UString> & options) {
    auto t = options;
    set_options(std::move(t));
}

void OptionsSlider::set_options(std::vector<UString> && options) {
    m_options.swap(options);
    select_option(0);
}

void OptionsSlider::select_option(std::size_t index) {
    if (index >= m_options.size()) {
        throw std::out_of_range("OptionsSlider::select_option: Index is out "
                                "of range");
    }
    m_selected_index = index;
    m_text.set_string(m_options[m_selected_index]);
    set_wrap_enabled(m_wrap_enabled);
    recenter_text();
}

std::size_t OptionsSlider::selected_option_index() const
    { return m_selected_index; }

const UString & OptionsSlider::selected_option() const
    { return m_options[m_selected_index]; }

void OptionsSlider::set_option_change_event(BlankFunctor && func)
    { m_press_func = std::move(func); }

void OptionsSlider::set_wrap_enabled(bool b) {
    m_wrap_enabled = b;

    if (m_options.empty()) return;

    using Dir = ArrowButton::Direction;
    if (!m_wrap_enabled && m_selected_index == m_options.size() - 1) {
        m_right_arrow.set_direction(Dir::k_none);
    } else {
        m_right_arrow.set_direction(Dir::k_right);
    }

    if (!m_wrap_enabled && m_selected_index == 0) {
        m_left_arrow.set_direction(Dir::k_none);
    } else {
        m_left_arrow.set_direction(Dir::k_left);
    }
}

/* private */ void OptionsSlider::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    target.draw(m_back);
    target.draw(m_front);
    target.draw(m_text);
    target.draw(m_left_arrow );
    target.draw(m_right_arrow);
}

/* private */ void OptionsSlider::issue_auto_resize() {
    if (width() != 0.f || height() != 0.f || !m_text.has_font_assigned()) return;
    float width_ = 0.f;
    for (const auto & str : m_options) {
        auto gv = m_text.measure_text(str);
        width_  = std::max(width_ , gv.width );
    }

    float height_ = m_text.line_height() + 2.f*padding();
    set_interior_size(width_, height_);
}

/* private */ void OptionsSlider::iterate_children_
    (ChildWidgetIterator & itr)
{
    itr.on_child(m_left_arrow );
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::iterate_const_children_
    (ChildWidgetIterator & itr) const
{
    itr.on_child(m_left_arrow );
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::recenter_text() {
    // assume size has been decided, use front
    float width_diff  = m_front.width () - m_text.width ();
    float height_diff = m_front.height() - m_text.height();
    m_text.set_location(m_front.x() + std::max(0.f, width_diff  / 2.f),
                        m_front.y() + std::max(0.f, height_diff / 2.f));
}

/* private */ float OptionsSlider::padding() const noexcept
    { return std::max(0.f, m_left_arrow.padding()); }

} // end of ksg namespace
#endif
