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
#if 0
#include <ksg/Frame.hpp>

#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/OptionsSlider.hpp>
#endif

#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/TextButton.hpp>
#include <asgl/OptionsSlider.hpp>

#include <asgl/SfmlFlatRenderer.hpp>

#include <thread>

namespace {

using asgl::Frame;
using asgl::TextArea;
using asgl::TextButton;
using asgl::OptionsSlider;

constexpr const bool k_enable_borders = false;

class FurthestNested final : public Frame {
public:
    void setup();
private:
    TextArea m_furthest_nested_notice;
};

class NestedWithSliders final : public Frame {
public:
    void setup();
private:
    FurthestNested m_nested;
    OptionsSlider m_one;
    OptionsSlider m_two;
    OptionsSlider m_three;
};

class TopLevelFrame final : public Frame {
public:
    void setup();
    bool requesting_exit() const { return m_requesting_quit; }
private:
    TextArea m_top_level_notice;
    NestedWithSliders m_nested;
    TextButton m_exit;
    bool m_requesting_quit = false;
};

} // end of <anonymous> namespace

int main() {
    TopLevelFrame frame;
    frame.setup();

    asgl::SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");

    sf::RenderWindow win;
    engine.assign_target_and_states(win, sf::RenderStates::Default);
    engine.stylize(frame);

    win.create(sf::VideoMode(unsigned(frame.width()), unsigned(frame.height())), " ");
    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            frame.process_event(asgl::convert( event ));
            if (event.type == sf::Event::Closed || frame.requesting_exit()) {
                win.close();
            }
        }
        frame.check_for_geometry_updates();
        win.clear();
        frame.draw(engine);
#       if 0
        win.draw(frame);
#       endif
        win.display();
        std::this_thread::sleep_for(std::chrono::microseconds(16667));
    }
}

namespace {

using UString = asgl::Text::UString;

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
#   if 0
    auto styles_ = ksg::styles::construct_system_styles();
    styles_[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
#   endif
    set_title(U"Nested Frames Test App");
    set_drag_enabled(false);

    m_nested.setup();

    m_top_level_notice.set_string(U"Top level text notice.");
#   if 0
    m_top_level_notice.set_character_size(22);
#   endif
    m_exit.set_string(U"Exit");
    m_exit.set_press_event([this]() { m_requesting_quit = true; });

    begin_adding_widgets()
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
