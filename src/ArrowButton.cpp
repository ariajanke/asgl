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

#include <asgl/ArrowButton.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace asgl {

ArrowButton::ArrowButton(): m_dir(Direction::k_none) {}

void ArrowButton::set_direction(Direction dir_) {
    if (m_dir == dir_) return;
    m_dir = dir_;
    set_needs_geometry_update_flag();
}

void ArrowButton::process_event(const Event & evnt) {
    if (m_dir == Direction::k_none) return;
    Button::process_event(evnt);
}

void ArrowButton::stylize(const StyleMap & smap) {
    Button::stylize(smap);
    using std::make_tuple;
    Helpers::handle_required_fields("ArrowButton::stylize", {
        make_tuple(&m_triangle_style, "triangle style",
                   smap.find(DefaultStyles::to_key(k_triangle_style)))
    });
}

void ArrowButton::set_arrow_style(ItemKey ky)
    { m_triangle_style = ky; }

void ArrowButton::on_geometry_update() {
    Button::on_geometry_update();
    update_points();
}

/* private */ void ArrowButton::draw_(WidgetRenderer & target) const {
    Button::draw_(target);
    draw_to(target, m_tri, m_triangle_style);
}

/* private */ void ArrowButton::update_points() {
    using std::get;
    if (width() < padding()*2 || height() < padding()*2) {
        get<0>(m_tri) = get<1>(m_tri) = get<2>(m_tri) = VectorI();
        return;
    }

    VectorI anchor = location() + VectorI(width() / 2, height() / 2);
    int offset = std::min(width () / 2 - padding()*2,
                          height() / 2 - padding()*2);
    switch (m_dir) {
    case Direction::k_down:
        get<0>(m_tri) = anchor + VectorI(      0,  offset);
        get<1>(m_tri) = anchor + VectorI(-offset, -offset);
        get<2>(m_tri) = anchor + VectorI( offset, -offset);
        break;
    case Direction::k_left:
        get<0>(m_tri) = anchor + VectorI(-offset,       0);
        get<1>(m_tri) = anchor + VectorI( offset, -offset);
        get<2>(m_tri) = anchor + VectorI( offset,  offset);
        break;
    case Direction::k_right:
        get<0>(m_tri) = anchor + VectorI( offset,       0);
        get<1>(m_tri) = anchor + VectorI(-offset, -offset);
        get<2>(m_tri) = anchor + VectorI(-offset,  offset);
        break;
    case Direction::k_up:
        get<0>(m_tri) = anchor + VectorI(      0, -offset);
        get<1>(m_tri) = anchor + VectorI(-offset,  offset);
        get<2>(m_tri) = anchor + VectorI( offset,  offset);
        break;
    case Direction::k_none:
        deselect();
        break;
    }
}

} // end of ksg namespace
