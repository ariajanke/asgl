/****************************************************************************

    File: ProgressBar.cpp
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

#include <asgl/ProgressBar.hpp>

#include <common/Util.hpp>

#include <cmath>

namespace {

using VectorI    = asgl::ProgressBar::VectorI;
using InvArg     = std::invalid_argument;
using RtError    = std::runtime_error;
using ItemKey    = asgl::ItemKey;
using StyleField = asgl::StyleField;

void verify_padding_ok(int, const char * caller);

} // end of <anonymous> namespace

namespace asgl {

VectorI ProgressBar::location() const
    { return VectorI (m_bounds.left, m_bounds.top); }

void ProgressBar::set_size(int w, int h) {
    m_bounds.width  = w;
    m_bounds.height = h;
    set_needs_geometry_update_flag();
}

int ProgressBar::width() const { return m_bounds.width; }

int ProgressBar::height() const { return m_bounds.height; }

void ProgressBar::stylize(const StyleMap & map) {
    m_padding = Helpers::verify_padding
        (map.find(m_pad_key, to_key(k_padding_style), styles::k_global_padding),
         "ProgressBar::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("ProgressBar::stylize", {
        make_tuple(&m_outer_style, "k_outer_style",
                   map.find(m_outer_key, to_key(k_outer_style))),
        make_tuple(&m_fill_style, "k_fill_style",
                   map.find(m_fill_key, to_key(k_fill_style))),
        make_tuple(&m_void_style, "k_void_style",
                   map.find(m_void_key, to_key(k_void_style))),
    });
    verify_padding_set("stylize");
}

void ProgressBar::set_outer_style(StyleKey key) {
    m_outer_key = key;
    set_needs_redraw_flag();
}

void ProgressBar::set_fill_style(StyleKey key) {
    m_fill_key = key;
    set_needs_redraw_flag();
}

void ProgressBar::set_void_style(StyleKey key) {
    m_void_key = key;
    set_needs_redraw_flag();
}

void ProgressBar::set_padding(int p) {
    verify_padding_ok(p, "ProgressBar::set_padding");
    m_padding = p;
    set_padding(styles::k_null_key);
    set_needs_geometry_update_flag();
}

void ProgressBar::set_padding(StyleKey key) {
    m_pad_key = key;
    m_padding = styles::k_uninit_size;
}

int ProgressBar::padding() const { return m_padding; }

void ProgressBar::set_fill_amount(float fill_amount) {
    if (!is_real(fill_amount) || fill_amount < 0.f || fill_amount > 1.f) {
        throw InvArg("ProgressBar::set_fill_amount: fill amount must be in [0 1].");
    }
    m_fill_amount = fill_amount;
    set_needs_redraw_flag();
}

float ProgressBar::fill_amount() const { return m_fill_amount; }

/* private */ void ProgressBar::set_location_(int x, int y) {
    m_bounds.left = x;
    m_bounds.top  = y;
    set_needs_geometry_update_flag();
}

/* private */ void ProgressBar::on_geometry_update() {
    verify_padding_set("on_geometry_update");
    if (m_padding*2 >= m_bounds.width) {
        m_inner_bounds = sf::IntRect();
        return;
    }

    m_inner_bounds.left = m_bounds.left + m_padding;
    m_inner_bounds.top  = m_bounds.top  + m_padding;

    m_inner_bounds.width  = m_bounds.left - m_padding*2;
    m_inner_bounds.height = m_bounds.top  - m_padding*2;
}

/* private */ void ProgressBar::draw_(WidgetRenderer & renderer) const {
    verify_padding_set("draw");
    renderer.render_rectangle(m_bounds, m_outer_style, this);
    if (m_inner_bounds.width == 0) return;

    int fill_len = int(std::round( float(m_inner_bounds.width)*m_fill_amount ));
    int void_len = m_inner_bounds.width - fill_len;

    auto inner_copy = m_inner_bounds;
    inner_copy.width = fill_len;
    renderer.render_rectangle(inner_copy, m_fill_style, this);

    inner_copy.left += fill_len;
    inner_copy.width = void_len;
    renderer.render_rectangle(inner_copy, m_void_style, this);
}

