// a possible way to compile this application:
// g++ -std=c++17 demo.cpp -lcommon-d -lksg -lsfml-graphics -lsfml-window -lsfml-system -lz -L/media/ramdisk/ksg/demos -I../inc -I../lib/cul/inc -o demo
#if 0
#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/ImageWidget.hpp>
#include <ksg/EditableText.hpp>
#endif

#include <asgl/Frame.hpp>
#include <asgl/Text.hpp>
#include <asgl/ImageWidget.hpp>
#include <asgl/OptionsSlider.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/Event.hpp>
#include <asgl/SfmlFlatRenderer.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>

using UString = asgl::SfmlTextObject::UString;

namespace {

class FruitFrame final : public asgl::Frame {
public:
    void setup_frame(asgl::ImageLoader &);
private:
    asgl::ImageWidget   m_image_widget;
    asgl::OptionsSlider m_slider;

    std::vector<asgl::SharedImagePtr> m_fruit_images;
};

class DemoText final : public asgl::Frame {
public:
    DemoText(): m_close_flag(false) {}
    void setup_frame(asgl::ImageLoader &);
    bool requesting_to_close() const { return m_close_flag; }
private:
    asgl::TextArea   m_text_area    ;
    asgl::TextButton m_text_button  ;
    FruitFrame      m_embeded_frame;

    bool m_close_flag;
};

} // end of <anonymous>

int main() {
#   if 0
    ksg::Text::run_tests();
#   endif
    asgl::SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");
    engine.setup_default_styles();
#   if 0
    {
    auto font_ptr = std::make_shared<sf::Font>();
    font_ptr->loadFromFile("font.ttf");
    engine.add_global_font(font_ptr);
    }
#   endif

#   if 1
    DemoText dialog;
    dialog.setup_frame(engine);
    engine.stylize(dialog);

    sf::RenderWindow window;
    window.create(sf::VideoMode(unsigned(dialog.width()), unsigned(dialog.height())),
                  "Window Title", sf::Style::Close);
    window.setFramerateLimit(20);
    engine.assign_target_and_states(window, sf::RenderStates::Default);

    bool has_events = true;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            has_events = true;
            dialog.process_event(asgl::convert(event));
            if (event.type == sf::Event::Closed)
                window.close();
        }
        //dialog.check_for_geometry_updates();
        if (dialog.requesting_to_close())
            window.close();
        if (has_events) {
            window.clear();
#           if 0
            window.draw(dialog);
#           endif
            dialog.draw(engine);
            window.display();
            has_events = false;
        } else {
            sf::sleep(sf::microseconds(16667));
        }
    }
#   endif
}

namespace {

void FruitFrame::setup_frame(asgl::ImageLoader & loader) {
#   if 1
    begin_adding_widgets().
        add(m_image_widget).
        add_line_seperator().
        // --------------------
        add_horizontal_spacer().
        add(m_slider).
        add_horizontal_spacer();
#   endif
    const auto image_files_c =
        { "images/orange.jpg", "images/apple.jpg", "images/bananas.jpg" };

    m_slider.set_options({ U"Orange", U"Apple", U"Bananas" });

    for (const auto & image_file : image_files_c) {        
        m_fruit_images.emplace_back(
            m_image_widget.load_image(loader, image_file));
    }
    m_image_widget.set_image(m_fruit_images.front());

    m_slider.set_option_change_event([this]() {
        m_image_widget.set_image
            (m_fruit_images[m_slider.selected_option_index()]);
    });
    m_image_widget.set_size(200, 150);
}

void DemoText::setup_frame(asgl::ImageLoader & loader) {
    // careful not to use/capture tempory objects here
    // this may result in a segmentation fault, and is undefined behavior
    m_text_button.set_press_event([this]() { m_close_flag = true; });

    m_text_area.set_string(U"Hello World");
    m_text_area.set_max_width(200);

    m_text_area.set_string(U"Hello World.\n"
        "Images of fruit were graciously "
        "provided by \"freefoodphotos.com\" "
        "each of which are released under "
        "the creative commons attribution "
        "(3.0) license.");

    m_text_button.set_string(U"Close Application");
#   if 0
    auto styles_ = ksg::styles::construct_system_styles();
    styles_[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
    styles_[Frame::k_border_size] = ksg::StylesField(0.f);
#   endif
#   if 1
    m_embeded_frame.setup_frame(loader);

    begin_adding_widgets().
        add(m_text_area).
        add_horizontal_spacer().
#       if 1
        add(m_embeded_frame).
        add_line_seperator().
        add_horizontal_spacer().
#       endif
        add(m_text_button).
        add_horizontal_spacer();
#   endif
}

} // end of <anonymous> namespace
