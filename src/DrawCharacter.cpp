/****************************************************************************

    File: DrawCharacter.cpp
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

#include <asgl/DrawCharacter.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <common/Util.hpp>

#include <cassert>

namespace asgl {

namespace {

using VectorF = sf::Vector2f;

constexpr const int k_top_left_index     = 0;
constexpr const int k_top_right_index    = 1;
constexpr const int k_bottom_right_index = 2;
constexpr const int k_bottom_left_index  = 3;

} // end of <anonymous> namespace

namespace detail {

DrawableCharacter::DrawableCharacter(VectorF loc, const sf::Glyph & glyph, sf::Color clr):
    DrawableCharacter(glyph, clr)
{
    set_location(loc.x, loc.y);
}

DrawableCharacter::DrawableCharacter(const sf::Glyph & glyph, sf::Color clr) {
    using Vector2f = sf::Vector2f;

    float left   = glyph.bounds.left;
    float top    = glyph.bounds.top;
    float right  = glyph.bounds.left + glyph.bounds.width;
    float bottom = glyph.bounds.top  + glyph.bounds.height;

    // abbr: texture, left/top/right/bottom
    float trL = float(glyph.textureRect.left);
    float trT = float(glyph.textureRect.top);
    float trR = float(glyph.textureRect.left + glyph.textureRect.width);
    float trB = float(glyph.textureRect.top  + glyph.textureRect.height);

    // Add a quad for the current character
    m_verticies[k_top_left_index    ] = sf::Vertex(Vector2f(left , top   ), clr, Vector2f(trL, trT));
    m_verticies[k_top_right_index   ] = sf::Vertex(Vector2f(right, top   ), clr, Vector2f(trR, trT));
    m_verticies[k_bottom_right_index] = sf::Vertex(Vector2f(right, bottom), clr, Vector2f(trR, trB));
    m_verticies[k_bottom_left_index ] = sf::Vertex(Vector2f(left , bottom), clr, Vector2f(trL, trB));

    check_invarients();
}

void DrawableCharacter::set_color(sf::Color clr) {
    for (sf::Vertex & v : m_verticies)
        v.color = clr;
}

sf::Color DrawableCharacter::color() const
    { return m_verticies.front().color; }

float DrawableCharacter::width() const {
    return m_verticies[k_top_right_index].position.x -
           m_verticies[k_top_left_index ].position.x;
}

float DrawableCharacter::height() const {
    return m_verticies[k_bottom_left_index].position.y -
           m_verticies[k_top_right_index  ].position.y;
}

void DrawableCharacter::cut_on_right(float cut_line) {
    sf::Vertex & tr = m_verticies[k_top_right_index];
    sf::Vertex & br = m_verticies[k_bottom_right_index];
    const sf::Vertex & any_left = m_verticies[k_top_left_index];

    // do not do a cut, if the line is far away from the character
    if (cut_line > tr.position.x) return;

    // if the cut line exceeds the draw character's bounds, reduce its width
    // to zero
    if (cut_line < any_left.position.x) {
        tr.position.x = br.position.x = any_left.position.x;
        check_invarients();
        return;
    }

    float cut_ratio = (cut_line - any_left.position.x)/
                      (br.position.x - any_left.position.x);
    // need to find proportion of the char's texture to cut off
    float tx_width = tr.texCoords.x - any_left.texCoords.x;
    tr.position.x = br.position.x = cut_line;
    tr.texCoords.x = br.texCoords.x = any_left.texCoords.x + tx_width*cut_ratio;
    check_invarients();
}

void DrawableCharacter::cut_on_bottom(float cut_line) {
    sf::Vertex & bl = m_verticies[k_bottom_left_index];
    sf::Vertex & br = m_verticies[k_bottom_right_index];
    const sf::Vertex & any_top = m_verticies[k_top_left_index];

    // do not do a cut, if the line is far below the character
    if (cut_line > bl.position.y) return;

    // if the cut line exceeds the draw character's bounds, reduce its height
    // to zero
    if (cut_line < any_top.position.y) {
        bl.position.y = br.position.y = any_top.position.y;
        check_invarients();
        return;
    }

    float cut_ratio = (cut_line - any_top.position.y)/
                      (br.position.y - any_top.position.y);
    float tx_height = bl.texCoords.y - any_top.texCoords.y;
    br.position.y = bl.position.y = cut_line;
    br.texCoords.y = bl.texCoords.y = any_top.texCoords.y + tx_height*cut_ratio;
    check_invarients();
}

void DrawableCharacter::set_location(float x, float y) {
    float w = width(), h = height();
    for (sf::Vertex & v : m_verticies)
        v.position = sf::Vector2f(x, y);
    m_verticies[k_top_right_index   ].position.x += w;
    m_verticies[k_bottom_left_index ].position.y += h;
    m_verticies[k_bottom_right_index].position.y += h;
    m_verticies[k_bottom_right_index].position.x += w;
    check_invarients();
}

void DrawableCharacter::move(float x, float y) {
    for (sf::Vertex & v : m_verticies)
        v.position += sf::Vector2f(x, y);
    check_invarients();
}

VectorF DrawableCharacter::location() const {
    return m_verticies[k_top_left_index].position;
}

bool DrawableCharacter::whiped_out() const {
    return magnitude(width()) < 1.f || magnitude(height()) < 1.f;
}

/* private final */ void DrawableCharacter::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    const sf::Vertex & tl = m_verticies[k_top_left_index];
    const sf::Vertex & br = m_verticies[k_bottom_right_index];
    if (   magnitude(tl.position.x - br.position.x) < 0.5f
        || magnitude(tl.position.y - br.position.y) < 0.5f)
        return;
    target.draw(&m_verticies[0], m_verticies.size(), sf::Quads, states);
}

/* private */ void DrawableCharacter::check_invarients() const {
    for (const auto & vtx : m_verticies) {
        assert(is_real(vtx.position.x) && is_real(vtx.position.y));
    }
}

} // end of detail namespace

} // end of ksg namespace
