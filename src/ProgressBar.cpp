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

#include <asgl/ProgressBar.hpp>

#include <common/Util.hpp>

#include <cmath>

namespace {

using namespace cul::exceptions_abbr;

void verify_padding_ok(int, const char * caller);

} // end of <anonymous> namespace

namespace asgl {

Vector ProgressBar::location() const { return top_left_of(m_bounds); }

void ProgressBar::set_size(int w, int h) {
    Helpers::verify_non_negative(w, "ProgressBar::set_size", "width" );
    Helpers::verify_non_negative(h, "ProgressBar::set_size", "height");
    set_size_of(m_bounds, w, h);
    flag_needs_whole_family_geometry_update();
}

Size ProgressBar::size() const { return size_of(m_bounds); }

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
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

void ProgressBar::set_fill_style(StyleKey key) {
    m_fill_key = key;
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

void ProgressBar::set_void_style(StyleKey key) {
    m_void_key = key;
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

void ProgressBar::set_padding(int p) {
    verify_padding_ok(p, "ProgressBar::set_padding");
    m_padding = p;
    set_padding(styles::k_null_key);
    flag_needs_whole_family_geometry_update();
}

void ProgressBar::set_padding(StyleKey key) {
    m_pad_key = key;
    m_padding = styles::k_uninit_size;
}

int ProgressBar::padding() const { return m_padding; }

void ProgressBar::set_fill_amount(float fill_amount) {
    if (!cul::is_real(fill_amount) || fill_amount < 0.f || fill_amount > 1.f) {
        throw InvArg("ProgressBar::set_fill_amount: fill amount must be in [0 1].");
    }
    m_fill_amount = fill_amount;
    // it isn't clear to me how I should go about marking for needing redraw
#   if 0
    set_needs_redraw_flag();
#   endif
}

float ProgressBar::fill_amount() const { return m_fill_amount; }

void ProgressBar::draw(WidgetRenderer & renderer) const {
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

/* private */ void ProgressBar::set_location_(int x, int y) {
    set_top_left_of(m_bounds, x, y);

    verify_padding_set("on_geometry_update");
    const int & pad = m_padding;
    if (pad*2 >= m_bounds.width) {
        m_inner_bounds = Rectangle();
        return;
    }

    m_inner_bounds = Rectangle
        (m_bounds.left  + pad  , m_bounds.top    + pad,
         m_bounds.width - pad*2, m_bounds.height - pad*2);
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
