/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#include <common/DrawRectangle.hpp>

#include <SFML/Graphics.hpp>

DrawRectangle::DrawRectangle() {}

DrawRectangle::DrawRectangle
    (float x_, float y_, float w_, float h_, sf::Color clr_)
{
    set_position(x_, y_);
    set_size(w_, h_);
    set_color(clr_);
}

/* override */ DrawRectangle::~DrawRectangle() {}

void DrawRectangle::set_x(float x_) noexcept { set_position(x_, y()); }

void DrawRectangle::set_y(float y_) noexcept { set_position(x(), y_); }

void DrawRectangle::set_position(float x_, float y_) noexcept {
    // save old width and height
    float w = width(), h = height();

    // position
    for (auto & vtx : m_vertices) vtx.position = sf::Vector2f(x_, y_);

    // impl detail, must reset size, since we erased it with overwriting each
    // of the Quad's points
    set_size(w, h);
}

void DrawRectangle::set_position(const sf::Vector2f & r) noexcept
    { set_position(r.x, r.y); }

void DrawRectangle::move(const sf::Vector2f & r) noexcept
    { move(r.x, r.y); }

void DrawRectangle::move(float x_, float y_) noexcept
    { set_position(x_ + x(), y_ + y()); }

void DrawRectangle::set_size(float w, float h) noexcept {
    // impl detail, position accessors x() and y() only access the first
    // vertex, which this function does not change
    set_width (w);
    set_height(h);
}

void DrawRectangle::set_color(sf::Color clr) noexcept
    { for (auto & vtx : m_vertices) vtx.color = clr; }

void DrawRectangle::set_width(float w) noexcept {
    // clear width, add new width
    m_vertices[k_top_right   ].position.x = x() + w;
    m_vertices[k_bottom_right].position.x = x() + w;
}

void DrawRectangle::set_height(float h) noexcept {
    // clear height, and new height
    m_vertices[k_bottom_right].position.y = y() + h;
    m_vertices[k_bottom_left ].position.y = y() + h;
}

float DrawRectangle::width() const noexcept
    { return m_vertices[k_top_right].position.x - x(); }

float DrawRectangle::height() const noexcept
    { return m_vertices[k_bottom_right].position.y - y(); }

float DrawRectangle::x() const noexcept
    { return m_vertices[k_top_left].position.x; }

float DrawRectangle::y() const noexcept
    { return m_vertices[k_top_left].position.y; }

sf::Vector2f DrawRectangle::position() const noexcept
    { return sf::Vector2f(x(), y()); }

sf::Color DrawRectangle::color() const noexcept
    { return m_vertices[k_top_left].color; }

/* virtual protected */ void DrawRectangle::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
    { target.draw(&*m_vertices.begin(), k_vertex_count, sf::Quads, states); }
