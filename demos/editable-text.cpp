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

#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/EditableText.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/sfml/SfmlEngine.hpp>
#if 0
#include <asgl/SelectionMenu.hpp>
#endif
#include <SFML/Graphics/RenderWindow.hpp>

#include <common/StringUtil.hpp>
#include <common/TestSuite.hpp>

#include <thread>
#include <chrono>
#include <iostream>

#include <cassert>

using namespace asgl;

namespace {

constexpr const bool k_log_key_events = true;

class EditableTextFrame final : public Frame {
public:
    EditableTextFrame(): m_request_close_flag(false) {}

    bool requesting_to_close() const { return m_request_close_flag; }
    void setup_frame();
private:
#   if 0
    SelectionMenu m_menu;
#   endif
    TextArea m_option_text;

    TextArea m_text_area;
    EditableText m_editable_text;

    EditableText m_num_only_et;
    TextArea m_num_only_notice;

    TextButton m_exit_button;

    TextArea m_phone_num_example;
    EditableText m_phone_number_et;

    bool m_request_close_flag;
};

void test_editable_text_utils();

} // end of <anonymous> namespace

int main() {
    test_editable_text_utils();

    SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");

    EditableTextFrame dialog;
    dialog.setup_frame();
    engine.stylize(dialog);

    sf::RenderWindow window(
        sf::VideoMode(unsigned(dialog.width()), unsigned(dialog.height())), 
        "Window Title");
    engine.assign_target_and_states(window, sf::RenderStates::Default);
    window.setFramerateLimit(20);
    bool has_events = true;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            has_events = true;
            auto convevent = SfmlFlatEngine::convert(event);
            dialog.process_event(convevent);
            if (k_log_key_events
                && (convevent.is_type<KeyPress>() || convevent.is_type<KeyRelease>()
                    || convevent.is_type<KeyTyped>()))
            {
                SfmlFlatEngine::convert(event);
                static int count = 0;
                std::cout << "event #" << count++ << " "
                          << to_string(convevent) << std::endl;
            }
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (dialog.requesting_to_close())
            window.close();
        if (has_events) {
            window.clear();
            dialog.check_for_geometry_updates();
            dialog.draw(engine);
            window.display();
            has_events = false;
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(16667));
        }
    }
    return 0;
}

