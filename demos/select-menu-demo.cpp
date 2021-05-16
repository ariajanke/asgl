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
#include <asgl/ImageWidget.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/TextButton.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <common/Grid.hpp>
#if 0
#include "FillIterate.hpp"
#endif
#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Sleep.hpp>

#include <iostream>

#include <cassert>

namespace {

using InvArg = std::invalid_argument;
template <typename ... Types>
using Tuple = std::tuple<Types...>;
using VectorI = asgl::Widget::VectorI;

using namespace asgl;

// ------------------------------ generalization ------------------------------

class SelectionEntryDecoration final : public FrameDecoration {
public:
    // This has a set, fixed size
    void set_size(int width, int height) {
        m_bounds.width  = width ;
        m_bounds.height = height;
    }

    VectorI widget_start() const final { return get_top_left(m_bounds); }

    VectorI location() const final { return get_top_left(m_bounds); }

    int width() const final { return m_bounds.width; }

    int height() const final { return m_bounds.height; }

    EventResponseSignal process_event(const asgl::Event &) final
        { return EventResponseSignal(); }

    void set_location(int frame_x, int frame_y) final {
        set_top_left(m_bounds, frame_x, frame_y);
    }

    void stylize(const StyleMap &) final {}

    void update_geometry() final {}

    void draw(WidgetRenderer &) const final {}

    int minimum_width() const final { return width(); }

    int width_available_for_widgets() const final { return width(); }

    void set_click_inside_event(ClickFunctor &&) final {
        throw InvArg("SelectionEntryDecoration::set_click_inside_event: "
                     "This decoration does not accept click events.");
    }

private:
    void request_size(int, int) final
        { /* reject size requests */ }

    sf::IntRect m_bounds;
};

class SelectionEntryFrame : public BareFrame {
public:
    // It makes me sad, views are C++20 :c
    template <bool kt_is_const>
    class ShapeViewImpl {
    public:
        using PointerType = std::conditional_t<kt_is_const, const VectorI *, VectorI *>;
        ShapeViewImpl(PointerType beg, PointerType end);

        PointerType begin() const { return m_beg; }
        PointerType end  () const { return m_end; }

    private:
        PointerType m_beg;
        PointerType m_end;
    };

    using ShapeView = ShapeViewImpl<true>;

    // This has a set, fixed size
    void set_size(int width, int height);

protected:
    SelectionEntryFrame() {}

private:
    FrameDecoration & decoration() final { return m_deco; }

    const FrameDecoration & decoration() const final { return m_deco; }

    SelectionEntryDecoration m_deco;
};

class SelectionMenu : public BareFrame {
public:

private:
    FrameDecoration & decoration() final { return m_deco; }

    const FrameDecoration & decoration() const final { return m_deco; }

    SelectionEntryDecoration m_deco;

    int padding = 0;
    Grid<SelectionEntryFrame *> m_menu_grid;
};

// ------------------------------ specialization ------------------------------

class ForegroundDraggableImage final : public Draggable {
public:
    void set_image(SharedImagePtr image)
        { m_foreground_image = image; }

    void set_click_matrix(Grid<bool> && mask)
        { m_click_mask = std::move(mask); }

    sf::IntRect bounds(VectorI offset_) const {
        return sf::IntRect(offset_.x + m_offset.x, offset_.y + m_offset.y,
                           width(), height());
    }

    int width() const
        { return (m_click_mask.width() + 1)*scale(); }

    int height() const
        { return (m_click_mask.height() + 1)*scale(); }

    ItemKey item_key() const
        { return m_foreground_image->item_key(); }

    using Draggable::mouse_move;

    void drag_release() {
        m_offset = VectorI();
        Draggable::drag_release();
    }

    bool mouse_click(int x, int y, sf::Vector2i matrix_location) {
        auto pos = VectorI(x, y) - matrix_location;
        pos /= scale();
        if (!m_click_mask.has_position(pos)) return false;
        if (m_click_mask(pos)) return Draggable::mouse_click(x, y);
        return false;
    }