/* private */ void ProgressBar::verify_padding_set(const char * caller) const {
    if (m_padding != styles::k_uninit_size) return;
    throw RtError("ProgressBar::" + std::string(caller) + ": padding has not"
                  " been initialized. It may either be done so with a call to "
                  "\"stylize\" or \"set_padding\" with an integer.");
}

} // end of asgl namespace

namespace {

void verify_padding_ok(int i, const char * caller) {
    if (i >= 0) return;
    throw InvArg("ProgressBar::" + std::string(caller)
                 + " padding must be a non-negative integer.");
}

} // end of <anonymous> namespace


#if 0
#include <ksg/ProgressBar.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

namespace ksg {

namespace {

static const char * const k_fill_out_of_range_msg =
    "ProgressBar::set_fill_amount: fill amount is not in range: [0 1].";

} // end of <anonymous> namespace

using VectorF = ksg::Widget::VectorF;

/* static */ constexpr const char * const ProgressBar::k_outer_color      ;
/* static */ constexpr const char * const ProgressBar::k_inner_front_color;
/* static */ constexpr const char * const ProgressBar::k_inner_back_color ;
/* static */ constexpr const char * const ProgressBar::k_padding          ;

void ProgressBar::process_event(const sf::Event &) {}

void ProgressBar::set_location(float x, float y) {
    m_outer.set_position(x, y);
    update_positions_using_outer();
}

VectorF ProgressBar::location() const
    { return VectorF(m_outer.x(), m_outer.y()); }

void ProgressBar::set_size(float w, float h) {
    m_outer.set_size(w, h);
    update_sizes_using_outer();
}

float ProgressBar::width() const
    { return m_outer.width(); }

float ProgressBar::height() const
    { return m_outer.height(); }

void ProgressBar::set_style(const StyleMap & smap) {
#   if 0
    if (auto * pad = styles::find<float>(smap, k_padding)) {
        m_padding = *pad;
    }
#   endif
    styles::set_if_found(smap, k_padding, m_padding);

    // it is 2020, it is time to use c++17
    for (auto [key, drect] : {
         std::make_pair(k_outer_color      , &m_outer      ),
         std::make_pair(k_inner_front_color, &m_inner_front),
         std::make_pair(k_inner_back_color , &m_inner_back )
     }) {
        styles::set_if_color_found(smap, key, *drect);
#       if 0
        if (auto * color = styles::find<sf::Color>(smap, key)) {
            drect->set_color(*color);
        }
#       endif
    }

    update_positions_using_outer();
    update_sizes_using_outer();
}

void ProgressBar::set_outer_color(sf::Color color_)
    { m_outer.set_color(color_); }

void ProgressBar::set_inner_front_color(sf::Color color_)
    { m_inner_front.set_color(color_); }

void ProgressBar::set_inner_back_color(sf::Color color_)
    { m_inner_back.set_color(color_); }

void ProgressBar::set_fill_amount(float fill_amount) {
    if (fill_amount < 0.f or fill_amount > 1.f)
        throw std::out_of_range(k_fill_out_of_range_msg);
    m_fill_amount = fill_amount;
    set_size(width(), height());
}

float ProgressBar::fill_amount() const
    { return m_fill_amount; }

void ProgressBar::set_padding(float p) {
    m_padding = p;
    update_sizes_using_outer();
}

/* protected */ void ProgressBar::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    target.draw(m_outer      );
    target.draw(m_inner_back );
    target.draw(m_inner_front);
}

/* private */ float ProgressBar::active_padding() const {
    float padding;
    if (width() < m_padding || height() < m_padding)
        padding = 0.f;
    else
        padding = std::max(0.f, m_padding);
    return padding;
}

/* private */ void ProgressBar::update_positions_using_outer() {
    auto x = m_outer.x();
    auto y = m_outer.y();
    auto padding = active_padding();
    m_inner_back .set_position(x + padding, y + padding);
    m_inner_front.set_position(x + padding, y + padding);
}

/* private */ void ProgressBar::update_sizes_using_outer() {
    auto width  = m_outer.width ();
    auto height = m_outer.height();
    auto pad = active_padding();
    m_inner_back .set_size(width - pad*2.f, height - pad*2.f);
    m_inner_front.set_size((width - pad*2.f)*m_fill_amount,
                           height - pad*2.f                );
}

} // end of namespace ksg
#endif