namespace {

using UString          = asgl::Text::UString;
using UStringConstIter = asgl::Text::UStringConstIter;
using UChar            = UString::value_type;

template <typename T>
UString shorten(UString &&, T error);

bool is_valid_us_phone_number(const UString &);

UString format_us_phone_number(const UString &, UString && = UString());

void EditableTextFrame::setup_frame() {
    m_option_text.set_string(U"none selected");
#   if 0
    m_menu.add_options({
        U"Option 1",
        U"Option 2",
        U"Option 3"
    });
    m_menu.set_response_function([this](std::size_t, const UString & ustr) {
        m_option_text.set_string(ustr);
    });
#   endif
    m_text_area.set_string(U"Editable Text Box:");
    m_editable_text.set_text_width(150);

    m_num_only_notice.set_string(U"Note: this text box only accepts numbers.");
    m_num_only_et.set_text_width(150);
    m_num_only_et.set_entered_string(shorten(U"0.50000", 0.05));
    m_num_only_et.set_check_string_event([](const UString & ustr, UString & display_text) {
        int num = 0;
        bool rv;
        if ((rv = string_to_number(ustr, num) ))
            display_text = ustr;
        return rv;
    });

    m_exit_button.set_press_event([this]() { m_request_close_flag = true; });
    m_exit_button.set_string(U"Close Application");

    m_phone_num_example.set_string(U"Example Phone Number thing:");

    m_phone_number_et.set_empty_string(U"1 (800) 555 - 5555");
    m_phone_number_et.set_check_string_event([]
        (const UString & entered_string, UString & display)
    {
        if (!EditableText::default_check_string_event(entered_string, display))
            return false;
        if (!is_valid_us_phone_number(entered_string)) return false;
        display = format_us_phone_number(entered_string, std::move(display));
        return true;
    });

    begin_adding_widgets()
        ./*add(m_menu).*/add(m_option_text).add_line_seperator()
        .add(m_text_area).add(m_editable_text).add_line_seperator()
        .add(m_num_only_et).add_line_seperator()
        .add(m_num_only_notice).add_line_seperator()
        .add(m_phone_num_example).add(m_phone_number_et).add_line_seperator()
        .add(m_exit_button);
}

void test_editable_text_utils() {
    ts::TestSuite suite;
    suite.start_series("editable text is_display_string_ok");
    suite.test([]() {
        return ts::test(EditableText::is_display_string_ok(
            U"abcdefghi", U"abcdefghi"));
    });
    suite.test([]() {
        return ts::test(EditableText::is_display_string_ok(
            U" abcdefghi", U"abcdefghi"));
    });
    suite.test([]() {
        return ts::test(EditableText::is_display_string_ok(
            U" a b c d e f g h i", U"abcdefghi"));
    });
    suite.test([]() {
        return ts::test(EditableText::is_display_string_ok(
            U"cat ", U"cat"));
    });
    suite.test([]() {
        return ts::test(!EditableText::is_display_string_ok(
            U"cat ", U"c-at"));
    });
    suite.test([]() {
        return ts::test(EditableText::is_display_string_ok(
            U" 1 (800) 478-", U"1800478"));
    });
    suite.start_series("editable text find_display_position");
    suite.test([]() {
        //                   0
        //                  01
        UString display = U" 1 (800) 478-";
        UString entered = U"1800478";
        auto itr = EditableText::find_display_position(display, entered, 0);
        return ts::test(display.begin() + 1 == itr);
    });
    suite.test([]() {
        //                   0  123
        //                  0123456
        UString display = U" 1 (800) 478-";
        UString entered = U"1800478";
        auto itr = EditableText::find_display_position(display, entered, 3);
        return ts::test(display.begin() + 6 == itr);
    });
}

// ----------------------------------------------------------------------------

template <typename T>
UString shorten(UString && ustr, T error) {
    T last_out = T(0);
    if (!string_to_number(ustr, last_out)) {
        throw std::invalid_argument("base input is not numeric");
    }
    T out = last_out;
    auto end = ustr.end();
    while (true) {
        auto mid = ustr.begin() + ((end - ustr.begin()) / 2);
        string_to_number(ustr.begin(), mid, out);
        if (magnitude(out - last_out) > error) {
            ustr.erase(end, ustr.end());
            return std::move(ustr);
        }
        end = mid;
    }
}

bool is_valid_us_phone_number(const UString & entered_string) {
    if (entered_string.size() > 11) return false;
    auto beg = entered_string.begin();
    auto end = entered_string.end();
    if (end - beg == 11) {
        if (*beg++ != U'1') return false;
    }
    if (std::any_of(beg, end, [](UChar c) { return c < U'0' || c > U'9'; })) {
        return false;
    }
    return true;
}

UString format_us_phone_number(const UString & source, UString && display) {
    display.clear();
    using Iter = UStringConstIter;
    auto append_at_most = [&display](Iter beg, Iter end, int n) {
        if (end - beg > n) end = beg + n;
        display.insert(display.end(), beg, end);
        return end;
    };
    auto beg = source.begin();
    auto end = source.end();
    if (end == beg) {
        // do nothing
    } else if (end - beg <= 7) {
        beg = append_at_most(beg, end, 3);
        display += U" - ";
        append_at_most(beg, end, 4);
    } else if (end - beg <= 10) {
        display += U"(";
        beg = append_at_most(beg, end, 3);
        display += U") ";
        beg = append_at_most(beg, end, 3);
        display += U" - ";
        beg = append_at_most(beg, end, 4);
    } else {
        assert(end - beg == 11);
        beg = append_at_most(beg, end, 1);
        display += U"(";
        beg = append_at_most(beg, end, 3);
        display += U") ";
        beg = append_at_most(beg, end, 3);
        display += U" - ";
        beg = append_at_most(beg, end, 4);
    }
    return std::move(display);
}

} // end of <anonymous> namespace
