#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/OptionsSlider.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream>

#include <cassert>

using StyleMap = ksg::StyleMap;
using UChar    = ksg::Text::UChar;

namespace {

class AppFrame : public ksg::Frame {
public:
    void setup_frame(const StyleMap & styles) {
        set_title(get_frame_name());
        set_location(get_start_location().x, get_start_location().y);
        setup_widgets(styles);

    }
    static bool requesting_quit() { return s_requesting_app_quit; }
protected:
    virtual void setup_widgets(const StyleMap &) = 0;
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

    void setup_widgets(const StyleMap &) override;

    ksg::TextArea m_text;
    ksg::TextButton m_ok;
};

class ExFrameB final : public AppFrame {
    VectorF get_start_location() const override { return VectorF(200.f, 200.f); }
    const UChar * get_frame_name() const override { return U"Frame B"; }

    void setup_widgets(const StyleMap &) override;

    ksg::TextArea m_text;
    ksg::OptionsSlider m_slider;
};

class ExFrameC final : public AppFrame {
    VectorF get_start_location() const override { return VectorF(200.f, 0.f); }
    const UChar * get_frame_name() const override { return U"Frame C"; }

    void setup_widgets(const StyleMap &) override;

    ksg::TextArea m_exit_notice;
    ksg::TextButton m_exit_button;
};

sf::Event filter_mouse_only_events(sf::Event);

} // end of <anonymous> namespace

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode(640, 480), "Draggable Frames", sf::Style::Close);
    window.setFramerateLimit(20);

    auto styles = ksg::styles::construct_system_styles();
    auto font   = ksg::styles::load_font("font.ttf");
    if (!font.is_valid()) {
        std::cerr << "Error loading font." << std::endl;
        return ~0;
    }
    styles[ksg::styles  ::k_global_font] = font;
    styles[ksg::TextArea::k_text_size  ] = ksg::StylesField(18.f);

    std::vector<std::shared_ptr<AppFrame>> frame_list;
    frame_list.push_back(std::make_shared<ExFrameA>());
    frame_list.push_back(std::make_shared<ExFrameB>());
    frame_list.push_back(std::make_shared<ExFrameC>());

    std::shared_ptr<AppFrame> requesting_focus = nullptr;
    for (auto & frame : frame_list) {
        frame->setup_frame(styles);
        // will obviously last the duration of the frame
        frame->set_register_click_event([frame, &requesting_focus]() {
            requesting_focus = frame;
            return ksg::FrameBorder::k_continue_other_events;
        });
    }

    while (window.isOpen()) {
        {
        sf::Event event;
        while (window.pollEvent(event)) {
            for (auto & frame : frame_list) {
                frame->process_event(
                    frame == frame_list.back() ? event : filter_mouse_only_events(event));
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
        for (auto & frame : frame_list)
            window.draw(*frame);
        window.display();
    }
    return 0;
}

namespace {

/* static */ bool AppFrame::s_requesting_app_quit = false;

// ----------------------------------------------------------------------------

void ExFrameA::setup_widgets(const StyleMap & smap) {
    m_text.set_text(U"Some random text for frame A.");
    m_ok.set_string(U"Ok");

    begin_adding_widgets(smap).
        add(m_text).
        add_line_seperator().
        add_horizontal_spacer().
        add(m_ok).
        add_horizontal_spacer();
}

// ----------------------------------------------------------------------------

void ExFrameB::setup_widgets(const StyleMap & smap) {
    m_text.set_text(U"Sample text for frame B,\ndifferent from frame A.");

    std::vector<ksg::Frame::UString> options =
        { U"Option one", U"Option two", U"Option three" };
    m_slider.swap_options(options);    

    begin_adding_widgets(smap).
        add(m_text).add_line_seperator().
        add(m_slider);
}

// ----------------------------------------------------------------------------

void ExFrameC::setup_widgets(const StyleMap & smap) {
    m_exit_notice.set_text(U"Press this button to exit the application.");
    m_exit_button.set_string(U"Close");
    m_exit_button.set_press_event([]() {
        AppFrame::request_application_quit();
    });

    begin_adding_widgets(smap).
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
