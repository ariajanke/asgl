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

#include <asgl/DrawCharacter.hpp>
#include <asgl/StyleMap.hpp>

#include <common/MultiType.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include <memory>
#include <string>

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
class SfmlTextObject final : public sf::Drawable {
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

    UString take_string();

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

    TextSize measure_text(UStringConstIter beg, UStringConstIter end) const;

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

class SfmlFontN;

class SfmlTextN final : public TextBase {
public:
    struct FontStyle {
        FontStyle() {}
        FontStyle(int chr_sz_, sf::Color c_): character_size(chr_sz_), color(c_) {}

        int character_size = 12;
        sf::Color color = sf::Color::White;
    };
    using FontStyleMap = std::map<ItemKey, FontStyle>;

    const UString & string() const override { return m_object.string(); }

    void set_location(int x, int y) override {
        m_object.set_location(float(x), float(y));
    }

    VectorI location() const override {
        return VectorI(m_object.location());
    }

    int width() const override {
        return int(m_object.width());
    }

    int height() const override {
        return int(m_object.height());
    }

    int full_width() const override {
        return int(m_object.width());
    }

    int full_height() const override {
        return int(m_object.height());
    }

    void set_limiting_line(int x_limit) override {
        m_line_lim = x_limit;
        m_object.set_limiting_width(float(x_limit));
    }

    void stylize(ItemKey itemkey) override {
        if (m_font_styles.expired()) {
            throw std::runtime_error("");
        }
        auto ptr = m_font_styles.lock();
        auto itr = ptr->find(itemkey);
        if (itr == ptr->end()) {
            throw std::runtime_error("");
        }
        m_object.set_character_size(itr->second.character_size);
        m_object.set_color(itr->second.color);
    }

    TextSize measure_text(UStringConstIter beg, UStringConstIter end) const override {
        auto rv1t = m_object.measure_text(beg, end);
        TextSize rv2t;
        rv2t.width = rv1t.width;
        rv2t.height = rv1t.height;
        return rv2t;
    }

    ProxyPointer clone() const override {
        return make_clone<SfmlTextN>(*this);
    }

    int limiting_line() const override {
        return m_line_lim;
    }

    const sf::IntRect & viewport() const override {
        // idk what to do here yet
        static const sf::IntRect inst;
        return inst;
    }

    const SfmlTextObject & text_object() const { return m_object; }

    void set_font(const sf::Font & font)
        { m_object.assign_font(&font); }

    void set_font_styles_map(std::weak_ptr<FontStyleMap> mapptr)
        { m_font_styles = mapptr; }

protected:
    void set_viewport_(const sf::IntRect &) override {}

    void swap_string(UString & str) override {
        auto t = m_object.string();
        m_object.set_string(std::move(str));
        str = t;
    }

    UString give_string_() override {
        return m_object.take_string();
    }

    SfmlTextObject m_object;
    int m_line_lim = 0;

    std::weak_ptr<FontStyleMap> m_font_styles;
};

class SfmlFontN final : public Font {
public:
    using FontStyle = SfmlTextN::FontStyle;
    using FontStyleMap = SfmlTextN::FontStyleMap;

    TextPointer fit_pointer_to_adaptor(TextPointer && ptr) const override {
        if (!m_font || !m_font_styles) {
            throw std::runtime_error("");
        }
        // no idea what to do with the rv
        auto & text = check_and_transform_text<SfmlTextN>(ptr);
        text.set_font(*m_font);
        text.set_font_styles_map(m_font_styles);

        return std::move(ptr);
    }

    TextSize measure_text
        (ItemKey fontstyle, UStringConstIter beg, UStringConstIter end) const override
    {
        auto make_not_found_error = []() { return std::runtime_error(""); };
        if (!m_font_styles) throw make_not_found_error();
        auto itr = m_font_styles->find(fontstyle);
        if (itr == m_font_styles->end()) throw make_not_found_error();
        auto rv1t = SfmlTextObject::measure_text(*m_font, itr->second.character_size, beg, end);
        TextSize rv2t;
        rv2t.width = rv1t.width;
        rv2t.height = rv1t.height;
        return rv2t;
    }

    void load_font(const std::string & filename) {
        if (!m_font) {
            m_font = std::make_unique<sf::Font>();
        }
        m_font->loadFromFile(filename);
    }

    void add_font_style(ItemKey key, int char_size, sf::Color color) {
        m_font_styles = (m_font_styles ? m_font_styles : std::make_shared<FontStyleMap>());
        auto gv = m_font_styles->insert(std::make_pair(key, FontStyle(char_size, color) ));
        if (gv.second) return;
        throw std::runtime_error("");
    }

private:
    std::unique_ptr<sf::Font> m_font;
    std::shared_ptr<FontStyleMap> m_font_styles;
};

} // end of asgl namespace
