/****************************************************************************

    File: Text.cpp
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

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#include <asgl/Text.hpp>
#include <common/Util.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <array>
#include <memory>
#include <algorithm>

#include <cassert>

using UChar                  = asgl::Text::UString::value_type;
using LineBreakList          = std::vector<int>;
using VertexContainer        = std::vector<sf::Vertex>;
using UString                = asgl::Text::UString;
using FontMtPtr              = asgl::detail::FontMtPtr;
using TextSize               = asgl::Text::TextSize;
using VectorF                = asgl::Text::VectorF;
using InvalidArg             = std::invalid_argument;
using DrawableCharacter      = asgl::detail::DrawableCharacter;

namespace {

bool is_whitespace(UChar c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool is_newline   (UChar c) { return c == '\n'; }

// each iterator is a chunk begining
// we can and SHOULD test this! :)
std::vector<UString::const_iterator> find_chunks_dividers(const UString &);

void place_renderables(const sf::Font & font, const UString & ustr,
       float width_constraint, int char_size, sf::Color color,
       std::vector<DrawableCharacter> & renderables);

void cut_renderables(float width_constraint, float height_constraint,
                     std::vector<DrawableCharacter> & renderables);

} // end of <anonymous> namespace

namespace asgl {

/* static */ constexpr const int   Text::k_max_string_length;
/* static */ constexpr const float Text::k_inf;

void Text::set_string(const UString & str) {
    UString temp(str);
    set_string(std::move(temp));
}

void Text::set_string(UString && str) {
    m_string.swap(str);
    update_geometry();
}

UString Text::take_cleared_string() {
    m_string.clear();
    update_geometry();
    return std::move(m_string);
}

void Text::set_limiting_width(float w) {
    set_limiting_dimensions(w, m_height_constraint);
}

void Text::set_limiting_height(float h) {
    set_limiting_dimensions(m_width_constraint, h);
}

void Text::enable_bottom_cutting() { m_allow_bottom_cuts = true; }

void Text::disable_bottom_cutting() { m_allow_bottom_cuts = false; }

void Text::set_limiting_dimensions(float w, float h) {
    const static auto make_bad_dim_msg = [](const char * dimname) {
        return std::string("ksg::Text::set_limiting_dimensions: ") +
               dimname + " must be a positive real number, or infinity.";
    };
    if (w <= 0.f) { throw InvalidArg(make_bad_dim_msg("width" )); }
    if (h <= 0.f) { throw InvalidArg(make_bad_dim_msg("height")); }
    m_width_constraint  = w;
    m_height_constraint = h;
    update_geometry();
}

void Text::relieve_width_limit() {
    set_limiting_dimensions(k_inf, m_height_constraint);
}

void Text::relieve_height_limit() {
    set_limiting_dimensions(m_width_constraint, k_inf);
}

void Text::relieve_size_limit() {
    set_limiting_dimensions(k_inf, k_inf);
}

void Text::set_character_size(int char_size) {
    m_char_size = char_size;
    update_geometry();
}

void Text::set_color(sf::Color color) {
    m_color = color;
}

void Text::set_location(float x, float y) {
    m_bounds.left = x;
    m_bounds.top = y;
    update_geometry();
}

void Text::set_location(VectorF r) {
    set_location(r.x, r.y);
}

void Text::assign_font(const sf::Font * ptr) {
    m_font_ptr = FontMtPtr(ptr);
    update_geometry();
}

void Text::assign_font(const std::shared_ptr<const sf::Font> & ptr) {
    m_font_ptr = FontMtPtr(ptr);
    update_geometry();
}

void Text::set_color_for_character(int index, sf::Color clr) {
    m_renderables.at(std::size_t(index)).set_color(clr);
}

VectorF Text::character_location(int index) const {
    if (m_renderables.size() == std::size_t(index)) {
        return location() + VectorF(m_bounds.width, 0);
    } else if (m_renderables.size() > std::size_t(index)) {
        return m_renderables.at(std::size_t(index)).location();
    }
    throw std::out_of_range(
        "Text::character_location: index must be [0 len], where \"len\" is "
        "the length of this text's string.");
}

