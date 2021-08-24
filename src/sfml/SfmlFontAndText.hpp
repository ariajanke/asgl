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

/** The following is a work derived from Laurent Gomila's and others work from
 *  the SFML.
 *
 *  Attached below is the original license.
 */

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

#include <asgl/Text.hpp>
#include <asgl/StyleMap.hpp>

#include "SfmlDrawCharacter.hpp"

#include <common/MultiType.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include <memory>
#include <string>

namespace asgl {

class SfmlFont;

namespace detail {

class TextWithFontStyle {
public:
    struct FontStyle {
        FontStyle() {}
        FontStyle(int chr_sz_, sf::Color c_): character_size(chr_sz_), color(c_) {}

        int character_size = 12;
        sf::Color color = sf::Color::White;
    };
    using FontStyleMap = std::map<StyleValue, FontStyle>;

    void stylize(StyleValue itemkey);

    virtual void set_character_size_and_color(int char_size, sf::Color) = 0;

    void set_font_styles_map(std::weak_ptr<FontStyleMap> mapptr)
        { m_font_styles = mapptr; }

private:
    std::weak_ptr<FontStyleMap> m_font_styles;
};

/** This helper class is used by an internal algorithm, by the SfmlText class.
 */
class RenderablesPlacer {
public:    
    using UCharIterVector = std::vector<UString::const_iterator>;
    using VectorF = sf::Vector2f;

    virtual ~RenderablesPlacer() {}

    virtual void operator () (VectorF loc, const sf::Glyph & glyph) = 0;
    virtual UCharIterVector give_old_cleared_container()
        { return UCharIterVector(); }
    virtual void take_old_container(UCharIterVector &&) {}
};

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
class SfmlText final :
    public TextBase, public detail::TextWithFontStyle, public sf::Drawable
{
public:
    using RectangleF = DrawableCharacter::RectangleF;
    SfmlText() {}
    SfmlText(const SfmlText &);
    SfmlText(SfmlText &&) = default;
    ~SfmlText() {}

    SfmlText & operator = (const SfmlText &);
    SfmlText & operator = (SfmlText &&) = default;

    void stylize(StyleValue itemkey) override
        { TextWithFontStyle::stylize(itemkey); }

    const UString & string() const override;

    void set_location(int x, int y) override;

    Vector location() const override;

    int width() const override;

    int height() const override;

    int full_width() const override;

    int full_height() const override;

    void set_limiting_line(int x_limit) override;

    Size measure_text(UStringConstIter beg, UStringConstIter end) const override;

    ProxyPointer clone() const override
        { return make_clone<SfmlText>(*this); }

    int limiting_line() const override;

    const Rectangle & viewport() const override;

    void assign_font(const sf::Font & font);

    void update_geometry();

    void set_character_size_and_color(int char_size, sf::Color) override;

private:
    void set_viewport_(const Rectangle &) override;

    void swap_string(UString & str) override;

    UString give_string_() override;

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    static constexpr const int   k_default_font_size = 12;
    static constexpr const float k_inf               = std::numeric_limits<float>::infinity();

    const sf::Font * m_font_ptr = nullptr;
    UString m_string;

    std::vector<detail::DrawableCharacter> m_renderables;

    std::unique_ptr<detail::RenderablesPlacer> m_placer_ptr;
    RectangleF m_full_bounds;
    float m_limiting_line = k_inf;
    Rectangle m_viewport = TextBase::k_default_viewport;

    int m_char_size = k_default_font_size;
    sf::Color m_color;
};

class SfmlFont final : public Font {
public:
    using FontStyle    = detail::SfmlText::FontStyle;
    using FontStyleMap = detail::SfmlText::FontStyleMap;

    TextPointer fit_pointer_to_adaptor(TextPointer && ptr) const override;

    Size measure_text
        (StyleValue fontstyle, UStringConstIter beg, UStringConstIter end) const override;

    void load_font(const std::string & filename);

    void add_font_style(StyleValue key, int char_size, sf::Color color);

    static Size measure_text(const sf::Font & font, int character_size,
                             UStringConstIter beg, UStringConstIter end);

private:
    std::unique_ptr<sf::Font> m_font;
    std::shared_ptr<FontStyleMap> m_font_styles;
};

} // end of detail namespace -> into ::asgl

} // end of asgl namespace
