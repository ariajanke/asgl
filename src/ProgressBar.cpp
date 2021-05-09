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