VectorF Text::location() const {
    return VectorF(m_bounds.left, m_bounds.top);
}

float Text::width() const { return m_bounds.width; }

float Text::height() const { return m_bounds.height; }

float Text::line_height() const {
    if (!has_font_assigned()) return 0.f;
    return font_ptr()->getLineSpacing(m_char_size);
}

const UString & Text::string() const
    { return m_string; }

bool Text::has_font_assigned() const
    { return font_ptr(); }

const sf::Font & Text::assigned_font() const {
    if (!font_ptr()) {
        throw std::runtime_error(
            "Text::assigned_font: cannot access font, no font has been assigned.");
    }
    return *font_ptr();
}

int Text::character_size() const { return m_char_size; }

TextSize Text::measure_text(const UString & ustring) const {
    return measure_text(assigned_font(), m_char_size, ustring);
}

float Text::measure_width(UStringConstIter beg, UStringConstIter end) {
    return Text::measure_width(assigned_font(), m_char_size, beg, end);
}

float Text::maximum_height(UStringConstIter beg, UStringConstIter end) {
    return Text::maximum_height(assigned_font(), m_char_size, beg, end);
}

bool Text::is_visible() const {
    return !m_string.empty() && has_font_assigned();
}

/* static */ TextSize Text::measure_text
    (const sf::Font & font, unsigned character_size, const UString & str)
{
    return measure_text(font, character_size, str.begin(), str.end());
}

/* static */ TextSize Text::measure_text
    (const sf::Font & font, int character_size,
     UStringConstIter beg, UStringConstIter end)
{
    if (character_size < 1) return TextSize();
    return TextSize { measure_width(font, character_size, beg, end),
                      font.getLineSpacing(character_size) };
}

/* static */ float Text::measure_width
    (const sf::Font & font, int character_size,
     UStringConstIter beg, UStringConstIter end)
{
    if (character_size < 1) return 0.f;
    assert(beg <= end);
    float w = 0.f;
    for (auto itr = beg; itr != end; ++itr) {
        const auto & glyph = font.getGlyph(*itr, character_size, false);
        w += glyph.advance;
        if (itr + 1 != end) {
            w += font.getKerning(*itr, *(itr + 1), character_size);
        }
    }
    return w;
}

/* static */ float Text::maximum_height
    (const sf::Font & font, int character_size,
     UStringConstIter beg, UStringConstIter end)
{
    if (character_size < 1) return 0.f;
    float h = 0.f;
    for (auto itr = beg; itr != end; ++itr) {
        h = std::max(h, font.getGlyph(*itr, character_size, false).bounds.height);
    }
    return h;
}

/* static */ void Text::run_tests() {
    {
    UString ustr = U"Hello World!";
    auto rv = find_chunks_dividers(ustr);
    assert(rv.size() == 3);
    assert(rv[0] - ustr.begin() == 5);
    assert(rv[1] - ustr.begin() == 6);
    assert(rv[2] == ustr.end());
    }
    {
    UString ustr = U"Hello\nWorld";
    auto rv = find_chunks_dividers(ustr);
    assert(rv.size() == 3);
    assert(rv[0] - ustr.begin() == 5);
    assert(rv[1] - ustr.begin() == 6);
    assert(rv[2] == ustr.end());
    }
    {
    UString ustr = U"Je \nk";
    auto rv = find_chunks_dividers(ustr);
    assert(rv.size() == 4);
    assert(rv[0] - ustr.begin() == 2);
    assert(rv[1] - ustr.begin() == 3);
    assert(rv[2] - ustr.begin() == 4);
    assert(rv[3] == ustr.end());
    }
}

/* private */ void Text::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!has_font_assigned()) return;
    states.texture = &font_ptr()->getTexture(unsigned(m_char_size));
    states.transform.translate(m_bounds.left, m_bounds.top);
    for (const auto & dc : m_renderables) {
        target.draw(dc, states);
    }
}

