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

#include <asgl/Defs.hpp>

#include <common/MultiType.hpp>

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

struct MouseLocation : public Vector {
    MouseLocation():
        Vector(mouse::k_mouse_no_location, mouse::k_mouse_no_location) {}
    MouseLocation(int x_, int y_): Vector(x_, y_) {}
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

struct KeyTyped final {
    char32_t code;
};

using MouseButton = mouse::Button;

struct MousePress final : public mouse::MouseEventImpl {
    MousePress() = default;
    explicit MousePress(const mouse::MouseEventImpl & o): mouse::MouseEventImpl(o) {}
};

struct MouseRelease final : public mouse::MouseEventImpl {
    MouseRelease() = default;
    explicit MouseRelease(const mouse::MouseEventImpl & o): mouse::MouseEventImpl(o) {}
};

struct MouseMove final : public MouseLocation {};

struct KeyPress final : public keys::KeyEventImpl {
    KeyPress() = default;
    explicit KeyPress(const keys::KeyEventImpl & o): keys ::KeyEventImpl(o) {}
};

struct KeyRelease final : public keys::KeyEventImpl {
    KeyRelease() = default;
    explicit KeyRelease(const keys::KeyEventImpl & o): keys ::KeyEventImpl(o) {}
};

namespace general_motion {

enum GeneralMotionEnum {
    k_advance_focus,
    k_regress_focus,
    k_motion_focus_up,
    k_motion_focus_down,
    k_motion_focus_right,
    k_motion_focus_left,
    k_accept,
    k_cancel,
    k_general_motion_count
};

} // end of general_motion namespace -> into ::asgl

using GeneralMotion = general_motion::GeneralMotionEnum;

namespace gamepad {

constexpr const int k_no_id     = -1;
constexpr const int k_no_button = -1;

struct Button {
    int gamepad_id = k_no_id;
    int button     = k_no_button;
};

struct Axis {
    int gamepad_id = k_no_id;
    int axis_id = k_no_id;
    // [-1 1]
    double position = 0.;
};

} // end of gamepad namespace -> into ::asgl

using AxisMove = gamepad::Axis;
struct ButtonPress final : public gamepad::Button {
    ButtonPress() = default;
    ButtonPress(const gamepad::Button & o): gamepad::Button(o) {}
};

struct ButtonRelease final : public gamepad::Button {
    ButtonRelease() = default;
    ButtonRelease(const gamepad::Button & o): gamepad::Button(o) {}
};

using Event = cul::MultiType<
    MousePress, MouseRelease, MouseMove, KeyPress, KeyRelease, KeyTyped,
    AxisMove, ButtonPress, ButtonRelease,
    GeneralMotion
>;

template <typename T>
constexpr const int k_event_id_of = Event::GetTypeId<T>::k_value;

Key collapse_numerics(Key);

Key collapse_modifiers(Key);

inline Key collapse_all(Key k) { return collapse_numerics(collapse_modifiers(k)); }

char to_char(const keys::KeyEventImpl &);

std::string to_string(const Event &);

} // end of asgl namespace
