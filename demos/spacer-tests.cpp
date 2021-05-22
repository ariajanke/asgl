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
#include <asgl/TextButton.hpp>
#include <asgl/ArrowButton.hpp>
#include <asgl/ProgressBar.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

using UString     = asgl::Text::UString;
using Frame       = asgl::Frame;
using TextArea    = asgl::TextArea;
using ArrowButton = asgl::ArrowButton;
using ProgressBar = asgl::ProgressBar;
using TextButton  = asgl::TextButton;

namespace {

enum { k_pb0_style, k_pb1_style, k_custom_style_count };

using CustomStyles = asgl::styles::StyleKeysEnum<decltype(k_pb0_style), k_custom_style_count>;

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
    TextButton m_force_update_geo;
    // newline
    // spacer
    TextButton m_exit;
    // spacer
    bool m_request_close_flag;
};

} // end of <anonymous> namespace

int main() {
    SpacerTest dialog;

    asgl::SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");
    //engine.setup_default_styles();
    engine.add_rectangle_style(sf::Color( 12, 200, 86), CustomStyles::to_key(k_pb0_style));
    engine.add_rectangle_style(sf::Color(200,  12, 86), CustomStyles::to_key(k_pb1_style));
    dialog.setup_frame();
    engine.stylize(dialog);
    dialog.check_for_geometry_updates();

    sf::RenderWindow window(
        sf::VideoMode(unsigned(dialog.width() + 200), unsigned(dialog.height() + 200)),
        "Window Title", sf::Style::Close);
    window.setFramerateLimit(20);
    engine.assign_target_and_states(window, sf::RenderStates::Default);

    bool has_events = true;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            has_events = true;
            dialog.process_event(asgl::SfmlFlatEngine::convert( event ));
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (dialog.requesting_to_close())
            window.close();
        if (has_events) {
            window.clear();
            dialog.check_for_geometry_updates();
            dialog.draw(engine);

            dialog.iterate_children_const_f([&window](const asgl::Widget & widget) {
                DrawRectangle rect(float(widget.location().x), float(widget.location().y),
                                   float(widget.width()), float(widget.height()), sf::Color(140, 0, 0, 50));
                window.draw(rect);
            });

#           if 0
            window.draw(dialog);
#           endif
            window.display();
            has_events = false;
        } else {
            sf::sleep(sf::microseconds(16667));
        }
    }
}

namespace {

void SpacerTest::setup_frame() {
    m_row1_ta.set_string(U"Hjg Sample");
    m_row1_ab.set_direction(ArrowButton::Direction::k_right);
    m_row1_ab.set_size(32, 32);
    m_row2_pb.set_size(100, 32);
    m_row2_pb.set_fill_amount(0.48f);

    m_row2_ta.set_string(U"Hello World");
    m_row3_ab.set_direction(ArrowButton::Direction::k_down);
    m_row3_ab.set_size(32, 32);
    m_row3_ta.set_string(U"Row 3");
    m_row3_pb.set_size(100, 32);
    m_row3_pb.set_fill_amount(0.78f);
    m_exit.set_string(U"Close Application");
    m_exit.set_press_event([this]() { m_request_close_flag = true; });
#   if 0
    auto styles = ksg::styles::construct_system_styles();
    styles[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
#   endif

    m_row2_pb.set_fill_style(CustomStyles::to_key(k_pb0_style));
    m_row3_pb.set_fill_style(CustomStyles::to_key(k_pb1_style));

    m_force_update_geo.set_string(U"Force Geometry Update");
    m_force_update_geo.set_press_event([this]() { flag_needs_whole_family_geometry_update(); });

    set_width_minimum(1000);

    begin_adding_widgets(/*styles*/).
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
        add(m_force_update_geo).
        add_line_seperator().
        add_horizontal_spacer().
        add( m_exit).
        add_horizontal_spacer();
#   if 0
    // override styles for specific widgets
    m_row2_pb.set_inner_front_color(sf::Color( 12, 200, 86));
    m_row3_pb.set_inner_front_color(sf::Color(200,  12, 86));
#   endif
}

} // end of <anonymous> namespace