/* private */ const sf::Font * Text::font_ptr() const noexcept {
    if (m_font_ptr.is_type<const sf::Font *>()) {
        return m_font_ptr.as<const sf::Font *>();
    } else if (m_font_ptr.is_type<std::shared_ptr<const sf::Font>>()) {
        return m_font_ptr.as<std::shared_ptr<const sf::Font>>().get();
    } else {
        return nullptr;
    }
}

void Text::update_geometry() {
    if (!has_font_assigned() || m_char_size < 1 ||
        (m_string.empty() && m_renderables.empty()))
    { return; }

    place_renderables(m_renderables);
    cut_renderables  (m_renderables);

    m_bounds.width = m_bounds.height = 0.f;

    float right_most  = -k_inf;
    float bottom_most = -k_inf;
    for (const auto & dc : m_renderables) {
        right_most  = std::max(right_most , dc.location().x + dc.width ());
        bottom_most = std::max(bottom_most, dc.location().y + dc.height());
        assert(is_real(right_most) && is_real(bottom_most));
        assert(&dc >= &m_renderables.front() && &dc <= &m_renderables.back());
    }
    m_bounds.width  = std::max(0.f, right_most );
    m_bounds.height = std::max(0.f, bottom_most);
}

void Text::place_renderables(std::vector<detail::DrawableCharacter> & renderables) const {
    ::place_renderables(*font_ptr(), m_string, m_width_constraint,
                        m_char_size, m_color, renderables);
}

void Text::cut_renderables(std::vector<detail::DrawableCharacter> & renderables) const {
    ::cut_renderables(m_width_constraint, m_height_constraint, renderables);
}

} // end of asgl namespace

namespace {

std::vector<UString::const_iterator> find_chunks_dividers(const UString & ustr) {
    static const auto class_of_char = [](UChar c) {
        if (is_whitespace(c) && !is_newline(c)) return 0;
        if (is_newline   (c)) return 1;
        return 2;
    };
    std::vector<UString::const_iterator> rv;
    assert(!ustr.empty());
    int char_class = class_of_char(ustr[0]);

    for (auto itr = ustr.begin(); itr != ustr.end(); ++itr) {
        if (char_class == class_of_char(*itr)) continue;
        rv.push_back(itr);
        char_class = class_of_char(*itr);
    }
    rv.push_back(ustr.end());
    return rv;
}

void place_renderables(const sf::Font & font, const UString & ustr,
       float width_constraint, int char_size, sf::Color color,
       std::vector<DrawableCharacter> & renderables)
{
    renderables.clear();

    if (ustr.empty()) {
        // nothing to render
        return;
    }

    renderables.reserve(ustr.size());
    VectorF write_pos;
    auto itr = ustr.begin();
    for (auto chunk_end : find_chunks_dividers(ustr)) {
        assert(itr <= chunk_end);
        if (is_newline(*itr)) {
            write_pos.x = 0.f;
            write_pos.y += font.getLineSpacing(char_size);

            itr = chunk_end;
            continue;
        }

        auto chunk_width = asgl::Text::measure_width(font, char_size, itr, chunk_end);
        if (write_pos.x + chunk_width > width_constraint) {
            write_pos.x = 0.f;
            write_pos.y += font.getLineSpacing(char_size);
        }
        for (auto jtr = itr; jtr != chunk_end; ++jtr) {
            const auto & glyph = font.getGlyph(*jtr, char_size, false);
            VectorF p(write_pos.x + glyph.bounds.left, write_pos.y + glyph.bounds.top + char_size);
            renderables.emplace_back(p, glyph, color);
            write_pos.x += glyph.advance;
            if (jtr + 1 != ustr.end()) {
                write_pos.x += font.getKerning(*jtr, *(jtr + 1), char_size);
            }
        }
        itr = chunk_end;
    }
}

void cut_renderables(float width_constraint, float height_constraint,
                     std::vector<DrawableCharacter> & renderables)
{
    for (auto & renderable : renderables) {
        renderable.cut_on_bottom(height_constraint);
        renderable.cut_on_right (width_constraint );
    }
    renderables.erase(
        std::remove_if(renderables.begin(), renderables.end(),
                       [](const DrawableCharacter & dc) { return dc.whiped_out(); }),
        renderables.end());
}

} // end of <anonymous> namespace
