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

#include <asgl/EditableText.hpp>
#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/OptionsSlider.hpp>

#include <cassert>

namespace {

using UString          = asgl::Text::UString;
using RtError          = std::runtime_error;
using InvArg           = std::invalid_argument;
using UStringConstIter = asgl::Text::UStringConstIter;
using OutOfRange       = std::out_of_range;
using UChar            = asgl::Text::UString::value_type;
using Text             = asgl::Text;

template <typename ... Types>
using Tuple = std::tuple<Types...>;

Tuple<bool, UStringConstIter> find_display_position
    (const UString & display_string, const UString & entered_string);

Tuple<bool, UStringConstIter> find_display_position
    (const UString & display_string, const UString & entered_string,
     std::size_t pos);

inline bool is_control_char(UChar chr)
    { return (chr < 32 || (chr >= 127 && chr < 256)); }

inline void set_string(Text & text, UStringConstIter beg, UStringConstIter end) {
    auto ustr = text.give_cleared_string();
    ustr.insert(ustr.begin(), beg, end);
    text.set_string(std::move(ustr));
}

} // end of <anonymous> namespace

namespace asgl {

void EditableText::set_text_width(int new_width) {
    Helpers::verify_non_negative(
        new_width, "EditableText::set_text_width", "text width");
    m_chosen_width = new_width;
    flag_needs_whole_family_geometry_update();
    check_invarients();
}

void EditableText::set_text_width_to_match_empty_text() {
    m_chosen_width = styles::k_uninit_size;
    flag_needs_whole_family_geometry_update();
    check_invarients();
}

void EditableText::set_check_string_event(StringCheckFunc && func)
    { m_string_check_func = std::move(func); }

void EditableText::process_event(const Event & event) {
    switch (event.type_id()) {
    case k_event_id_of<MouseRelease>:
        if (bounds().contains(to_vector(event.as<MouseRelease>()))) {
            request_focus();
        }
        break;
    default: break;
    }
    check_invarients();
}

int EditableText::width() const
    { return text_width() + cursor_width() + m_padding*2; }

int EditableText::height() const
    { return text_height() + m_padding*2; }

void EditableText::stylize(const StyleMap & stylemap) {
    auto set_fields_for_fill_text = [&stylemap]
        (decltype(k_style_count) style, Text & text)
    {
        TextArea::set_required_text_fields(
            text, stylemap.find(styles::k_global_font),
            stylemap.find(to_key(style), Frame::to_key(Frame::k_widget_text_style)),
            "EditableText::stylize");
    };
    set_fields_for_fill_text(k_fill_text_style , m_display_left );
    set_fields_for_fill_text(k_fill_text_style , m_display_right);
    set_fields_for_fill_text(k_empty_text_style, m_empty_text   );

    m_padding = Helpers::verify_padding
        (stylemap.find(styles::k_global_padding), "EditableText::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("EditableText::stylize", {
        make_tuple(&m_border_appearance, "border",
                   stylemap.find(to_key(k_widget_border_style),
                                 OptionsSlider::to_key(OptionsSlider::k_back_style))),
        make_tuple(&m_border_hover_appearance, "border (hover)",
                   stylemap.find(to_key(k_widget_border_on_hover),
                                 OptionsSlider::to_key(OptionsSlider::k_front_style))),
        make_tuple(&m_area_appearance, "text area",
                   stylemap.find(to_key(k_text_background_style),
                                 Frame::to_key(Frame::k_widget_text_style))),
        make_tuple(&m_cursor_appearance, "cursor",
                   stylemap.find(to_key(k_cursor_style),
                                 Frame::to_key(Frame::k_widget_text_style)))
    });

    flag_needs_whole_family_geometry_update();
}
#if 0
void EditableText::update_geometry() {
    const auto & any_display_text = m_display_left;
    const auto & disp             = m_display_string;

    auto itr     = find_display_position(disp, m_entered_string, m_edit_position);
    int on_left  = any_display_text.measure_text(disp.begin(), itr).width;
    int on_right = any_display_text.measure_text(itr, disp.end()).width;

    m_cursor.width  = cursor_width();
    m_cursor.height = text_height ();

    auto set_cursor_location = [this](int x_offset) {
        m_cursor.left = m_location.x + x_offset + m_padding,
        m_cursor.top  = m_location.y + m_padding;
    };

    set_string(m_display_left , disp.begin(), itr);
    set_string(m_display_right, itr, disp.end());
    m_display_left.set_location( m_location + VectorI(1, 1)*m_padding );
    m_empty_text.set_location( m_display_left.location() );
    if (on_left + on_right <= text_width()) {
        // no ellipse
        set_cursor_location(on_left);
        m_display_right.set_location(
            m_display_left.location() + VectorI(on_left + cursor_width(), 0) );
    } else {
        int set_aside_right = on_right;
        if (on_right > text_width() / 2) {
            set_aside_right = text_width() / 2;
        }

        int set_aside_left = text_width() - set_aside_right;
        assert((on_left + on_right) >= set_aside_left);
        sf::IntRect left_viewport((on_left + on_right) - set_aside_left, 0,
                                  set_aside_left, text_height());
        sf::IntRect right_viewport(0, 0, set_aside_right, text_height());

        set_cursor_location(set_aside_left);
        m_display_right.set_location(
            m_display_left.location() + VectorI(set_aside_left + cursor_width(), 0) );
        m_display_left .set_viewport(left_viewport );
        m_display_right.set_viewport(right_viewport);
    }

    check_invarients();
}
#endif
void EditableText::draw(WidgetRenderer & target) const {
    draw_to(target, bounds(),
            has_focus() ? m_border_hover_appearance : m_border_appearance);
    auto inner = bounds();
    set_top_left(inner, m_location + VectorI(1, 1)*m_padding);
    inner.width -= m_padding*2;
    inner.height -= m_padding*2;
    draw_to(target, inner, m_area_appearance);
    if (m_display_left.string().empty() && m_display_right.string().empty()) {
        m_empty_text.draw_to(target);
    } else {
        m_display_left.draw_to(target);
        m_display_right.draw_to(target);
    }
    if (has_focus()) {
        draw_to(target, m_cursor, m_cursor_appearance);
    }
}

