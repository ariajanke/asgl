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

#include <asgl/Draggable.hpp>

#include <stdexcept>

#include <cmath>

namespace {

using InvalidArg = std::invalid_argument;

} // end of <anonymous> namespace

namespace asgl {

Draggable::~Draggable() {}

void Draggable::mouse_move(int x, int y) {
    if (!m_dragged) return;
    x -= m_drag_offset.x;
    y -= m_drag_offset.y;
    if (has_position_contraints()) {
        const auto & contr = m_position_contraints;
        x = std::min(x, contr.left + contr.width);
        x = std::max(x, contr.left);
        y = std::min(y, contr.top + contr.height);
        y = std::max(y, contr.top);
    }
    update_drag_position(x, y);
}

bool Draggable::mouse_click
    (int x, int y, const sf::IntRect &drect)
{
    if (!m_watch_drag_events) return false;
    if (drect.contains(x, y)) {
        m_dragged = true;
        m_drag_offset = sf::Vector2i
            (int(std::round(x - drect.left)), int(std::round(y - drect.top)));
        return true;
    }
    return false;
}

void Draggable::set_drag_contraints(const sf::IntRect & area) {
    if (area.width == 0 || area.height == 0) {
        throw InvalidArg("Draggable::set_position_contraints: Position "
                         "constraint area may not have a zero sized area.\n"
                         "Consider disabling this drag feature instead (if "
                         "possible).");
    }
    m_position_contraints = area;
}

void Draggable::remove_drag_contraints() {
    m_position_contraints = sf::IntRect();
}

/* private */ bool Draggable::has_position_contraints() const {
    return    m_position_contraints.width  != 0
           && m_position_contraints.height != 0;
}

} // end of asgl namespace
