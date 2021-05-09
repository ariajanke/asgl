#include <asgl/Event.hpp>

#include <SFML/Window/Event.hpp>

namespace {

using asgl::keys::KeyEventImpl;
using asgl::mouse::MouseEventImpl;
using asgl::MouseMove;

KeyEventImpl convert(const sf::Event::KeyEvent &);
MouseEventImpl convert(const sf::Event::MouseButtonEvent &);
MouseMove convert(const sf::Event::MouseMoveEvent &);

} // end of <anonymous> namespace

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

Event convert(const sf::Event & sfevent) {
    switch (sfevent.type) {
    case sf::Event::KeyPressed:
        return Event(KeyPress(::convert(sfevent.key)));
    case sf::Event::KeyReleased:
        return Event(KeyRelease(::convert(sfevent.key)));
    case sf::Event::MouseButtonPressed:
        return Event(MousePress(::convert(sfevent.mouseButton)));
    case sf::Event::MouseButtonReleased:
        return Event(MouseRelease(::convert(sfevent.mouseButton)));
    case sf::Event::MouseMoved:
        return Event(MouseMove(::convert(sfevent.mouseMove)));
    case sf::Event::TextEntered:
        return Event(KeyTyped{sfevent.text.unicode});
    default: return Event();
    }
}

} // end of asgl namespace

namespace {

using asgl::Key;
using asgl::MouseButton;

Key convert(sf::Keyboard::Key);
MouseButton convert(sf::Mouse::Button);

KeyEventImpl convert(const sf::Event::KeyEvent & ke) {
    KeyEventImpl kei;
    kei.alt    = ke.alt;
    kei.key    = convert(ke.code);
    kei.ctrl   = ke.control;
    kei.shift  = ke.shift;
    kei.system = ke.system;
    return kei;
}

MouseEventImpl convert(const sf::Event::MouseButtonEvent & mbe) {
    MouseEventImpl mei;
    mei.x = mbe.x;
    mei.y = mbe.y;
    mei.button = convert(mbe.button);
    return mei;
}

// ----------------------------------------------------------------------------

Key convert(sf::Keyboard::Key k) {
    using namespace asgl::keys;
    using Ky = sf::Keyboard;
    switch (k) {
    case Ky::Unknown: return k_count;
    case Ky::A: return k_a; case Ky::B: return k_b; case Ky::C: return k_c;
    case Ky::D: return k_d; case Ky::E: return k_e; case Ky::F: return k_f;
    case Ky::G: return k_g; case Ky::H: return k_h; case Ky::I: return k_i;
    case Ky::J: return k_j; case Ky::K: return k_k; case Ky::L: return k_l;
    case Ky::M: return k_m; case Ky::N: return k_n; case Ky::O: return k_o;
    case Ky::P: return k_p; case Ky::Q: return k_q; case Ky::R: return k_r;
    case Ky::S: return k_s; case Ky::T: return k_t; case Ky::U: return k_u;
    case Ky::V: return k_v; case Ky::W: return k_w; case Ky::X: return k_x;
    case Ky::Y: return k_y; case Ky::Z: return k_z;
    case Ky::Num0: return k_zero ; case Ky::Num1: return k_one  ;
    case Ky::Num2: return k_two  ; case Ky::Num3: return k_three;
    case Ky::Num4: return k_four ; case Ky::Num5: return k_five ;
    case Ky::Num6: return k_six  ; case Ky::Num7: return k_seven;
    case Ky::Num8: return k_eight; case Ky::Num9: return k_nine ;
    case Ky::Escape   : return k_escape        ;
    case Ky::LControl : return k_left_control  ;
    case Ky::LShift   : return k_left_shift    ;
    case Ky::LAlt     : return k_left_alt      ;
    case Ky::LSystem  : return k_left_system   ;
    case Ky::RControl : return k_right_control ;
    case Ky::RShift   : return k_right_shift   ;
    case Ky::RAlt     : return k_right_alt     ;
    case Ky::RSystem  : return k_right_system  ;
    case Ky::Menu     : return k_menu          ;
    case Ky::LBracket : return k_left_bracket  ;
    case Ky::RBracket : return k_right_bracket ;
    case Ky::SemiColon: return k_semicolon     ;
    case Ky::Comma    : return k_comma         ;
    case Ky::Period   : return k_period        ;
    case Ky::Quote    : return k_quote         ;
    case Ky::Slash    : return k_foreslash     ;
    case Ky::BackSlash: return k_backslash     ;
    case Ky::Tilde    : return k_tilde         ;
    case Ky::Equal    : return k_equal         ;
    case Ky::Dash     : return k_dash          ;
    case Ky::Space    : return k_space         ;
    case Ky::Return   : return k_enter         ;
    case Ky::BackSpace: return k_backslash     ;
    case Ky::Tab      : return k_tab           ;
    case Ky::PageUp   : return k_pageup        ;
    case Ky::PageDown : return k_pagedown      ;
    case Ky::End      : return k_end           ;
    case Ky::Home     : return k_home          ;
    case Ky::Insert   : return k_insert        ;
    case Ky::Delete   : return k_delete        ;
    case Ky::Add      : return k_plus          ;
    case Ky::Subtract : return k_minus         ;
    case Ky::Multiply : return k_multiplication;
    case Ky::Divide   : return k_division      ;
    case Ky::Left     : return k_left ; case Ky::Right    : return k_right;
    case Ky::Up       : return k_up   ; case Ky::Down     : return k_down ;
    case Ky::Numpad0: return k_pad_zero ;
    case Ky::Numpad1: return k_pad_one  ;
    case Ky::Numpad2: return k_pad_two  ;
    case Ky::Numpad3: return k_pad_three;
    case Ky::Numpad4: return k_pad_four ;
    case Ky::Numpad5: return k_pad_five ;
    case Ky::Numpad6: return k_pad_six  ;
    case Ky::Numpad7: return k_pad_seven;
    case Ky::Numpad8: return k_pad_eight;
    case Ky::Numpad9: return k_pad_nine ;
    case Ky::F1 : return k_f1 ; case Ky::F2 : return k_f2 ;
    case Ky::F3 : return k_f3 ; case Ky::F4 : return k_f4 ;
    case Ky::F5 : return k_f5 ; case Ky::F6 : return k_f6 ;
    case Ky::F7 : return k_f7 ; case Ky::F8 : return k_f8 ;
    case Ky::F9 : return k_f9 ; case Ky::F10: return k_f10;
    case Ky::F11: return k_f11; case Ky::F12: return k_f12;
    case Ky::F13: case Ky::F14: case Ky::F15:
        return k_count;
    case Ky::Pause: return k_pause;
    case Ky::KeyCount: return k_count;
    }
    throw std::runtime_error("impossible branch");
}

MouseButton convert(sf::Mouse::Button b) {
    using namespace asgl::mouse;
    using Mb = sf::Mouse;
    switch (b) {
    case Mb::Left  : return k_left  ;
    case Mb::Right : return k_right ;
    case Mb::Middle: return k_middle;
    default        : return k_count ;
    }
}

MouseMove convert(const sf::Event::MouseMoveEvent & mme) {
    MouseMove rv;
    rv.x = mme.x;
    rv.y = mme.y;
    return rv;
}

} // end of <anonymous> namespace
