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

#include <asgl/Event.hpp>

namespace asgl {

Key collapse_numerics(Key k) {
    using namespace keys;
    switch (k) {
    case k_pad_zero : return k_zero ;
    case k_pad_one  : return k_one  ;
    case k_pad_two  : return k_two  ;
    case k_pad_three: return k_three;
    case k_pad_four : return k_four ;
    case k_pad_five : return k_five ;
    case k_pad_six  : return k_six  ;
    case k_pad_seven: return k_seven;
    case k_pad_eight: return k_eight;
    case k_pad_nine : return k_nine ;
    default         : return k;
    }
}

Key collapse_modifiers(Key k) {
    using namespace keys;
    switch (k) {
    case k_left_control : return k_control;
    case k_left_shift   : return k_shift;
    case k_left_alt     : return k_alt;
    case k_left_system  : return k_system;
    case k_right_control: return k_control;
    case k_right_shift  : return k_shift;
    case k_right_alt    : return k_alt;
    case k_right_system : return k_system;
    default             : return k;
    }
}

char to_char(const keys::KeyEventImpl & key_event) {
    using namespace keys;
    bool shift = key_event.shift;
    // what about localization? idk tbh
    switch (key_event.key) {
    case k_tilde: return shift ? '~' : '`';
    case k_zero : return shift ? ')' : '0';
    case k_one  : return shift ? '!' : '1';
    case k_two  : return shift ? '@' : '2';
    case k_three: return shift ? '#' : '3';
    case k_four : return shift ? '$' : '4';
    case k_five : return shift ? '%' : '5';
    case k_six  : return shift ? '^' : '6';
    case k_seven: return shift ? '&' : '7';
    case k_eight: return shift ? '*' : '8';
    case k_nine : return shift ? '(' : '9';
    case k_pad_zero : return '0';
    case k_pad_one  : return '1';
    case k_pad_two  : return '2';
    case k_pad_three: return '3';
    case k_pad_four : return '4';
    case k_pad_five : return '5';
    case k_pad_six  : return '6';
    case k_pad_seven: return '7';
    case k_pad_eight: return '8';
    case k_pad_nine : return '9';
    case k_dash : return shift ? '_' : '@';
    case k_equal: return shift ? '+' : '@';
    case k_a: return shift ? 'A' : 'a';
    case k_b: return shift ? 'B' : 'b';
    case k_c: return shift ? 'C' : 'c';
    case k_d: return shift ? 'D' : 'd';
    case k_e: return shift ? 'E' : 'e';
    case k_f: return shift ? 'F' : 'f';
    case k_g: return shift ? 'G' : 'g';
    case k_h: return shift ? 'H' : 'h';
    case k_i: return shift ? 'I' : 'i';
    case k_j: return shift ? 'J' : 'j';
    case k_k: return shift ? 'K' : 'k';
    case k_l: return shift ? 'L' : 'l';
    case k_m: return shift ? 'M' : 'm';
    case k_n: return shift ? 'N' : 'n';
    case k_o: return shift ? 'O' : 'o';
    case k_p: return shift ? 'P' : 'p';
    case k_q: return shift ? 'Q' : 'q';
    case k_r: return shift ? 'R' : 'r';
    case k_s: return shift ? 'S' : 's';
    case k_t: return shift ? 'T' : 't';
    case k_u: return shift ? 'U' : 'u';
    case k_v: return shift ? 'V' : 'v';
    case k_w: return shift ? 'W' : 'w';
    case k_x: return shift ? 'X' : 'x';
    case k_y: return shift ? 'Y' : 'y';
    case k_z: return shift ? 'Z' : 'z';
    case k_left_bracket : return shift ? '{'  : '[' ;
    case k_right_bracket: return shift ? '}'  : ']' ;
    case k_semicolon    : return shift ? ':'  : ';' ;
    case k_comma        : return shift ? '<'  : ',' ;
    case k_period       : return shift ? '.'  : '>' ;
    case k_quote        : return shift ? '"'  : '\'';
    case k_foreslash    : return shift ? '/'  : '?' ;
    case k_backslash    : return shift ? '\\' : '|' ;
    case k_space: return ' ';
    case k_enter: return '\n';
    case k_tab  : return '\t';
    case k_plus          : return '+';
    case k_minus         : return '-';
    case k_multiplication: return '*';
    case k_division      : return '/';
    default: return k_no_char_map;
    }
}

std::string to_string(const Event & event) {
    static auto ml_to_string = [](const MouseLocation & ev)
        { return "( " + std::to_string(ev.x) + ", " + std::to_string(ev.y) + ")"; };
    static auto me_to_string = [](const mouse::MouseEventImpl & ev) {
        using namespace mouse;
        std::string rv;
        switch (ev.button) {
        case k_left  : rv += "left "   ; break;
        case k_middle: rv += "middle " ; break;
        case k_right : rv += "right "  ; break;
        default      : rv += "unknown "; break;
        }
        return rv + ml_to_string(ev);
    };
    static auto ke_to_string = [](const keys::KeyEventImpl & ev)
        { return std::string(1, to_char(ev)); };
    switch (event.type_id()) {
    case k_event_id_of<MousePress>:
        return "Mouse button pressed " + me_to_string(event.as<MousePress>());
    case k_event_id_of<MouseMove>:
        return "Mouse moved " + ml_to_string(event.as<MouseMove>());
    case k_event_id_of<MouseRelease>:
        return "Mouse button released " + me_to_string(event.as<MouseRelease>());
    case k_event_id_of<KeyPress>:
        return "Key pressed " + ke_to_string(event.as<KeyPress>());
    case k_event_id_of<KeyRelease>:
        return "Key release " + ke_to_string(event.as<KeyRelease>());
    case k_event_id_of<KeyTyped>: {
        std::string rv = "Key typed ";
        auto code = event.as<KeyTyped>().code;
        if (code >= 0x20 && code < 0x7F) {
            rv += "\"" + std::string(1, char(code)) + "\"";
        } else {
            rv += "code " + std::to_string(code);
        }
        return rv;
    }
    default: return "<unidentified event>";
    }
}

} // end of asgl namespace
