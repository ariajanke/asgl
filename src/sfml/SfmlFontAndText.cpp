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

#include "SfmlFontAndText.hpp"
#include <common/Util.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <array>
#include <memory>
#include <algorithm>

#include <cmath>
#include <cassert>

namespace {

using UChar             = asgl::detail::SfmlText::UString::value_type;
using UString           = asgl::detail::SfmlText::UString;
using TextSize          = asgl::detail::SfmlText::TextSize;
using VectorF           = sf::Vector2f;
using LineBreakList     = std::vector<int>;
using VertexContainer   = std::vector<sf::Vertex>;
using InvalidArg        = std::invalid_argument;
using DrawableCharacter = asgl::detail::DrawableCharacter;
using UCharIterVector   = std::vector<UString::const_iterator>;
using RenderablesPlacer = asgl::detail::RenderablesPlacer;

bool is_whitespace(UChar c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool is_newline   (UChar c) { return c == '\n'; }

class AlgoPlacer final : public RenderablesPlacer {
public:
    void operator () (VectorF loc, const sf::Glyph & glyph) override {
        renderables->emplace_back(loc, glyph, *color);
        auto & new_dc = renderables->back();
        new_dc.cut_outside_of(sf::FloatRect(*viewport));
        if (new_dc.whiped_out()) {
            renderables->pop_back();
            return;
        }
        full_bounds->width  = std::max(full_bounds->width , new_dc.location().x + new_dc.width ());
        full_bounds->height = std::max(full_bounds->height, new_dc.location().y + new_dc.height());
    }

    UCharIterVector give_old_cleared_container() override
        { return std::move(m_cont); }

    void take_old_container(UCharIterVector && cont) override {
        cont.clear();
        m_cont = std::move(cont);
    }

    std::vector<DrawableCharacter> * renderables = nullptr;
    const sf::Color * color = nullptr;
    sf::FloatRect * full_bounds = nullptr;
    const sf::IntRect * viewport = nullptr;

private:
    UCharIterVector m_cont;
};

void place_renderables(const sf::Font & font, const UString & ustr,
       float width_constraint, int char_size,
       RenderablesPlacer & placer);

} // end of <anonymous> namespace

namespace asgl {

namespace detail {

void TextWithFontStyle::stylize(ItemKey itemkey) {
    auto make_error = [](const char * what)
        { return std::runtime_error("TextWithFontStyle::stylize: " + std::string(what)); };
    if (m_font_styles.expired()) {
        throw make_error("Font styles map is missing.");
    }
    auto ptr = m_font_styles.lock();

    auto itr = ptr->find(itemkey);
    if (itr == ptr->end()) {
        throw make_error("Itemkey is not found on map.");
    }
    set_character_size_and_color(itr->second.character_size, itr->second.color);
}

// ----------------------------------------------------------------------------

SfmlText::SfmlText(const SfmlText & rhs):
    m_font_ptr     (rhs.m_font_ptr     ),
    m_string       (rhs.m_string       ),
    m_renderables  (rhs.m_renderables  ),
    m_placer_ptr   (nullptr            ),
    m_full_bounds  (rhs.m_full_bounds  ),
    m_limiting_line(rhs.m_limiting_line),
    m_viewport     (rhs.m_viewport     ),
    m_char_size    (rhs.m_char_size    ),
    m_color        (rhs.m_color        )
{}

SfmlText & SfmlText::operator = (const SfmlText & rhs) {
    if (this != &rhs) {
        SfmlText temp(rhs);
        // sorry I don't want to implement a swap method
        (*this) = std::move(rhs);
    }
    return *this;
}

const UString & SfmlText::string() const { return m_string; }

void SfmlText::set_location(int x, int y) {
    m_full_bounds.left = float(x);
    m_full_bounds.top  = float(y);
}

SfmlText::VectorI SfmlText::location() const {
    return VectorI(int(std::round(m_full_bounds.left)), int(std::round(m_full_bounds.top)));
}

int SfmlText::width() const {
    static const auto k_def_width = k_default_viewport.width;
    return m_viewport.width == k_def_width ? full_width() : m_viewport.width;
}

int SfmlText::height() const {
    static const auto k_def_height = k_default_viewport.height;
    return m_viewport.height == k_def_height ? full_height() : m_viewport.height;
}

int SfmlText::full_width() const
    { return int(std::round(m_full_bounds.width)); }

int SfmlText::full_height() const
    { return int(std::round(m_full_bounds.height)); }

void SfmlText::set_limiting_line(int x_limit) {
    m_limiting_line = float(x_limit);
}

SfmlText::TextSize SfmlText::measure_text(UStringConstIter beg, UStringConstIter end) const {
    if (!m_font_ptr || m_char_size == 0) {
        return TextSize();
    }
    return SfmlFont::measure_text(*m_font_ptr, m_char_size, beg, end);
}

int SfmlText::limiting_line() const { return int(std::round(m_limiting_line)); }

const sf::IntRect & SfmlText::viewport() const { return m_viewport; }

void SfmlText::assign_font(const sf::Font & font) {
    m_font_ptr = &font;
}

void SfmlText::update_geometry() {
    if (!m_font_ptr || m_char_size == 0) {
        m_renderables.clear();
        return;
    }

    // we'll pay a dynamic allocation fee
    // to save on further reallocation for "chunk dividers" used by the placer
    // algorithm
    if (!m_placer_ptr) {
        auto placer = std::make_unique<AlgoPlacer>();
        placer->color       = &m_color;
        placer->full_bounds = &m_full_bounds;
        placer->viewport    = &m_viewport;
        placer->renderables = &m_renderables;
        m_placer_ptr = std::move(placer);
    }

    // the "min" height
    m_full_bounds.height = 0.f;
    m_full_bounds.width  = 0.f;

    m_renderables.clear();
    m_renderables.reserve(m_string.size());
    ::place_renderables(*m_font_ptr, m_string, m_limiting_line, m_char_size, *m_placer_ptr);
}

void SfmlText::set_character_size_and_color
    (int char_size, sf::Color color)
{
    m_char_size = char_size;
    m_color     = color;
    update_geometry();
}

/* private */ void SfmlText::set_viewport_(const sf::IntRect & rect) {
    m_viewport = rect;
}

/* private */ void SfmlText::swap_string(UString & str) {
    m_string.swap(str);
    update_geometry();
}

/* private */ UString SfmlText::give_string_() {
    m_renderables.clear();
    return std::move(m_string);
}

/* private */ void SfmlText::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    if (!m_font_ptr) return;
    states.texture = &m_font_ptr->getTexture(unsigned(m_char_size));
    states.transform.translate(m_full_bounds.left - float(m_viewport.left),
                               m_full_bounds.top  - float(m_viewport.top ));
    for (const auto & dc : m_renderables) {
        target.draw(dc, states);
    }
}

// ----------------------------------------------------------------------------

SfmlFont::TextPointer SfmlFont::fit_pointer_to_adaptor(TextPointer && ptr) const {
    if (!m_font || !m_font_styles) {
        throw std::runtime_error("SfmlFont::fit_pointer_to_adaptor: underlying font pointer and styles pointer is not set, not able to transform text pointer.");
    }
    // no idea what to do with the rv
    auto & text = check_and_transform_text<detail::SfmlText>(ptr);
    text.assign_font(*m_font);
    text.set_font_styles_map(m_font_styles);

    return std::move(ptr);
}

TextSize SfmlFont::measure_text
    (ItemKey fontstyle, UStringConstIter beg, UStringConstIter end) const
{
    auto make_not_found_error = []()
        { return std::runtime_error("SfmlFont::measure_text: cannot find font style for given item key."); };
    if (!m_font_styles) throw make_not_found_error();
    auto itr = m_font_styles->find(fontstyle);
    if (itr == m_font_styles->end()) throw make_not_found_error();
    return measure_text(*m_font, itr->second.character_size, beg, end);
}

void SfmlFont::load_font(const std::string & filename) {
    if (!m_font) {
        m_font = std::make_unique<sf::Font>();
    }
    if (!m_font->loadFromFile(filename)) {
        m_font = nullptr;
        throw std::invalid_argument("SfmlFont::load_font: cannot load font \"" + filename + "\".");
    }
}

void SfmlFont::add_font_style(ItemKey key, int char_size, sf::Color color) {
    m_font_styles = (m_font_styles ? m_font_styles : std::make_shared<FontStyleMap>());
    auto gv = m_font_styles->insert(std::make_pair(key, FontStyle(char_size, color) ));
    if (gv.second) return;
    throw std::runtime_error("SfmlFont::add_font_style: Failed to insert font style, dupelicate item key.");
}

/* static */ TextSize SfmlFont::measure_text
    (const sf::Font & font, int character_size,
     UStringConstIter beg, UStringConstIter end)
{
    if (character_size < 1) return TextSize();
    assert(beg <= end);
    float w = 0.f;
    for (auto itr = beg; itr != end; ++itr) {
        const auto & glyph = font.getGlyph(*itr, character_size, false);
        w += glyph.advance;
        if (itr + 1 != end) {
            w += font.getKerning(*itr, *(itr + 1), character_size);
        }
    }

    TextSize rv2;
    rv2.width  = int(std::round(w));
    rv2.height = int(std::round(font.getLineSpacing(character_size)));
    return rv2;
}

} // end of detail namespace -> into ::asgl

} // end of asgl namespace

