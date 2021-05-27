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

#pragma once

#include <common/Vector2Util.hpp>

namespace asgl {

using Vector    = cul::Vector2<int>;
using Size      = cul::Size2<int>;
using Triangle  = std::tuple<Vector, Vector, Vector>;
using Rectangle = cul::Rectangle<int>;

using cul::top_left_of;
using cul::set_top_left_of;
using cul::size_of;
using cul::set_size_of;
using cul::is_contained_in;

template <typename T>
bool is_contained_in(T x_, T y_, const cul::Rectangle<T> & rect)
    { return is_contained_in(cul::Vector2<T>(x_, y_), rect); }

#if 0
inline bool is_contained(const Rectangle & rect, const Vector & r) {
    using namespace cul;
    return    r.x >= rect.left      && r.y >= rect.top
           && r.x <  right_of(rect) && r.y <  bottom_of(rect);
}

inline bool is_contained(const Rectangle & rect, int x, int y) {
    return is_contained(rect, Vector(x, y));
}
#endif
} // end of asgl namespace
