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
#include <asgl/Frame.hpp>

#include <cmath>
#include <cassert>

namespace {

using VectorI = asgl::Widget::VectorI;
using UString = asgl::OptionsSlider::UString;

} // end of <anonymous> namespace

namespace asgl {

OptionsSlider::OptionsSlider() {
    using Dir = ArrowButton::Direction;
    m_left_arrow .set_direction(Dir::k_left );
    m_right_arrow.set_direction(Dir::k_right);
    set_arrow_events();
}

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
    { return m_inner_bounds.width + m_inner_bounds.height*2; }

int OptionsSlider::height() const { return m_inner_bounds.height; }

void OptionsSlider::stylize(const StyleMap & stylemap) {
    m_left_arrow.stylize(stylemap);
    m_right_arrow.stylize(stylemap);
    TextArea::set_required_text_fields(
        m_text, stylemap.find(styles::k_global_font),
        stylemap.find(Frame::to_key(Frame::k_widget_text_style)),
        "OptionsSlider::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("OptionsSlider::stylize", {
        make_tuple(&m_front, "regular front style",
                   stylemap.find(m_front_style, Button::to_key(Button::k_regular_front_style))),
        make_tuple(&m_back, "regular back style",
                   stylemap.find(m_back_style, Button::to_key(Button::k_regular_back_style))),
    });
}

void OptionsSlider::set_options(const std::vector<UString> & options) {
    m_options = options;
    set_needs_geometry_update_flag();
}

void OptionsSlider::set_options(std::vector<UString> && options) {
    m_options = std::move(options);
    set_needs_geometry_update_flag();
}

void OptionsSlider::select_option(std::size_t index) {
    if (index == m_selected_index) return;    
    m_selected_index = index;
    update_selections();
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
#   if 0
    target.render_text(m_text);
#   endif
    m_text.draw_to(target);
}

/* private */ void OptionsSlider::issue_auto_resize() {
    int width_ = 0, height_ = 0;
    for (const auto & str : m_options) {
#       if 0
        auto gv = m_text.measure_text(str);
#       endif
        auto gv = m_text.measure_text(str.begin(), str.end());
        width_  = std::max(width_ , gv.width );
        height_ = std::max(height_, gv.height);
    }

    //float height_ = m_text./*line_*/height() + 2.f*float(std::max(m_padding, 0));
    m_inner_bounds.width  = int(std::round( width_ ));
    m_inner_bounds.height = int(std::round( height_));

    assert(m_inner_bounds.width  >= 0);
    assert(m_inner_bounds.height >= 0);
}

/* private */ void OptionsSlider::iterate_children_(ChildWidgetIterator & itr) {
    itr.on_child(m_left_arrow );
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::iterate_children_const_
    (ChildWidgetIterator & itr) const
{
    itr.on_child(m_left_arrow );
    itr.on_child(m_right_arrow);
}

/* private */ void OptionsSlider::on_geometry_update() {
    update_selections();
    m_left_arrow .set_size(m_inner_bounds.height, m_inner_bounds.height);
    m_right_arrow.set_size(m_inner_bounds.height, m_inner_bounds.height);

    // center text
    float center_offset = std::max(0.f, float(m_inner_bounds.width) - m_text.width());
    m_text.set_location(
        float(m_left_arrow.location().x + m_left_arrow.width()) + center_offset,
        float(m_left_arrow.location().y) + float(m_padding));

    m_inner_bounds.left = m_left_arrow.location().x + m_left_arrow.width();
    m_inner_bounds.top  = m_left_arrow.location().y;

    // reposition right arrow
    m_right_arrow.set_location(
        m_left_arrow.location().x + m_left_arrow.width() + m_inner_bounds.width,
        m_left_arrow.location().y);

    // update internal geometry of left and right arrows
    m_left_arrow .on_geometry_update();
    m_right_arrow.on_geometry_update();
}

/* private */ void OptionsSlider::set_location_(int x, int y) {
    m_left_arrow.set_location(x, y);
}

/* private */ void OptionsSlider::set_arrow_events() {
    m_left_arrow.set_press_event([this]() {
        if (m_options.empty() || m_selected_index == 0) return;
        --m_selected_index;
        update_selections();
        m_press_func();

    });
    m_right_arrow.set_press_event([this]() {
        auto last_idx = m_options.size() - 1;
        if (m_options.empty() || m_selected_index == last_idx) return;
        ++m_selected_index;
        update_selections();
        m_press_func();
    });
}

/* private */ void OptionsSlider::update_selections() {
    using Dir = ArrowButton::Direction;
    auto idx = m_selected_index;
    m_text.set_string( m_options.at(idx) );
    if (!m_wrap_enabled) {
        const auto last_idx = m_options.size() - 1;
        m_left_arrow .set_direction(idx == 0        ? Dir::k_none : Dir::k_left );
        m_right_arrow.set_direction(idx == last_idx ? Dir::k_none : Dir::k_right);
    }
}

} // end of asgl namespace
