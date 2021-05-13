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

#pragma once

#include <common/MultiType.hpp>

// needed for to_vector
#include <SFML/System/Vector2.hpp>

namespace asgl {

namespace keys {

enum Key {
    // printables
    k_tilde,
    k_zero, k_one, k_two, k_three, k_four, k_five, k_six, k_seven, k_eight,
    k_nine, // heheh
    // pad values collapse into the above
    k_pad_zero, k_pad_one, k_pad_two  , k_pad_three, k_pad_four,
    k_pad_five, k_pad_six, k_pad_seven, k_pad_eight, k_pad_nine,
    k_dash,
    k_equal,
    k_a, k_b, k_c, k_d, k_e, k_f, k_g, k_h, k_i, k_j, k_k, k_l, k_m, k_n, k_o,
    k_p, k_q, k_r, k_s, k_t, k_u, k_v, k_w, k_x, k_y, k_z,
    k_left_bracket,
    k_right_bracket,
    k_semicolon,
    k_comma,
    k_period,
    k_quote,
    k_foreslash,
    k_backslash,
    k_space,
    k_enter,
    k_tab,
    k_plus,
    k_minus,
    k_multiplication,
    k_division,
    // end of printables
    k_escape,
    k_f1, k_f2, k_f3, k_f4, k_f5, k_f6, k_f7, k_f8, k_f9, k_f10, k_f11, k_f12,
    k_menu,
    k_backspace,
    k_pageup,
    k_pagedown,
    k_end,
    k_home,
    k_insert,
    k_delete,
    k_left,
    k_right,
    k_up,
    k_down,
    k_pause,
    // the keys without preface are only possible if collapse is called
    k_control,
    k_shift,
    k_alt,
    k_system,
    // these values are possible without collapse calls
    k_left_control,
    k_left_shift,
    k_left_alt,
    k_left_system,
    k_right_control,
    k_right_shift,
    k_right_alt,
    k_right_system,
    k_count
}; // end of Key enum

struct KeyEventImpl {
    Key  key    = k_count;
    bool shift  = false  ;
    bool alt    = false  ;
    bool ctrl   = false  ;
    bool system = false  ;
};

constexpr const char k_no_char_map = -1;

} // end of keys namespace -> into ::asgl

using Key = keys::Key;

namespace mouse {

constexpr const int k_mouse_no_location = -1;

}  // end of mouse namespace -> into ::asgl

struct MouseLocation {
    int x = mouse::k_mouse_no_location;
    int y = mouse::k_mouse_no_location;
};

namespace mouse {
    enum Button {
        k_left,
        k_middle,
        k_right,
        k_count
    };

    struct MouseEventImpl : public MouseLocation {
        Button button = k_count;
    };

} // end of mouse namespace -> into ::asgl

struct KeyTyped {
    char32_t code;
};

using MouseButton = mouse::Button;

struct MousePress : public mouse::MouseEventImpl {
    MousePress() = default;
    explicit MousePress(const mouse::MouseEventImpl & o): mouse::MouseEventImpl(o) {}
};

struct MouseRelease : public mouse::MouseEventImpl {
    MouseRelease() = default;
    explicit MouseRelease(const mouse::MouseEventImpl & o): mouse::MouseEventImpl(o) {}
};

struct MouseMove : public MouseLocation {};

struct KeyPress : public keys::KeyEventImpl {
    KeyPress() = default;
    explicit KeyPress(const keys::KeyEventImpl & o): keys ::KeyEventImpl(o) {}
};

struct KeyRelease : public keys::KeyEventImpl {
    KeyRelease() = default;
    explicit KeyRelease(const keys::KeyEventImpl & o): keys ::KeyEventImpl(o) {}
};

using Event = MultiType<
    MousePress, MouseRelease, MouseMove, KeyPress, KeyRelease, KeyTyped
>;

constexpr const int k_mouse_press_id   = Event::GetTypeId<MousePress  >::k_value;
constexpr const int k_mouse_move_id    = Event::GetTypeId<MouseMove   >::k_value;
constexpr const int k_mouse_release_id = Event::GetTypeId<MouseRelease>::k_value;
constexpr const int k_key_press_id     = Event::GetTypeId<KeyPress    >::k_value;
constexpr const int k_key_release_id   = Event::GetTypeId<KeyRelease  >::k_value;
constexpr const int k_key_typed_id     = Event::GetTypeId<KeyTyped    >::k_value;

Key collapse_numerics(Key);
Key collapse_modifiers(Key);
inline Key collapse_all(Key k) { return collapse_numerics(collapse_modifiers(k)); }
char to_char(const keys::KeyEventImpl &);

inline sf::Vector2i to_vector(const asgl::MouseLocation & loc)
    { return sf::Vector2i(loc.x, loc.y); }

std::string to_string(const Event &);

} // end of asgl namespace
