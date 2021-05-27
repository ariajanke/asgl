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

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <thread>

#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>
#include <asgl/OptionsSlider.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <common/StringUtil.hpp>
#include <common/TestSuite.hpp>

#include <limits>
#include <random>   // debug
#include <iostream> // debug

#include <cstring>
#include <cassert>

namespace {

using asgl::BareFrame;
using asgl::TextArea;
using asgl::OptionsSlider;
using asgl::TextButton;
using asgl::Event;
using asgl::UString;
using UChar = decltype(U'l');
using cul::for_split;

bool is_space(UChar c) { return c == ' '; }

template <typename CharType>
const CharType * end_of_str(const CharType * s) {
    if (*s) while (*++s) {}
    return s;
}

// source: https://lipsum.com/
constexpr const auto * const k_ipsum = U""
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur risus "
    "lorem, suscipit nec dolor ac, aliquet lobortis libero. Cras cursus "
    "molestie elementum. Nulla hendrerit est in cursus interdum. Mauris "
    "iaculis lacus erat, eu imperdiet risus ultrices maximus. In hac habitasse "
    "platea dictumst. Mauris volutpat risus sed neque feugiat, eu tincidunt "
    "magna convallis. Cras nec lorem arcu. Interdum et malesuada fames ac "
    "ante ipsum primis in faucibus. Morbi mi ante, dapibus eget lobortis ut, "
    "aliquet at dolor. Proin vestibulum ante ut risus mollis efficitur. "
    "Vivamus in justo lobortis, luctus risus vitae, laoreet nibh. Vivamus eget "
    "iaculis lectus, a ullamcorper risus.";

const int k_word_count = []() {
    int rv = 1;
    for_split<is_space>(k_ipsum, end_of_str(k_ipsum),
                        [&rv](const UChar *, const UChar *){ ++rv; });
    return rv;
}();

using IntLims = std::numeric_limits<int>;

class ParamFrame : public asgl::Frame {
public:
    virtual void on_update() = 0;
};

class TesterBase {
public:
    static constexpr const int k_min_size  = 100;
    static constexpr const int k_max_size  = 450;
    static constexpr const int k_size_step =  15;

    static constexpr const int k_min_view_x =   0;
    static constexpr const int k_max_view_x = 350;
    static constexpr const int k_view_step  =  10;

    static constexpr const int k_min_words =   1;
    static constexpr const int k_word_step =   2;

    struct Params {
        int word_count  = k_min_words;
        int view_width  = IntLims::max();
        int view_height = IntLims::max();
        int view_x      = 0;
        int view_y      = 0;
        int limit_x     = IntLims::max();
    };

    virtual void on_parameters_update() = 0;

    const Params & parameters() const { return m_params; }

    void set_parameters(const Params & params) { m_params = params; }

    static asgl::Rectangle as_viewport_rectangle(const Params & params);

private:
    Params m_params;
};

class SizeParamsFrame final : public ParamFrame {
public:
    void setup(TesterBase * base_ptr);

private:
    void on_update() override;

    static int get_options_value(const OptionsSlider & slider, int min, int step);

    TextArea m_view_width_lbl;
    TextArea m_view_height_lbl;
    TextArea m_x_limit_lbl;

    OptionsSlider m_view_width;
    OptionsSlider m_view_height;
    OptionsSlider m_x_limit;

    TesterBase * m_tester_base = nullptr;
};

class LocationsParamsFrame final : public ParamFrame {
public:
    void setup(TesterBase * base_ptr);

private:
    void on_update() override;

    static int get_options_value(const OptionsSlider & slider, int min, int step)
        { return int(slider.selected_option_index())*step + min; }

    TextArea m_view_x_lbl;
    TextArea m_view_y_lbl;

    OptionsSlider m_view_x;
    OptionsSlider m_view_y;

    TesterBase * m_tester_base = nullptr;
};

class TestFrame final : public TesterBase, public asgl::Frame {
public:
    // I want to test restricted width
    void setup();
    bool requesting_exit() const { return m_req_exit; }

    void process_event(const Event &) override;

private:
    void first_setup();

    void on_parameters_update() override;

    TextArea m_test_text_area;

    TextArea m_amount_of_text_notice;

    OptionsSlider m_amount_of_text;

    LocationsParamsFrame m_locs_frame;
    SizeParamsFrame      m_size_frame;

    TextButton m_force_geometry_update;
    TextButton m_exit;

    bool m_req_exit = false;
    bool m_redo_setup = false;
    bool m_first_setup = true;
};

void run_tests();

void run_engine_tests(asgl::SfmlFlatEngine &);

} // end of <anonymous> namespace

