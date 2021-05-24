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
#if 0
#include <asgl/DrawTriangle.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

namespace {

using VectorF = DrawTriangle::VectorF;

constexpr const int k_point_a = 0;
constexpr const int k_point_b = 1;
constexpr const int k_point_c = 2;

} // end of <anonymous> namesoace

void DrawTriangle::set_point_a(float x, float y)
    { m_verticies[k_point_a].position = VectorF(x, y); }

void DrawTriangle::set_point_b(float x, float y)
    { m_verticies[k_point_b].position = VectorF(x, y); }

void DrawTriangle::set_point_c(float x, float y)
    { m_verticies[k_point_c].position = VectorF(x, y); }

void DrawTriangle::move(VectorF r) {
    for (sf::Vertex & v : m_verticies)
        v.position += r;
}

void DrawTriangle::set_center(VectorF r)
    { move(r - center()); }

void DrawTriangle::set_center(float x, float y)
    { set_center(VectorF(x, y)); }

VectorF DrawTriangle::point_a() const
    { return m_verticies[k_point_a].position; }

VectorF DrawTriangle::point_b() const
    { return m_verticies[k_point_b].position; }

VectorF DrawTriangle::point_c() const
    { return m_verticies[k_point_c].position; }

VectorF DrawTriangle::center() const {
    VectorF loc;
    for (const sf::Vertex & v : m_verticies)
        loc += v.position;
    return loc * (1.f / float(k_vertex_count));
}

sf::Color DrawTriangle::color() const { return m_verticies[k_point_a].color; }

void DrawTriangle::set_color(sf::Color color_) {
    for (sf::Vertex & v : m_verticies)
        v.color = color_;
}

/* private */ void DrawTriangle::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
    { target.draw(&*m_verticies.begin(), m_verticies.size(), sf::Triangles, states); }
#endif