    void set_scale(int i) {
        if (i < 1) {
            throw InvArg("no");
        }
        m_scale = i;
    }
    int scale() const { return m_scale; }

private:
    void update_drag_position(int x, int y) final {
        m_offset = VectorI(x, y);
    }

    VectorI m_offset;
    Grid<bool> m_click_mask;
    SharedImagePtr m_foreground_image;
    int m_scale = 5;
};

// I want to test this first!
// Let's get the simplest parts finished first!
class DraggableImageWidget final : public Widget {
public:
    void process_event(const Event &) override;

    VectorI location() const override;

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;

    void update_geometry() override;

    void draw(WidgetRenderer &) const override;

    void load_images(ImageLoader &, const std::string & foreground_filename);
    void load_images(ImageLoader &, const std::string & foreground_filename,
                    const std::string & background_filename);
    void set_images(SharedImagePtr foreground_image,
                    SharedImagePtr background_image);

    void set_mask(const Grid<bool> &);

    void set_mask(Grid<bool> &&);

private:
    static const std::string k_no_file_filename;

    void set_location_(int x, int y) override;

    VectorI m_location;

    ForegroundDraggableImage m_foreground_image;
    SharedImagePtr m_background_image;
};

class ImageTextSelectionEntry final : public SelectionEntryFrame {
public:
    void load_images(ImageLoader &, const std::string & foreground_filename);
    void load_images(ImageLoader &, const std::string & foreground_filename,
                    const std::string & background_filename);
    void set_images(SharedImagePtr foreground_image,
                    SharedImagePtr background_image);

    enum Orientation { k_image_top, k_image_bottom, k_image_left, k_image_right };
    void setup(Orientation);

private:
    TextArea m_text;
    DraggableImageWidget m_image;
};

// --------------------------------- testing ----------------------------------

class TestFrame final : public Frame {
public:
    void setup(Tuple<SharedImagePtr, SharedImagePtr, Grid<bool>> && tuple) {
        setup(std::get<0>(tuple), std::get<1>(tuple), std::move(std::get<2>(tuple)));
    }

    void setup(SharedImagePtr foreground, SharedImagePtr background, Grid<bool> && mask) {
        m_image.set_images(foreground, background);
        m_image.set_mask(std::move(mask));

        m_exit.set_press_event([this]() { m_request_exit = true; });
        m_exit.set_string(U"Quit App");

        begin_adding_widgets()
            .add(m_image).add_line_seperator()
            .add(m_exit);
    }

    bool is_requesting_exit() const { return m_request_exit; }

private:
    TextArea m_text;
    DraggableImageWidget m_image;

    TextButton m_exit;
    bool m_request_exit = false;
};

namespace icon_fields {

constexpr const int k_foreground   = 0;
constexpr const int k_background   = 1;
constexpr const int k_bitmask      = 2;

// anything less opaque (alpha of smaller value) is not part of the mask
constexpr const int k_opacity_limit = 127;
constexpr const int k_expansion_size = 3;

constexpr const int k_icon_size = 32;

} // end of icon_fields namespace -> into <anonymous>

Tuple<Grid<sf::Color>, Grid<sf::Color>, Grid<bool>> load_as_icon
    (const std::string & filename, int frame_number);

Tuple<SharedImagePtr, SharedImagePtr, Grid<bool>> load_as_icon
    (const std::string & filename, int frame_number, SfmlFlatEngine &);

} // end of <anonymous> namespace