void EditableText::set_empty_string(const UString & empt_str) {
    m_empty_text.set_string(empt_str);
}

void EditableText::set_entered_string(const UString & new_string) {
    auto disp = std::move(m_display_string);
    bool all_good = false;
    if (( all_good = m_string_check_func(new_string, disp) )) {
        m_display_string = std::move(disp);
        m_entered_string = new_string;
    }

    flag_needs_individual_geometry_update();
    check_invarients();
    if (!all_good) {
        throw InvArg("EditableText::set_entered_string: string entered was "
                     "not accepted by the set string checker function."     );
    }
}

const UString & EditableText::entered_string() const
    { return m_entered_string; }

/* static */ bool EditableText::default_check_string_event
    (const UString & new_string, UString & display_string)
{
    if (std::any_of(new_string.begin(), new_string.end(), is_control_char)) {
        return false;
    }
    display_string = new_string;
    return true;
}

/* static */ bool EditableText::is_display_string_ok
    (const UString & display_string, const UString & entered_string)
{ return std::get<0>(::find_display_position(display_string, entered_string)); }

/* static */ UStringConstIter EditableText::find_display_position
    (const UString & display_string, const UString & entered_string,
     std::size_t pos)
{
    auto gv = ::find_display_position(display_string, entered_string, pos);
    if (!std::get<0>(gv)) {
        throw InvArg("EditableText::find_display_position: provided display "
                     "string does not contain an in-order sub-sequence that "
                     "is equivalent to the entered string.");
    }
    return std::get<1>(gv);
}

/* private */ void EditableText::set_location_(int x, int y) {
    m_location = VectorI(x, y);
    update_internals_locations();
}

/* private */ void EditableText::update_size() {
    if (m_chosen_width == styles::k_uninit_size) {
        m_used_width = m_empty_text.full_width();
    } else {
        m_used_width = m_chosen_width;
    }
}

/* private */ void EditableText::process_focus_event(const Event & event) {
    switch (event.type_id()) {
    case k_event_id_of<KeyTyped>:
        handle_focused_key_typed(event.as<KeyTyped>());
        break;
    case k_event_id_of<KeyPress>:
        handle_focused_key_press(event.as<KeyPress>());
        break;
    default: break;
    }
    check_invarients();
}

/* private */ void EditableText::notify_focus_gained() {}

/* private */ void EditableText::notify_focus_lost() {}

/* private */ int EditableText::text_width() const {
    return m_used_width;
#   if 0
    if (m_chosen_width == styles::k_uninit_size) {
        return m_empty_text.full_width();
    }
    return m_chosen_width;
#   endif
}

/* private */ int EditableText::text_height() const {
    const static UString k_sample_text = U"a";
    const auto & any_display_text = m_display_left;
    return any_display_text.measure_text(
        k_sample_text.begin(), k_sample_text.end()).height;
}

/* private */ void EditableText::check_invarients() const {
    assert(is_display_string_ok(m_display_string, m_entered_string));
    assert(m_padding >= 0);
    assert(m_chosen_width >= 0 || m_chosen_width == styles::k_uninit_size);
}

/* private */ void EditableText::handle_focused_key_typed(const KeyTyped & keytyped) {
    auto ustr = std::move(m_entered_string);
    {
    auto disp = m_display_string;
    ustr.insert(ustr.begin() + m_edit_position, keytyped.code);
    if (m_string_check_func(ustr, disp)) {
        ++m_edit_position;
        m_display_string = std::move(disp);
    } else {
        ustr.erase(ustr.begin() + m_edit_position);
    }
    }
    flag_needs_individual_geometry_update();
    if (!is_display_string_ok(m_display_string, ustr)) {
        m_display_string = U"";
        check_invarients();
        throw RtError("EditableText::process_focus_event: string check "
                      "function returned a display string which does not "
                      "contain the entered string's character in the same "
                      "sequence.");
    }
    m_entered_string = std::move(ustr);
}

