/****************************************************************************

    File: DrawTriangle.hpp
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
#include <SFML/Graphics/Vertex.hpp>

#include <common/FixedLengthArray.hpp>

class DrawTriangle final : public sf::Drawable {
public:
    static constexpr const auto k_vertex_count = 3u;

    using VectorF = sf::Vector2f;

    void set_point_a(float x, float y);

    void set_point_a(VectorF pos)
        { set_point_a(pos.x, pos.y); }

    void set_point_b(float x, float y);

    void set_point_b(VectorF pos)
        { set_point_b(pos.x, pos.y); }

    void set_point_c(float x, float y);

    void set_point_c(VectorF pos)
        { set_point_c(pos.x, pos.y); }

    void move(VectorF);
#   if 0
    [[deprecated]] void set_location(VectorF);

    [[deprecated]] void set_location(float, float);
#   endif
    void set_center(VectorF);

    void set_center(float, float);

    VectorF point_a() const;

    VectorF point_b() const;

    VectorF point_c() const;
#   if 0
    [[deprecated]] VectorF location() const;
#   endif
    VectorF center() const;

    sf::Color color() const;

    void set_color(sf::Color);

private:
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    FixedLengthArray<sf::Vertex, k_vertex_count> m_verticies;
};
