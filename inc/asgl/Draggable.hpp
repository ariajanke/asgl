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

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <common/Grid.hpp>

namespace asgl {

/** @brief Any on-screen object that can be dragged by a mouse.
 *
 *  Draggable provides the "draggable" feature for inheriting classes. In order
 *  for this to work the child class must have a rectangle object to define the
 *  bounds which can be dragged. Functions (as well as keeping track of a
 *  current state) are provided to start a drag (mouse_click), update a drag's
 *  position (mouse_click, update_drag_position) and end the drag
 *  (drag_release).
 */
class Draggable {
public:
    /** Ignores all potential drag events regardless of where a mouse click
     *  event occurs.
     */
    void ignore_drag_events() { m_watch_drag_events = false; }

    /** Causes the draggable object to watch for drag events. This function
     *  negates the effect of the ignore_drag_events method.
     */
    void watch_for_drag_events() { m_watch_drag_events = true; }

    /** @returns true if the draggable is watching for drag events */
    bool is_watching_for_drag_events() const { return m_watch_drag_events; }

protected:
    Draggable() {}

    virtual ~Draggable();

    /** @brief mouse_move updates the Draggable with the current mouse
     *  position.
     *
     *  If the Draggable is in the drag state, "update_drag_position" will be
     *  called, otherwise this funtion does nothing.
     *
     *  @param x X-coordinate of the current mouse position.
     *  @param y Y-coordinate of the current mouse position.
     */
    void mouse_move(int x, int y);

    /** @brief mouse_click Handles mouse click events.
     *
     *  Begins the drag event regardless of where the mouse is (unless not
     *  watching for drag events).
     */
    bool mouse_click(int x, int y);

    /** @brief mouse_click Handles mouse click events.
     *
     *  If a mouse click were to take place within the given rectangle, this
     *  object enters a drag state. Any additional mouse movements will cause
     *  this object to follow the cursor until drag_release is called.
     *
     *  @param x X-coordinate of the current mouse position.
     *  @param y Y-coordinate of the current mouse position.
     *  @param drect The given rectangle.
     *  @return Returns true if the object has entered the drag state.
     */
    bool mouse_click(int x, int y, const sf::IntRect & drect);

    /** @brief mouse_click Handles mouse click events.
     *
     *  If a mouse click were to take place within the given click matrix, this
     *  object enters a drag state. Any additional mouse movements will cause
     *  this object to follow the cursor until drag_release is called.
     *
     *  @param x X-coordinate of the current mouse position.
     *  @param y Y-coordinate of the current mouse position.
     *  @param matrix_location an "on screen" location of the matrix
     *  @param click_matrix a mask precise of which pixels are and are not
     *                      clickable (true meaning clickable, false otherwise)
     *  @return Returns true if the object has entered the drag state.
     */
    bool mouse_click(int x, int y, sf::Vector2i matrix_location,
                     const Grid<bool> & click_matrix);

    /** @brief drag_release Causes object to leave its drag state.
     */
    void drag_release()
        { m_dragged = false; }

    /** Constrains the draggable's positions, as it changes from mouse_move.
     *  @param area rectangle which this object's position may not go outside
     *              of
     *  @note Set constraints will not take size into account, so this will
     *        have to be subtracted from the area parameter.
     */
    void set_drag_contraints(const sf::IntRect & area);

    /** Allows the draggable to be dragged to any position. */
    void remove_drag_contraints();

    /** When this function is called, it is provided the new coordinates where
     *  the rectangle object that was used when calling mouse_click(), should
     *  be placed now.
     *  @see mouse_move()
     *  @param x New "should-be" X-coordinate of the rectangle object.
     *  @param y New "should-be" Y-coordinate of the rectangle object.
     */
    virtual void update_drag_position(int x, int y) = 0;

private:
    bool has_position_contraints() const;

    bool m_watch_drag_events = true;
    bool m_dragged = false;
    sf::Vector2i m_drag_offset;
    sf::IntRect m_position_contraints;
};

} // end of asgl namespace
