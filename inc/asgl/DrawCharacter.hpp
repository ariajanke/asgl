/****************************************************************************

    File: DrawCharacter.hpp
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

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <common/FixedLengthArray.hpp>

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

    void set_location(float x, float y);

    VectorF location() const;

    void move(float x, float y);

    bool whiped_out() const;

private:
    void draw(sf::RenderTarget & target, sf::RenderStates states) const final;

    void check_invarients() const;

    FixedLengthArray<sf::Vertex, 4> m_verticies;
};

} // end of detail namespace

} // end of ksg namespace
