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

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <common/FixedLengthArray.hpp>

#include <type_traits>

/** A drawing rectangle by default is zero-sized white "box" located at the
 *  origin. @n
 *  Units for this class are in pixels. @n
 *  I hope it's self-explanitory what the class members do.
 */
class DrawRectangle final : public sf::Drawable {
public:

    static constexpr const unsigned k_vertex_count = 4;

    DrawRectangle();

    DrawRectangle
        (float x_, float y_, float width_ = 0.f, float height_ = 0.f,
         sf::Color clr_ = sf::Color::White);

    ~DrawRectangle() override;

    void set_x(float) noexcept;

    void set_y(float) noexcept;

    void set_width(float) noexcept;

    void set_height(float) noexcept;

    void set_position(float x_, float y_) noexcept;

    void set_position(const sf::Vector2f & r) noexcept;

    void move(const sf::Vector2f & r) noexcept;

    void move(float x_, float y_) noexcept;

    void set_size(float width, float height) noexcept;

    void set_color(sf::Color) noexcept;

    float width() const noexcept;

    float height() const noexcept;

    float x() const noexcept;

    float y() const noexcept;

    sf::Vector2f position() const noexcept;

    sf::Color color() const noexcept;

protected:

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

private:

    static constexpr const unsigned k_top_left     = 0;
    static constexpr const unsigned k_top_right    = 1;
    static constexpr const unsigned k_bottom_right = 2;
    static constexpr const unsigned k_bottom_left  = 3;

    // use that dank af copy semantics
    FixedLengthArray<sf::Vertex, k_vertex_count> m_vertices;
};
