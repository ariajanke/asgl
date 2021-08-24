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

#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>

#include <cassert>

namespace {

using namespace cul::exceptions_abbr;

} // end of <anonymous> namespace

namespace asgl {

Vector TextArea::location() const { return m_draw_text.location(); }

Size TextArea::size() const
    { return Size(m_draw_text.width(), std::max(m_draw_text.height(), m_height_fix)); }

void TextArea::stylize(const StyleMap & stylemap) {
    set_required_text_fields(
        m_draw_text, stylemap.find(styles::k_global_font),
        stylemap.find(asgl::to_key(frame_styles::k_widget_text_style)),
        "TextArea::stylize");
    flag_needs_whole_family_geometry_update();
}

void TextArea::set_string(const UString & str)
    { m_draw_text.set_string(str); }

void TextArea::set_string(UString && str)
    { m_draw_text.set_string(std::move(str)); }

UString TextArea::give_cleared_string()
    { return m_draw_text.give_cleared_string(); }

void TextArea::set_limiting_line(int x_limit) {
    m_draw_text.set_limiting_line(x_limit);
    flag_needs_whole_family_geometry_update();
}

void TextArea::set_fixed_height(int height) {
    Helpers::verify_non_negative(height, "TextArea::set_fixed_height", "height");
    m_height_fix = height;
    flag_needs_whole_family_geometry_update();
}

void TextArea::set_viewport(const Rectangle & rect) {
    m_draw_text.set_viewport(rect);
    flag_needs_whole_family_geometry_update();
}

void TextArea::reset_viewport() {
    m_draw_text.reset_viewport();
    flag_needs_whole_family_geometry_update();
}

const Rectangle & TextArea::viewport() const
    { return m_draw_text.viewport(); }

void TextArea::draw(WidgetRenderer & target) const
    { m_draw_text.draw_to(target); }

/* static */ void TextArea::set_required_text_fields
    (Text & text, const StyleField * font, const StyleField * style_key,
     const char * full_call)
{
    using FontPtr = std::weak_ptr<const Font>;
    auto make_error = [full_call](const char * what)
        { return RtError(std::string(full_call) + ": " + what); };
    if (!font) {
        throw make_error("no font found for style keys.");
    } else if (!font->is_type<FontPtr>()) {
        throw make_error("font item is not a font type (was the wrong key used?)");
    } else if (font->as<FontPtr>().expired()) {
        throw make_error("font pointer has expired (was the engine deleted?)");
    }
    if (!style_key) {
        throw make_error("no text style key found.");
    } else if (!style_key->is_type<StyleValue>()) {
        throw make_error("text style is not an item key.");
    } else if (style_key->as<StyleValue>() == StyleValue()) {
        throw make_error("text style item key is the default (null) key.");
    }
    const auto & font_ = *font->as<FontPtr>().lock();
    text.set_font(font_);
    text.stylize(style_key->as<StyleValue>());
}

/* private */ void TextArea::set_location_(int x, int y) {
    if (m_height_fix == 0) {
        m_draw_text.set_location(x, y);
        m_controls_y = m_draw_text.location().y;
    } else {
        // should have been taken care of by now
        assert(m_draw_text.height() <= m_height_fix);
        m_draw_text.set_location(x, y + (m_height_fix - m_draw_text.height()) / 2);
        m_controls_y = y;
    }
}

void TextArea::update_size() {
    if (m_height_fix == 0) return;
    static const UString k_est_min = U"a";
    m_height_fix = std::max(
        m_height_fix,
        m_draw_text.measure_text(k_est_min.begin(), k_est_min.end()).height);
    check_and_adjust_for_text_too_big();
}

void TextArea::check_and_adjust_for_text_too_big() {
    if (m_draw_text.height() <= m_height_fix) return;
    using UStrIter = UStringConstIter;
    UString whole_string = m_draw_text.string();
    if (whole_string.empty()) {
        throw RtError("bad");
    }
    auto rbeg = whole_string.begin();
    auto rend = whole_string.end  ();
    // have to find a shorten string
    // have to copy u.u
    auto runder = rbeg + ((rend - rbeg) / 2);
    static auto take_and_append = [](UStrIter beg, UStrIter end, UString && target) {
        target.insert(target.begin(), beg, end);
        target += U"...";
        return std::move(target);
    };
    // find any under
    while (true) {
        m_draw_text.set_string( take_and_append(whole_string.begin(), runder, m_draw_text.give_cleared_string()) );
        bool under_fits = m_draw_text.height() <= m_height_fix;
        m_draw_text.set_string( take_and_append(whole_string.begin(), runder + 1, m_draw_text.give_cleared_string()) );
        bool over_too_big = m_draw_text.height() > m_height_fix;
        if (under_fits && over_too_big) break;
        if (under_fits && !over_too_big) {
            if (rend - rbeg == 1) break;
            // go right
            rbeg = runder + 1;
        } else if (!under_fits && over_too_big) {
            // go left
            rend = runder;
        } else {
            throw RtError("bad branch");
        }
        runder = rbeg + ((rend - rbeg) / 2);
    }
    m_draw_text.set_string( take_and_append(whole_string.begin(), runder, m_draw_text.give_cleared_string()) );
}

} // end of asgl namespace