/* private */ void EditableText::handle_focused_key_press(const KeyPress & keypress) {
    switch (keypress.key) {
    case keys::k_delete:
        if (m_entered_string.size() != m_edit_position) {
            (void)delete_character_at(m_edit_position);
            flag_needs_individual_geometry_update();
        }
        break;
    case keys::k_backspace:
        if (0 != m_edit_position) {
            if (delete_character_at(m_edit_position - 1))
                --m_edit_position;
            flag_needs_individual_geometry_update();
        }
        break;
    case keys::k_end:
        m_edit_position = m_entered_string.size();
        flag_needs_individual_geometry_update();
        break;
    case keys::k_home:
        m_edit_position = 0;
        flag_needs_individual_geometry_update();
        break;
    case keys::k_left:
        if (m_edit_position != 0) {
            --m_edit_position;
            flag_needs_individual_geometry_update();
        }
        break;
    case keys::k_right:
        if (m_edit_position != m_entered_string.size()) {
            ++m_edit_position;
            flag_needs_individual_geometry_update();
        }
        break;
    default: break;
    }
}

/* private */ bool EditableText::delete_character_at(std::size_t p) {
    assert(p < m_entered_string.size());
    auto chr_deleted = m_entered_string[p];
    m_entered_string.erase(m_entered_string.begin() + p);
    auto disp = m_display_string;
    if (!m_string_check_func(m_entered_string, disp)) {
        m_entered_string.insert(m_entered_string.begin() + p, chr_deleted);
        assert(m_string_check_func(m_entered_string, disp));
        return false;
    }
    m_display_string = std::move(disp);
    return true;
}

/* private */ sf::IntRect EditableText::bounds() const
    { return sf::IntRect(location().x, location().y, width(), height()); }

/* private */ void EditableText::update_internals_locations() {
    const auto & any_display_text = m_display_left;
    const auto & disp             = m_display_string;

    auto itr     = find_display_position(disp, m_entered_string, m_edit_position);
    int on_left  = any_display_text.measure_text(disp.begin(), itr).width;
    int on_right = any_display_text.measure_text(itr, disp.end()).width;

    m_cursor.width  = cursor_width();
    m_cursor.height = text_height ();

    auto set_cursor_location = [this](int x_offset) {
        m_cursor.left = m_location.x + x_offset + m_padding,
        m_cursor.top  = m_location.y + m_padding;
    };

    set_string(m_display_left , disp.begin(), itr);
    set_string(m_display_right, itr, disp.end());
    m_display_left.set_location( m_location + VectorI(1, 1)*m_padding );
    m_empty_text.set_location( m_display_left.location() );
    if (on_left + on_right <= text_width()) {
        // no ellipse
        set_cursor_location(on_left);
        m_display_right.set_location(
            m_display_left.location() + VectorI(on_left + cursor_width(), 0) );
    } else {
        int set_aside_right = on_right;
        if (on_right > text_width() / 2) {
            set_aside_right = text_width() / 2;
        }

        int set_aside_left = text_width() - set_aside_right;
        assert((on_left + on_right) >= set_aside_left);
        sf::IntRect left_viewport((on_left + on_right) - set_aside_left, 0,
                                  set_aside_left, text_height());
        sf::IntRect right_viewport(0, 0, set_aside_right, text_height());

        set_cursor_location(set_aside_left);
        m_display_right.set_location(
            m_display_left.location() + VectorI(set_aside_left + cursor_width(), 0) );
        m_display_left .set_viewport(left_viewport );
        m_display_right.set_viewport(right_viewport);
    }

    check_invarients();
}

} // end of asgl namespace

namespace {

Tuple<bool, UStringConstIter> find_display_position
    (const UString & display_string, const UString & entered_string)
{
    return find_display_position(display_string, entered_string,
                                 entered_string.size());
}

Tuple<bool, UStringConstIter> find_display_position
    (const UString & display_string, const UString & entered_string,
     std::size_t pos)
{
    using std::make_tuple;
    if (pos > entered_string.size()) {
        throw OutOfRange("EditableText::right_before_next: given position is "
                         "outside the string.");
    }

    assert(display_string.size() >= entered_string.size());
    auto disp_itr = display_string.begin();
    auto entr_itr = entered_string.begin();
    for (; entr_itr != entered_string.end(); ++entr_itr) {
        while (*entr_itr != *disp_itr) {
            ++disp_itr;
            if (disp_itr == display_string.end()) {
                return make_tuple(false, UStringConstIter());
            }
        }
        if (pos == 0) return make_tuple(true, disp_itr);
        --pos;
        ++disp_itr;
    }
    if (pos == 0) return make_tuple(true, disp_itr);
    return make_tuple(false, UStringConstIter());
}

} // end of <anonymous> namespace
