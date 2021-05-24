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

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>

#if 0
#include <common/FixedLengthArray.hpp>
#endif
namespace asgl {

namespace detail {

/** @brief Contains common features to both draw character and the version
 *         without advance (positional stepping) information.
 *  This class represents a single drawable character and can be treated
 *  similarly to a sprite. What makes this class notable is that it is able to
 *  partially truncate the character by "cutting" it. Methods that accomplish
 *  this are "cut_on_right" and "cut_on_bottom". Further cutting methods maybe
 *  added in the future.
 *  @warning Once a character has been cut it cannot be restored short of
 *           re-initializing the character from its original glyph.
 */
class DrawableCharacter : public sf::Drawable {
public:
    using VectorF = sf::Vector2f;

    DrawableCharacter() {}

    DrawableCharacter(VectorF loc, const sf::Glyph & glyph, sf::Color clr);

    DrawableCharacter(const sf::Glyph & glyph, sf::Color clr);

    void set_color(sf::Color clr);

    sf::Color color() const;

    float width() const;

    float height() const;

    /** Cuts off part of a quad by changing its right verticies. A cut is made
     *  by changing positions and the texture rectangle's coordinates to give a
     *  "cut-off" effect. This will cause the right side of the character to
     *  be omitted from rendering.
     *  @param cut_line X-coordinate where the cut is made.
     */
    void cut_on_right(float cut_line);

    /** Cuts off part of a quad by changing its bottom verticies. A cut is made
     *  by changing positions and the texture rectangle's coordinates to give a
     *  "cut-off" effect. This will cause the bottom of the character to be
     *  omitted from rendering.
     *  @param cut_line Y-coordinate where the cut is made.
     */
    void cut_on_bottom(float cut_line);

    void cut_outside_of(const sf::FloatRect &);

    void set_location(float x, float y);

    VectorF location() const;

    void move(float x, float y);

    bool whiped_out() const;

private:
    void draw(sf::RenderTarget & target, sf::RenderStates states) const final;

    void check_invarients() const;

    std::array<sf::Vertex, 4> m_verticies;
};

} // end of detail namespace -> into ::asgl

} // end of asgl namespace