namespace {

// each iterator is a chunk begining
// we can and SHOULD test this! :)
UCharIterVector find_chunks_dividers(const UString & ustr, UCharIterVector && = UCharIterVector());

void place_renderables
    (const sf::Font & font, const UString & ustr, float width_constraint,
     int char_size, RenderablesPlacer & placer)
{
    // nothing to render
    if (ustr.empty()) return;

    VectorF write_pos;
    auto itr = ustr.begin();
    auto chunks = find_chunks_dividers(ustr, placer.give_old_cleared_container());
    for (auto chunk_end : chunks) {
        assert(itr <= chunk_end);
        if (is_newline(*itr)) {
            write_pos.x = 0.f;
            write_pos.y += font.getLineSpacing(char_size);

            itr = chunk_end;
            continue;
        }

        auto chunk_width = float( asgl::detail::SfmlFont::measure_text(font, char_size, itr, chunk_end).width );
        if (write_pos.x + chunk_width > width_constraint) {
            write_pos.x = 0.f;
            write_pos.y += font.getLineSpacing(char_size);
        }
        for (auto jtr = itr; jtr != chunk_end; ++jtr) {
            const auto & glyph = font.getGlyph(*jtr, char_size, false);
            VectorF p(write_pos.x + glyph.bounds.left, write_pos.y + glyph.bounds.top + char_size);
            placer(p, glyph);
            write_pos.x += glyph.advance;
            if (jtr + 1 != ustr.end()) {
                write_pos.x += font.getKerning(*jtr, *(jtr + 1), char_size);
            }
        }
        itr = chunk_end;
    }
    placer.take_old_container(std::move(chunks));
}

// ----------------------------------------------------------------------------

UCharIterVector find_chunks_dividers
    (const UString & ustr, UCharIterVector && old_cont)
{
    static const auto class_of_char = [](UChar c) {
        if (is_whitespace(c) && !is_newline(c)) return 0;
        if (is_newline   (c)) return 1;
        return 2;
    };
    assert(old_cont.empty());
    UCharIterVector rv = std::move(old_cont);

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

} // end of <anonymous> namespace
