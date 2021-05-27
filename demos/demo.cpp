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

// a possible way to compile this application:
// g++ -std=c++17 demo.cpp -lcommon-d -lksg -lsfml-graphics -lsfml-window -lsfml-system -lz -L/media/ramdisk/ksg/demos -I../inc -I../lib/cul/inc -o demo

#include <asgl/Frame.hpp>
#include <asgl/ImageWidget.hpp>
#include <asgl/OptionsSlider.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>

using asgl::UString;

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
    FruitFrame       m_embeded_frame;

    bool m_close_flag;
};

} // end of <anonymous>

int main() {
    asgl::SfmlFlatEngine engine;
    engine.load_global_font("font.ttf");
    engine.setup_default_styles();

    DemoText dialog;
    dialog.setup_frame(engine);
    engine.stylize(dialog);
    dialog.check_for_geometry_updates();

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
            dialog.process_event(asgl::SfmlFlatEngine::convert(event));
            if (event.type == sf::Event::Closed)
                window.close();
        }
        dialog.check_for_geometry_updates();
        if (dialog.requesting_to_close())
            window.close();
        if (has_events) {
            window.clear();
            dialog.draw(engine);
            window.display();
            has_events = false;
        } else {
            sf::sleep(sf::microseconds(16667));
        }
    }
}

namespace {

void FruitFrame::setup_frame(asgl::ImageLoader & loader) {
    begin_adding_widgets().
        add(m_image_widget).
        add_line_seperator().
        // --------------------
        add_horizontal_spacer().
        add(m_slider).
        add_horizontal_spacer();

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

    m_text_area.set_limiting_line(200);
    m_text_area.set_string(U"Hello World.\n"
        "Images of fruit were graciously "
        "provided by \"freefoodphotos.com\" "
        "each of which are released under "
        "the creative commons attribution "
        "(3.0) license.");

    m_text_button.set_string(U"Close Application");

    m_embeded_frame.setup_frame(loader);

    begin_adding_widgets().
        add(m_text_area).
        add_horizontal_spacer().
        add(m_embeded_frame).
        add_line_seperator().
        add_horizontal_spacer().
        add(m_text_button).
        add_horizontal_spacer();
}

} // end of <anonymous> namespace
