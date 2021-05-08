#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <thread>
#if 0
#include <ksg/TextArea.hpp>
#include <ksg/Frame.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>
#endif
#include <common/StringUtil.hpp>
#include <common/TestSuite.hpp>

#include <cstring>

namespace {

using UChar = decltype(U'l');

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

class TestFrame final : public ksg::Frame {
public:
    static constexpr const std::size_t k_auto_size_option = 0;

    static constexpr const int k_min_words =   1;
    static constexpr const int k_word_step =   2;
    static constexpr const int k_min_size  =  10;
    static constexpr const int k_max_size  = 220;
    static constexpr const int k_size_step =   5;

    // I want to test restricted width
    void setup();
    bool requesting_exit() const { return m_req_exit; }

    void process_event(const sf::Event &) override;

private:
    void first_setup();

    ksg::TextArea m_test_text_area;

    ksg::TextArea m_amount_of_text_notice;
    ksg::TextArea m_width_notice;
    ksg::TextArea m_height_notice;

    ksg::OptionsSlider m_amount_of_text;
    ksg::OptionsSlider m_width_slider;
    ksg::OptionsSlider m_height_slider;
    ksg::TextButton m_exit;

    ksg::StyleMap m_styles = ksg::styles::construct_system_styles();

    bool m_req_exit = false;
    bool m_redo_setup = false;
    bool m_first_setup = true;
};

void run_tests();

} // end of <anonymous> namespace

int main() {
    run_tests();

    // my objects are getting a tad large (5100+ bytes)
    auto frame_ptr = std::make_unique<TestFrame>();
    TestFrame & frame = *frame_ptr;
    frame.setup();
    //frame.setup();

    sf::RenderWindow win;
    win.create(sf::VideoMode(unsigned(frame.width()) + 200, unsigned(frame.height()) + 200), "TextArea Test App");
    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            frame.process_event(event);
            if (event.type == sf::Event::Closed || frame.requesting_exit()) {
                win.close();
            }
        }
        win.clear();
        win.draw(frame);
        win.display();
        std::this_thread::sleep_for(std::chrono::microseconds(16667));
    }
}

namespace {

using UString = ksg::Text::UString;

std::vector<UString> make_range(int min, int max, int step);
std::vector<UString> make_range(std::vector<UString> &&, int min, int max, int step);

template <typename CharType>
const CharType * after_n_words(int n, const CharType *);

void TestFrame::setup() {
    if (m_first_setup) first_setup();

    float w = ksg::TextArea::k_unassigned_size;
    float h = ksg::TextArea::k_unassigned_size;

    set_title(U"Just some title");

    using std::make_pair;
    for (auto & [wid, dim] : { make_pair(&m_width_slider, &w), make_pair(&m_height_slider, &h) }) {
        auto idx = wid->selected_option_index();
        if (idx == k_auto_size_option) continue;
        *dim = k_min_size + (idx - 1)*k_size_step;
    }
    m_test_text_area.set_size(w, h);

    int num_of_words = k_min_words + int(m_amount_of_text.selected_option_index())*k_word_step;
    m_test_text_area.set_string(UString(k_ipsum, after_n_words(num_of_words, k_ipsum)));

    begin_adding_widgets(m_styles)
        .add(m_amount_of_text_notice).add(m_amount_of_text).add_horizontal_spacer().add_line_seperator()
        .add(m_width_notice).add(m_width_slider).add_horizontal_spacer().add_line_seperator()
        .add(m_height_notice).add(m_height_slider).add_horizontal_spacer().add_line_seperator()
        .add_horizontal_spacer().add(m_test_text_area).add_horizontal_spacer().add_line_seperator()
        .add(m_exit);
}

void TestFrame::process_event(const sf::Event & event) {
    Frame::process_event(event);
    if (m_redo_setup) {
        setup();
        m_redo_setup = false;
    }
}

/* private */ void TestFrame::first_setup() {
    m_first_setup = false;

    m_styles[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");

    m_amount_of_text_notice.set_string(U"Number of Words.");
    m_width_notice         .set_string(U"Width (pixels)"  );
    m_height_notice        .set_string(U"Height (pixels)" );

    m_amount_of_text.set_options(make_range(k_min_words, k_word_count, k_word_step));
    m_width_slider  .set_options(make_range({ U"auto" }, k_min_size, k_max_size, k_size_step));
    m_height_slider .set_options(make_range({ U"auto" }, k_min_size, k_max_size, k_size_step));

    m_exit.set_string(U"Exit");
    m_exit.set_press_event([this]() { m_req_exit = true; });

    m_amount_of_text.set_option_change_event([this](){ m_redo_setup = true; });
    m_width_slider  .set_option_change_event([this](){ m_redo_setup = true; });
    m_height_slider .set_option_change_event([this](){ m_redo_setup = true; });
}

void run_tests() {
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
    const int org_n = n;
    const CharType * rv = beg;
    for_split<is_space>(beg, end_of_str(beg),
        [&n, &rv](const CharType * beg, const CharType *)
    {
        rv = beg;
        if (n) {
            --n;
            return fc_signal::k_continue;
        } else {
            return fc_signal::k_break;
        }
    });
    if (n > 1) {
        throw std::invalid_argument("after_n_words: string does not contain " + std::to_string(org_n) + " words.");
    }
    if (n == 1) return end_of_str(beg);

    return rv;
}

} // end of <anonymous> namespace
