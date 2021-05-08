/****************************************************************************

    File: Text.hpp
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

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <vector>
#include <string>
#include <memory>
#include <limits>

#include <common/DrawRectangle.hpp>
#include <common/MultiType.hpp>

#include <asgl/StyleMap.hpp>
#include <asgl/DrawCharacter.hpp>

namespace asgl {

namespace detail {

using FontMtPtr = MultiType<const sf::Font *, std::shared_ptr<const sf::Font>>;

} // end of detail namespace

/** The following is a rewrite/extention/retraction of Laurent Gomila's
 *  sf::Text class.
 *
 *  This class is responsible for most geometric work involving the rendering
 *  of text. Some things, like how the text fits in the bigger picture, cannot
 *  be handled by this class.
 *
 *  Features:
 *  - can restrict it's text rendering to a rectangle.
 *  - handles multi line text restricted by width.
 *  - automatic word wrapping (greedy) based on restricted width
 */
class Text final : public sf::Drawable {
public:
    using UString          = std::u32string;
    using UChar            = UString::value_type;
    using UStringConstIter = UString::const_iterator;
    using VectorF          = sf::Vector2f;

    static constexpr const int   k_max_string_length = std::numeric_limits<int>::max();
    static constexpr const float k_inf               = std::numeric_limits<float>::infinity();
    static constexpr const int   k_default_font_size = 12;

    struct TextSize {
        float width = 0.f, height = 0.f;
    };

    void set_string(const UString & str);

    void set_string(UString && str);

    UString take_cleared_string();

    void set_limiting_width(float w);

    void set_limiting_height(float h);

    void enable_bottom_cutting();

    void disable_bottom_cutting();

    void set_limiting_dimensions(float w, float h);

    void relieve_width_limit();

    void relieve_height_limit();

    void relieve_size_limit();

    void set_character_size(int);

    void set_color(sf::Color);

    void set_location(float x, float y);

    // this needs to correspond 1:1 to the text's on screen location
    void set_location(VectorF r);

    void assign_font(const sf::Font *);

    void assign_font(const std::shared_ptr<const sf::Font> &);
#   if 0
    template <typename KeyType>
    bool assign_font(const StyleMap &, const KeyType &);
#   endif
    void set_color_for_character(int index, sf::Color clr);

    VectorF character_location(int index) const;

    VectorF location() const;

    float width() const;

    float height() const;

    float line_height() const;

    const UString & string() const;

    bool has_font_assigned() const;

    const sf::Font & assigned_font() const;

    /** @returns get_unset_value<int>() if character size is not yet set since
     *           object's creation.
     */
    int character_size() const;

    TextSize measure_text(const UString &) const;

    float measure_width(UStringConstIter beg, UStringConstIter end);

    float maximum_height(UStringConstIter beg, UStringConstIter end);

    bool is_visible() const;

    static TextSize measure_text
        (const sf::Font &, unsigned character_size, const UString &);

    /** Measures text in a single line like fashion.
     *  @param character_size a value less than 1 indicates a zero sized font
     *         and therefore will cause this function to always return a zerod
     *         size object { 0.f, 0.f }
     */
    static TextSize measure_text
        (const sf::Font &, int character_size, UStringConstIter beg, UStringConstIter end);

    static float measure_width
        (const sf::Font &, int character_size, UStringConstIter beg, UStringConstIter end);

    static float maximum_height
        (const sf::Font &, int character_size, UStringConstIter beg, UStringConstIter end);

    static void run_tests();

private:
    /** SFML draw, draws all verticies of the text.
     *  @param target Target of all draws.
     *  @param states States texture set, used to draw quads.
     */
    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    const sf::Font * font_ptr() const noexcept;

    void place_renderables(std::vector<detail::DrawableCharacter> &) const;

    // cuts/removes renderables that fall outside of width/height constraints
    void cut_renderables(std::vector<detail::DrawableCharacter> &) const;

    void update_geometry();

    using FontMtPtr = detail::FontMtPtr;
    FontMtPtr m_font_ptr;
    UString m_string;

    std::vector<detail::DrawableCharacter> m_renderables;
    // next iterator to the next chunk of text alternating between
    // breakable and unbreakable
    std::vector<UString::const_iterator> m_next_chunk;
    int m_char_size = k_default_font_size;// styles::get_unset_value<int>();
    sf::FloatRect m_bounds;
    float m_width_constraint = k_inf;
    float m_height_constraint = k_inf;
    bool m_allow_bottom_cuts = false;
    sf::Color m_color;
};
#if 0
template <typename KeyType>
bool Text::assign_font(const StyleMap & map, const KeyType & key_type) {
    auto itr = map.find(key_type);
    if (itr == map.end()) return false;
    auto mt = itr->second;
    if (mt.template is_type<const sf::Font *>()) {
        m_font_ptr = FontMtPtr(mt.template as<const sf::Font *>());
    } else if (mt.template is_type<std::shared_ptr<const sf::Font>>()) {
        m_font_ptr = FontMtPtr(mt.template as<std::shared_ptr<const sf::Font>>());
    } else {
        return false;
    }
    update_geometry();
    return true;
}
#endif
} // end of asgl namespace