int main() {
    run_tests();

    asgl::SfmlFlatEngine engine;

    // my objects are getting a tad large (5100+ bytes)
    auto frame_ptr = std::make_unique<TestFrame>();
    TestFrame & frame = *frame_ptr;

    sf::RenderWindow win;
    engine.assign_target_and_states(win, sf::RenderStates::Default);
    engine.load_global_font("font.ttf");

    //run_engine_tests(engine);

    frame.setup();
    engine.stylize(frame);
    frame.check_for_geometry_updates();
    {
    auto old_width = frame.width();
    {
    asgl::Widget & asw = frame;
    asw.update_size();
    }
    assert(old_width == frame.width());
    }

    win.create(sf::VideoMode(unsigned(frame.width()) + 200, unsigned(frame.height()) + 200), "TextArea Test App");
    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            frame.process_event(asgl::SfmlFlatEngine::convert(event));
            if (event.type == sf::Event::Closed || frame.requesting_exit()) {
                win.close();
            }
        }
        win.clear();
        int old_width = frame.width();
        frame.check_for_geometry_updates();
        if (old_width < frame.width()) {
            std::cout << "Frame width increase " << (frame.width() - old_width) << std::endl;
        }
        frame.draw(engine);
#       if 0
        using IntDistri = std::uniform_int_distribution<int>;
        std::default_random_engine rng { 0x1201471 };
        frame.iterate_children_const_f([&win, &rng](const asgl::Widget & widget) {
            auto rnd_u8 = [](std::default_random_engine & rng)
                { return IntDistri(0, 255)(rng); };
            DrawRectangle rect(float(widget.location().x), float(widget.location().y),
                               float(widget.width()), float(widget.height()),
                               sf::Color(rnd_u8(rng), rnd_u8(rng), rnd_u8(rng), 100));
            win.draw(rect);
        });
#       endif
        win.display();
        std::this_thread::sleep_for(std::chrono::microseconds(16667));
    }
}

