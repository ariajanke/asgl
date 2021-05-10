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

    void set_center(VectorF);

    void set_center(float, float);

    VectorF point_a() const;

    VectorF point_b() const;

    VectorF point_c() const;

    VectorF center() const;

    sf::Color color() const;

    void set_color(sf::Color);

private:
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    FixedLengthArray<sf::Vertex, k_vertex_count> m_verticies;
};