int main() {

    SfmlFlatEngine engine;
    TestFrame test_frame;
    sf::RenderWindow win;

    engine.assign_target_and_states(win, sf::RenderStates::Default);
    engine.load_global_font("font.ttf");
    test_frame.setup(load_as_icon("images/project-utumno-excerpt.png", 3, engine));
    engine.stylize(test_frame);

    win.create(sf::VideoMode(640, 480), " ");
    win.setFramerateLimit(20);
    while (win.isOpen()) {
        if (test_frame.is_requesting_exit()) return 0;
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            test_frame.process_event(SfmlFlatEngine::convert(event));
            switch (event.type) {
            case sf::Event::Closed:
                win.close();
                break;
            default: break;
            }
        }
        }
        sf::sleep(sf::microseconds(16667));

        test_frame.check_for_geometry_updates();
        win.clear();
        test_frame.draw(engine);
        win.display();
    }
    return 0;
}

namespace {

// ------------------------------ generalization ------------------------------

// ------------------------------ specialization ------------------------------

void DraggableImageWidget::process_event(const Event & event) {
    switch (event.type_id()) {
    case k_mouse_press_id: {
        auto pos = to_vector(event.as<MousePress>());
        m_foreground_image.mouse_click(pos.x, pos.y, m_location);
        }
        break;
    case k_mouse_move_id: {
        auto pos = to_vector(event.as<MouseMove>());
        m_foreground_image.mouse_move(pos.x, pos.y);
        }
        break;
    case k_mouse_release_id:
        m_foreground_image.drag_release();
        break;
    default: break;
    }
}

VectorI DraggableImageWidget::location() const { return m_location; }

int DraggableImageWidget::width() const { return m_foreground_image.width(); }

int DraggableImageWidget::height() const { return m_foreground_image.height(); }

void DraggableImageWidget::stylize(const StyleMap &) {}

void DraggableImageWidget::update_geometry() {}

void DraggableImageWidget::draw(WidgetRenderer & target) const {
    sf::IntRect bounds = m_foreground_image.bounds(m_location);
    set_top_left(bounds, m_location);
    draw_to(target, bounds, m_background_image->item_key());

    bounds = m_foreground_image.bounds(m_location);
    draw_to(target, bounds, m_foreground_image.item_key());
}

void DraggableImageWidget::load_images
    (ImageLoader & loader, const std::string & foreground_filename)
{
    load_images(loader, foreground_filename, k_no_file_filename);
}

void DraggableImageWidget::load_images
    (ImageLoader & loader, const std::string & foreground_filename,
     const std::string & background_filename)
{
    SharedImagePtr image_ptr = nullptr;
    if (background_filename != k_no_file_filename) {
        image_ptr = loader.make_image_resource(background_filename);
    }
    set_images(loader.make_image_resource(foreground_filename), image_ptr);
}

void DraggableImageWidget::set_images
    (SharedImagePtr foreground_image, SharedImagePtr background_image)
{
    m_foreground_image.set_image(foreground_image);
    m_background_image = background_image;
}

void DraggableImageWidget::set_mask(const Grid<bool> & mask) {
    auto temp = mask;
    set_mask(std::move(temp));
}

void DraggableImageWidget::set_mask(Grid<bool> && mask) {
    m_foreground_image.set_click_matrix(std::move(mask));
    flag_needs_whole_family_geometry_update();
}

/* private */ void DraggableImageWidget::set_location_(int x, int y) {
    m_location = VectorI(x, y);
}

/* private static */ const std::string DraggableImageWidget::k_no_file_filename = "";

// ----------------------------------------------------------------------------

void ImageTextSelectionEntry::load_images
    (ImageLoader & loader, const std::string & foreground_filename)
{ m_image.load_images(loader, foreground_filename); }

void ImageTextSelectionEntry::load_images
    (ImageLoader & loader, const std::string & foreground_filename,
     const std::string & background_filename)
{ m_image.load_images(loader, foreground_filename, background_filename); }

void ImageTextSelectionEntry::set_images
    (SharedImagePtr foreground_image, SharedImagePtr background_image)
{ m_image.set_images(foreground_image, background_image); }


void ImageTextSelectionEntry::setup(Orientation ori) {
    bool should_add_text = !m_text.string().empty();
    switch (ori) {
    case k_image_bottom: {
        auto adder = begin_adding_widgets();
        if (should_add_text) {
            adder.add_horizontal_spacer().add(m_text).add_horizontal_spacer()
                 .add_line_seperator();
        }
        adder.add(m_image);
        }
        break;
    case k_image_top: {
        auto adder = begin_adding_widgets();
        adder.add(m_image);
        if (should_add_text) {
            adder.add_line_seperator()
                 .add_horizontal_spacer().add(m_text).add_horizontal_spacer();
        }
        }
        break;
    // under these two cases we would want a "fixed height" text
    case k_image_left: {
        }

        break;
    case k_image_right:
        break;
    }
}

// --------------------------------- testing ----------------------------------

template <typename Func>
inline void do_n_times(int n, Func && f) {
    for (int i = 0; i != n; ++i) f();
}

static const auto k_neighbors =
    { VectorI(1, 0), VectorI(-1, 0), VectorI(0, 1), VectorI(0, -1) };

template <typename T, typename Predicate>
inline bool any_are_edge
    (const Grid<T> & grid, VectorI r, Predicate && is_inside)
{
    assert(grid.has_position(r));
    assert(is_inside(grid(r)));
    for (auto n : k_neighbors) {
        if (!grid.has_position(n + r)) {
            // image edge pixels are considered on the edge
            return true;
        } else if (!is_inside(grid(n + r))) {
            return true;
        }
    }
    return false;
}

auto make_strict_weak_vector_orderer(const std::vector<VectorI> & col) {
    int max_x = std::max_element(col.begin(), col.end(),
        [](const VectorI & a, const VectorI & b) { return a.x < b.x; })->x + 1;
    if (max_x*max_x <= max_x) {
        throw std::overflow_error("make_strict_weak_vector_orderer: maximum value x squared overflows the data type.");
    }
    return [max_x](const VectorI & a, const VectorI & b) {
        int a_v = a.x + a.y*max_x;
        int b_v = b.x + b.y*max_x;
        return a_v < b_v;
    };
}

inline bool is_opaque_enough(sf::Color color)
    { return color.a >= icon_fields::k_opacity_limit; }

inline Grid<sf::Color> load_image(const std::string & fn) {
    sf::Image img;
    if (!img.loadFromFile(fn)) {
        throw InvArg("Cannot load file \"" + fn + "\".");
    }
    Grid<sf::Color> imgout;
    imgout.set_size(int(img.getSize().x), int(img.getSize().y));
    for (unsigned y = 0; y != img.getSize().y; ++y) {
    for (unsigned x = 0; x != img.getSize().x; ++x) {
        imgout(int(x), int(y)) = img.getPixel(x, y);
    }}
    return imgout;
}

inline void clean_up_edge_pixels(std::vector<VectorI> & edge_pixels) {
    std::sort(edge_pixels.begin(), edge_pixels.end(), make_strict_weak_vector_orderer(edge_pixels));
    static const VectorI k_remove_cand(-1, -1);
    static auto is_remove_cand = [](VectorI r) { return r == k_remove_cand; };
    if (std::any_of(edge_pixels.begin(), edge_pixels.end(), is_remove_cand)) {
        throw InvArg("No edge pixel maybe the remove candidate (should not be possible, the remove candidate is not a valid location on *any* grid).");
    }
    if (edge_pixels.size() > 1) {
        auto itr = edge_pixels.begin();
        for (auto jtr = itr + 1; jtr != edge_pixels.end(); ++jtr, ++itr) {
            if (*itr != *jtr) continue;
            *itr = k_remove_cand;
        }
    }
    auto rem_itr = std::remove_if(edge_pixels.begin(), edge_pixels.end(), is_remove_cand);
    edge_pixels.erase(rem_itr, edge_pixels.end());
}

inline Grid<bool> expand_mask(const Grid<bool> & mask, std::vector<VectorI> edge_pixels_copy, std::size_t reserve_offset = 0) {
    auto temp_mask = mask;
    do_n_times(icon_fields::k_expansion_size, [&]() {
        std::vector<VectorI> new_edge_pixels;
        new_edge_pixels.reserve(edge_pixels_copy.size() + reserve_offset);
        for (auto r : edge_pixels_copy) {
            assert(temp_mask(r));
            for (auto n : k_neighbors) {
                if (!temp_mask.has_position(n + r)) continue;
                if (!temp_mask(n + r)) {
                    temp_mask(n + r) = true;
                    new_edge_pixels.push_back(n + r);
                }
            }
        }
        clean_up_edge_pixels(new_edge_pixels);
        new_edge_pixels.swap(edge_pixels_copy);
    });
    return temp_mask;
}

Tuple<Grid<sf::Color>, Grid<sf::Color>, Grid<bool>> load_as_icon
    (const std::string & filename, int frame_number)
{
    auto img = [&] {
        using namespace icon_fields;
        auto full_img = load_image(filename);

        int width_in_icons  = full_img.width () / k_icon_size;
        int height_in_icons = full_img.height();

        if (frame_number >= width_in_icons*height_in_icons) {
            throw InvArg("no");
        }
        VectorI icon_offset(frame_number % width_in_icons, frame_number / width_in_icons);
        auto source = make_sub_grid(full_img, icon_offset*k_icon_size, k_icon_size, k_icon_size);

        Grid<sf::Color> dest;
        dest.set_size(k_icon_size + k_expansion_size*2,
                      k_icon_size + k_expansion_size*2,
                      sf::Color(0, 0, 0, 0));
        for (VectorI r; r != source.end_position(); r = source.next(r)) {
            dest(r + VectorI(1, 1)*k_expansion_size) = source(r);
        }
        return dest;
    } ();

    std::vector<VectorI> edge_pixels;
    Grid<bool> mask;
    // guess reserve
    edge_pixels.reserve( img.size() / 4 );
    mask.set_size(img.width(), img.height(), false);
    for (VectorI r; r != img.end_position(); r = img.next(r)) {
        assert(img.has_position(r));
        if (!is_opaque_enough(img(r))) continue;
        mask(r) = true;
        if (any_are_edge(img, r, is_opaque_enough)) {
            edge_pixels.push_back(r);
        }
    }

    clean_up_edge_pixels(edge_pixels);

    auto expanded_mask = expand_mask(mask, edge_pixels,  std::max(img.width(), img.height())*4);

    // NOT STATED IN DOCUMENTATION
    // by "RGBA" most significant is the rightmost letter
    static const sf::Color k_transparency(0x0);
    static const auto k_pallete = {
        sf::Color(0xD0D0D0FF),
        sf::Color(0xFFFFFFFF),
        sf::Color(0xD0D0D0FF),

        k_transparency,
        k_transparency,

        sf::Color(0x202020FF),
        sf::Color(0x000000FF),
        sf::Color(0x202020FF)
    };
    Grid<sf::Color> mask_img;
    mask_img.set_size(img.width(), img.height(), k_transparency);
    for (VectorI r; r != mask_img.end_position(); r = mask_img.next(r)) {
        if (!expanded_mask(r)) continue;
        int idx = (r.x + r.y % int(k_pallete.size())) % int(k_pallete.size());
        mask_img(r) = *(k_pallete.begin() + idx);
    }

    return std::make_tuple(std::move(img), std::move(mask_img), std::move(mask));
}

Tuple<SharedImagePtr, SharedImagePtr, Grid<bool>> load_as_icon
    (const std::string & filename, int frame_number, SfmlFlatEngine & engine)
{
    auto [img, mask_img, mask] = load_as_icon(filename, frame_number);
    return std::make_tuple(engine.make_image_from(img),
        engine.make_image_from(mask_img), std::move(mask));
}

} // end of <anonymous> namespace