namespace {

std::vector<UString> make_range(int min, int max, int step);
std::vector<UString> make_range(std::vector<UString> &&, int min, int max, int step);

template <typename CharType>
const CharType * after_n_words(int n, const CharType *);

// --- TesterBase ---

/* static */ asgl::Rectangle TesterBase::as_viewport_rectangle(const Params & params) {
    auto viewport = asgl::Text().viewport();
    viewport.left = params.view_x;
    viewport.top  = params.view_y;
    if (params.view_width != IntLims::max()) {
        viewport.width = params.view_width;
    }
    if (params.view_height != IntLims::max()) {
        viewport.height = params.view_height;
    }
    return viewport;
}

// --- SizeParamsFrame ---

void SizeParamsFrame::setup(TesterBase * base_ptr) {
    using Tb = TesterBase;
    m_tester_base = base_ptr;
    for (auto * slider : { &m_view_width, &m_view_height, &m_x_limit }) {

        auto options = make_range(Tb::k_min_size, Tb::k_max_size, Tb::k_size_step);
        options.insert(options.begin(), U"Automatic");
        slider->set_options(std::move(options));
        slider->set_option_change_event([this](){ this->on_update(); });
    }

    m_x_limit_lbl    .set_string(U"Limit X"    );
    m_view_width_lbl .set_string(U"View Width" );
    m_view_height_lbl.set_string(U"View Height");

    begin_adding_widgets()
        .add(m_x_limit_lbl).add_horizontal_spacer().add(m_x_limit).add_line_seperator()
        .add(m_view_width_lbl).add_horizontal_spacer().add(m_view_width).add_line_seperator()
        .add(m_view_height_lbl).add_horizontal_spacer().add(m_view_height);
}

/* private */ void SizeParamsFrame::on_update() {
    if (!m_tester_base)
        { throw std::runtime_error("SizeParamsFrame::on_update: tester pointer not set."); }

    auto params = m_tester_base->parameters();
    using Tb = TesterBase;
    params.view_width  = get_options_value(m_view_width , Tb::k_min_size, Tb::k_size_step);
    params.view_height = get_options_value(m_view_height, Tb::k_min_size, Tb::k_size_step);
    params.limit_x     = get_options_value(m_x_limit    , Tb::k_min_size, Tb::k_size_step);
    m_tester_base->set_parameters(params);
    m_tester_base->on_parameters_update();
}

/* private static */ int SizeParamsFrame::get_options_value
    (const OptionsSlider & slider, int min, int step)
{
    auto idx = slider.selected_option_index();
    if (idx == 0) return IntLims::max();
    --idx;
    return int(idx)*step + min;
}

// --- LocationsParamsFrame ---

void LocationsParamsFrame::setup(TesterBase * base_ptr) {
    using Tb = TesterBase;
    m_tester_base = base_ptr;
    for (auto * slider : { &m_view_x, &m_view_y }) {
        slider->set_options(make_range(Tb::k_min_view_x, Tb::k_max_view_x, Tb::k_view_step));
        slider->set_option_change_event([this](){ this->on_update(); });
    }

    m_view_x_lbl.set_string(U"View X");
    m_view_y_lbl.set_string(U"View Y");

    begin_adding_widgets()
        .add(m_view_x_lbl).add_horizontal_spacer().add(m_view_x).add_line_seperator()
        .add(m_view_y_lbl).add_horizontal_spacer().add(m_view_y);
}

/* private */ void LocationsParamsFrame::on_update() {
    if (!m_tester_base)
        { throw std::runtime_error("LocationsParamsFrame::on_update: tester pointer not set."); }
    auto params = m_tester_base->parameters();
    using Tb = TesterBase;
    params.view_x = get_options_value(m_view_x, Tb::k_min_view_x, Tb::k_size_step);
    params.view_y = get_options_value(m_view_y, Tb::k_min_view_x, Tb::k_size_step);
    m_tester_base->set_parameters(params);
    m_tester_base->on_parameters_update();
}

// --- TestFrame --

void TestFrame::setup() {
    if (m_first_setup) first_setup();

    set_title(U"Text Area Tester App");

    int num_of_words = k_min_words + int(m_amount_of_text.selected_option_index())*k_word_step;
    m_test_text_area.set_string(UString(k_ipsum, after_n_words(num_of_words, k_ipsum)));

    begin_adding_widgets()
        .add(m_amount_of_text_notice).add(m_amount_of_text).add_horizontal_spacer().add_line_seperator()
#       if 1
        .add(m_locs_frame).add(m_size_frame).add_line_seperator()
#       endif
        .add(m_exit).add(m_force_geometry_update).add_line_seperator()
        .add_horizontal_spacer().add(m_test_text_area).add_horizontal_spacer();
}

void TestFrame::process_event(const Event & event) {
    BareFrame::process_event(event);
    if (m_redo_setup) {
        setup();
        m_redo_setup = false;
    }
}

/* private */ void TestFrame::first_setup() {
    m_first_setup = false;

    m_amount_of_text_notice.set_string(U"Number of Words.");
    m_amount_of_text.set_options(make_range(k_min_words, k_word_count, k_word_step));
    m_amount_of_text.set_option_change_event([this](){ m_redo_setup = true; });

    m_exit.set_string(U"Exit App");
    m_exit.set_press_event([this]() { m_req_exit = true; });

    m_locs_frame.setup(this);
    m_size_frame.setup(this);

    m_force_geometry_update.set_string(U"Force Geometry Update");
    m_force_geometry_update.set_press_event([this]() { flag_needs_whole_family_geometry_update(); });
}

void TestFrame::on_parameters_update() {
    auto params = parameters();
    m_test_text_area.set_limiting_line(params.limit_x);
    m_test_text_area.set_viewport(as_viewport_rectangle(params));

    m_redo_setup = true;
}

// --- free fn ---

void run_tests() {
    using namespace cul;
    ts::TestSuite suite;
    suite.start_series("end_of_str");
    suite.test([]() {
        auto s = "hello";
        return ts::test(s + strlen(s) == end_of_str(s));
    });
    suite.test([]() {
        auto s = "";
        return ts::test(s + strlen(s) == end_of_str(s));
    });
    suite.test([]() {
        auto s = "1";
        return ts::test(s + strlen(s) == end_of_str(s));
    });

    static auto do_after_n_words_test = []
        (const char * test_str, int test_num, const char * correct)
    {
        auto res = after_n_words(test_num, test_str);
        return std::equal(res, end_of_str(res), correct);
    };
    suite.start_series("after_n_words");
    suite.test([]() {
        return ts::test(do_after_n_words_test("a b c", 1, "b c"));
    });
    suite.test([]() {
        return ts::test(do_after_n_words_test("a b c", 0, "a b c"));
    });
    suite.test([]() {
        return ts::test(do_after_n_words_test("why hello there", 2, "there"));
    });
    suite.test([]() {
        return ts::test(do_after_n_words_test("a b c d e", 5, ""));
    });
}

void run_engine_tests(asgl::SfmlFlatEngine & engine) {
    {
    TextArea test_text;
    test_text.set_string(U"Hello World!");
    engine.stylize(test_text);
    test_text.update_size();
    test_text.set_location(0, 0);
    int width = test_text.width();
    assert(width != 0);
    for (int x = 1; x != 100; ++x) {
        test_text.set_location(x, 0);
        assert(width == test_text.width());
    }
    }
    {
    asgl::FrameBorder border;
    border.request_size(300, 300);
    //border.update_geometry();
    border.request_size(300, 300);
    //border.update_geometry();
    int i = border.width();
    }
}

// ----------------------------------------------------------------------------

std::vector<UString> make_range(int min, int max, int step) {
    return make_range(std::vector<UString>(), min, max, step);
}

std::vector<UString> make_range
    (std::vector<UString> && init_cont, int min, int max, int step)
{
    init_cont.reserve(init_cont.size() + (max - min + 1) / step);
    for (int i = min; i <= max; i += step) {
        auto t = std::to_string(i);
        init_cont.emplace_back();
        init_cont.back().append(t.begin(), t.end());
    }
    return std::move(init_cont);
}

template <typename CharType>
const CharType * after_n_words(int n, const CharType * beg) {
    using namespace cul::fc_signal;
    const int org_n = n;
    const CharType * rv = beg;
    for_split<is_space>(beg, end_of_str(beg),
        [&n, &rv](const CharType * beg, const CharType *)
    {
        rv = beg;
        if (n) {
            --n;
            return k_continue;
        } else {
            return k_break;
        }
    });
    if (n > 1) {
        throw std::invalid_argument("after_n_words: string does not contain " + std::to_string(org_n) + " words.");
    }
    if (n == 1) return end_of_str(beg);

    return rv;
}

} // end of <anonymous> namespace
