#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <ksg/Frame.hpp>

#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/OptionsSlider.hpp>

#include <thread>

namespace {

constexpr const bool k_enable_borders = false;

class FurthestNested final : public ksg::Frame {
public:
    void setup();
private:
    ksg::TextArea m_furthest_nested_notice;
};

class NestedWithSliders final : public ksg::Frame {
public:
    void setup();
private:
    FurthestNested m_nested;
    ksg::OptionsSlider m_one;
    ksg::OptionsSlider m_two;
    ksg::OptionsSlider m_three;
};

class TopLevelFrame final : public ksg::Frame {
public:
    void setup();
    bool requesting_exit() const { return m_requesting_quit; }
private:
    ksg::TextArea m_top_level_notice;
    NestedWithSliders m_nested;
    ksg::TextButton m_exit;
    bool m_requesting_quit = false;
};

} // end of <anonymous> namespace

int main() {
    TopLevelFrame frame;
    frame.setup();

    sf::RenderWindow win;
    win.create(sf::VideoMode(unsigned(frame.width()), unsigned(frame.height())), " ");
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

std::vector<UString> options_from_range(int min, int max);

void FurthestNested::setup() {
    m_furthest_nested_notice.set_string
        (U"Text area in the furthest nested frame.");
    begin_adding_widgets().add(m_furthest_nested_notice);
    if (!k_enable_borders) set_frame_border_size(0.f);
}

void NestedWithSliders::setup() {
    m_nested.setup();

    m_one.set_options(options_from_range(4, 9));
    m_two.set_options(options_from_range(10, 14));
    m_three.set_options(options_from_range(19, 26));
    m_three.set_wrap_enabled(true);

    if (!k_enable_borders) set_frame_border_size(0.f);

    begin_adding_widgets()
        .add(m_nested).add_line_seperator()
        .add_horizontal_spacer().add(m_one).add_line_seperator()
        .add_horizontal_spacer().add(m_two).add_line_seperator()
        .add_horizontal_spacer().add(m_three).add_line_seperator();
}

void TopLevelFrame::setup() {
    auto styles_ = ksg::styles::construct_system_styles();
    styles_[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");

    set_title(U"Nested Frames Test App");
    set_drag_enabled(false);

    m_nested.setup();

    m_top_level_notice.set_string(U"Top level text notice.");
    m_top_level_notice.set_character_size(22);

    m_exit.set_string(U"Exit");
    m_exit.set_press_event([this]() { m_requesting_quit = true; });

    begin_adding_widgets(styles_)
        .add(m_top_level_notice).add_line_seperator()
        .add(m_nested).add_line_seperator()
        .add_horizontal_spacer().add(m_exit);
}

// ----------------------------------------------------------------------------

std::vector<UString> options_from_range(int min, int max) {
    std::vector<UString> rv;
    rv.reserve(max - min + 1);
    for (int i = min; i != max + 1; ++i) {
        UString t;
        for (char c : std::to_string(i)) t.push_back(c);
        rv.emplace_back(std::move(t));
    }
    return rv;
}

} // end of <anonymous> namespace
