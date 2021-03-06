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
#include <asgl/OptionsSlider.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

#include <cassert>

namespace {

using VectorF = sf::Vector2f;
using asgl::UChar;

class AppFrame : public asgl::Frame {
public:
    void setup_frame() {
        set_title(get_frame_name());
        set_location(get_start_location().x, get_start_location().y);
        setup_widgets();
    }
    static bool requesting_quit() { return s_requesting_app_quit; }
protected:
    virtual void setup_widgets() = 0;
    virtual VectorF get_start_location() const = 0;
    virtual const UChar * get_frame_name() const = 0;

    static void request_application_quit()
        { s_requesting_app_quit = true; }
private:
    static bool s_requesting_app_quit;
};

class ExFrameA final : public AppFrame {
    VectorF get_start_location() const override { return VectorF(0.f, 140.f); }

    const UChar * get_frame_name() const override { return U"Frame A"; }

    void setup_widgets() override;
    asgl::TextArea m_text;
    asgl::TextButton m_ok;
};

class ExFrameB final : public AppFrame {
    VectorF get_start_location() const override { return VectorF(200.f, 200.f); }
    const UChar * get_frame_name() const override { return U"Frame B"; }

    void setup_widgets() override;
    asgl::TextArea m_text;
    asgl::OptionsSlider m_slider;
};

class ExFrameC final : public AppFrame {
    VectorF get_start_location() const override { return VectorF(200.f, 0.f); }
    const UChar * get_frame_name() const override { return U"Frame C"; }

    void setup_widgets() override;

    asgl::TextArea m_exit_notice;
    asgl::TextButton m_exit_button;
};

sf::Event filter_mouse_only_events(sf::Event);

} // end of <anonymous> namespace

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode(640, 480), "Draggable Frames", sf::Style::Close);
    window.setFramerateLimit(20);

    asgl::SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");
    engine.setup_default_styles();

    std::vector<std::shared_ptr<AppFrame>> frame_list;
    frame_list.push_back(std::make_shared<ExFrameA>());
    frame_list.push_back(std::make_shared<ExFrameB>());
    frame_list.push_back(std::make_shared<ExFrameC>());

    std::shared_ptr<AppFrame> requesting_focus = nullptr;
    for (auto & frame : frame_list) {
        frame->setup_frame(/*styles*/);
        // will obviously last the duration of the frame
        frame->set_register_click_event([frame, &requesting_focus]() {
            requesting_focus = frame;
            return asgl::FrameBorder::k_continue_other_events;
        });
        engine.stylize(*frame);
    }

    while (window.isOpen()) {
        {
        sf::Event event;
        while (window.pollEvent(event)) {
            for (auto & frame : frame_list) {
                frame->process_event(asgl::SfmlFlatEngine::convert(
                    frame == frame_list.back() ? event : filter_mouse_only_events(event)));
            }
            if (requesting_focus) {
                auto itr = std::find(frame_list.begin(), frame_list.end(), requesting_focus);
                std::swap(*itr, frame_list.back());
                requesting_focus = nullptr;
            }
            if (AppFrame::requesting_quit()) {
                window.close();
            }
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            default: break;
            }
        }

        }


        window.clear();
        for (auto & frame : frame_list) {
            frame->check_for_geometry_updates();
            engine.draw(*frame, window);
        }
        window.display();
    }
    return 0;
}

namespace {

/* static */ bool AppFrame::s_requesting_app_quit = false;

// ----------------------------------------------------------------------------

void ExFrameA::setup_widgets() {
    m_text.set_string(U"Some random text for frame A.");
    m_ok.set_string(U"Ok");

    begin_adding_widgets().
        add(m_text).
        add_line_seperator().
        add_horizontal_spacer().
        add(m_ok).
        add_horizontal_spacer();
}

// ----------------------------------------------------------------------------

void ExFrameB::setup_widgets() {
    m_text.set_string(U"Sample text for frame B,\ndifferent from frame A.");

    std::vector<asgl::UString> options =
        { U"Option one", U"Option two", U"Option three" };
    m_slider.set_options(std::move(options));
    begin_adding_widgets().
        add(m_text).add_line_seperator().
        add(m_slider);
}

// ----------------------------------------------------------------------------

void ExFrameC::setup_widgets() {
    m_exit_notice.set_string(U"Press this button to exit the application.");
    m_exit_button.set_string(U"Close");
    m_exit_button.set_press_event([]() {
        AppFrame::request_application_quit();
    });

    begin_adding_widgets().
        add(m_exit_notice).add_line_seperator().
        add_horizontal_spacer().add(m_exit_button).add_horizontal_spacer();
}

// ----------------------------------------------------------------------------

sf::Event filter_mouse_only_events(sf::Event event) {
    switch (event.type) {
    case sf::Event::MouseWheelMoved    :
    case sf::Event::MouseWheelScrolled :
    case sf::Event::MouseButtonPressed :
    case sf::Event::MouseButtonReleased:
    case sf::Event::MouseMoved         : break;
    default:
        event.type = sf::Event::Count;
    }
    return event;
}

} // end of <anonymous> namespace
