#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/ArrowButton.hpp>
#include <ksg/ProgressBar.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

using UString     = ksg::Text::UString;
using Frame       = ksg::Frame;
using TextArea    = ksg::TextArea;
using ArrowButton = ksg::ArrowButton;
using ProgressBar = ksg::ProgressBar;
using TextButton  = ksg::TextButton;

namespace {

class SpacerTest final : public Frame {
public:
    SpacerTest(): m_request_close_flag(false) {}

    bool requesting_to_close() const { return m_request_close_flag; }
    void setup_frame(/*const sf::Font &*/);
private:
    TextArea m_row1_ta;
    // spacer
    ArrowButton m_row1_ab;
    // spacer
    // newline
    // spacer
    ProgressBar m_row2_pb;
    // spacer
    TextArea m_row2_ta;
    // newline
    ArrowButton m_row3_ab;
    // spacer
    TextArea m_row3_ta;
    // spacer
    ProgressBar m_row3_pb;
    // spacer
    // newline
    // spacer
    TextButton m_exit;
    // spacer
    bool m_request_close_flag;
};

} // end of <anonymous> namespace

int main() {
    SpacerTest dialog;
    dialog.setup_frame();

    sf::RenderWindow window(
        sf::VideoMode(unsigned(dialog.width()), unsigned(dialog.height())), 
        "Window Title", sf::Style::Close);
    window.setFramerateLimit(20);
    bool has_events = true;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            has_events = true;
            dialog.process_event(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (dialog.requesting_to_close())
            window.close();
        if (has_events) {
            window.clear();
            window.draw(dialog);
            window.display();
            has_events = false;
        } else {
            sf::sleep(sf::microseconds(16667));
        }
    }
}

namespace {

void SpacerTest::setup_frame() {
    m_row1_ta.set_text(U"Hjg Sample");
    m_row1_ab.set_direction(ArrowButton::Direction::k_right);
    m_row1_ab.set_size(32.f, 32.f);
    m_row2_pb.set_size(100.f, 32.f);
    m_row2_pb.set_fill_amount(0.48f);

    m_row2_ta.set_text(U"Hello World");
    m_row3_ab.set_direction(ArrowButton::Direction::k_down);
    m_row3_ab.set_size(32.f, 32.f);
    m_row3_ta.set_text(U"Row 3");
    m_row3_pb.set_size(100.f, 32.f);
    m_row3_pb.set_fill_amount(0.78f);
    m_exit.set_string(U"Close Application");
    m_exit.set_press_event([this]() { m_request_close_flag = true; });

    auto styles = ksg::styles::construct_system_styles();
    styles[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");

    begin_adding_widgets(styles).
        add( m_row1_ta).
        add_horizontal_spacer().
        add( m_row1_ab).
        add_horizontal_spacer().
        add_line_seperator().
        add_horizontal_spacer().
        add( m_row2_pb).
        add_horizontal_spacer().
        add( m_row2_ta).
        add_line_seperator().
        add( m_row3_ab).
        add_horizontal_spacer().
        add( m_row3_ta).
        add_horizontal_spacer().
        add( m_row3_pb).
        add_horizontal_spacer().
        add_line_seperator().
        add_horizontal_spacer().
        add( m_exit).
        add_horizontal_spacer();

    // override styles for specific widgets
    m_row2_pb.set_inner_front_color(sf::Color( 12, 200, 86));
    m_row3_pb.set_inner_front_color(sf::Color(200,  12, 86));
}

} // end of <anonymous> namespace
