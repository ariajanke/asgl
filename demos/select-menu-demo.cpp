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

    VectorI widget_start() const final { return get_top_left(m_bounds); }

    VectorI location() const final { return get_top_left(m_bounds); }

    int width() const final { return 0; }//m_bounds.width; }

    int height() const final { return 0; } //m_bounds.height; }

    EventResponseSignal process_event(const asgl::Event &) final
        { return EventResponseSignal(); }

    void set_location(int frame_x, int frame_y) final {
        set_top_left(m_bounds, frame_x, frame_y);
    }

    void stylize(const StyleMap &) final {}

    void update_geometry() final {}

    void draw(WidgetRenderer &) const final {}

    int minimum_width() const final { return 0; }

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

// It's true I don't need an abstract class, but the attempt here is to limit
// the interface
class DrawableWithWidgetRenderer {
public:
    virtual ~DrawableWithWidgetRenderer() {}

    virtual bool is_requesting_drop() const = 0;

    void draw(const Widget & parent, WidgetRenderer & target) const
        { draw_(parent.location(), target); }

    void draw(const VectorI & offset, WidgetRenderer & target) const
        { draw_(offset, target); }

protected:
    virtual void draw_(const VectorI &, WidgetRenderer &) const = 0;
};

class ForegroundDraggableImage final :
    public Draggable, public DrawableWithWidgetRenderer
{
public:
    void set_image(SharedImagePtr image)
        { m_foreground_image = image; }

    void set_click_matrix(ConstSubGrid<bool> mask)
        { m_click_mask = mask; }
private:
    sf::IntRect bounds(VectorI offset_) const {
        return sf::IntRect(offset_.x + m_offset.x, offset_.y + m_offset.y,
                           width(), height());
    }
public:
    int width() const
        { return (m_click_mask.width() + 1)*scale(); }

    int height() const
        { return (m_click_mask.height() + 1)*scale(); }

    using Draggable::mouse_move;

    void drag_release() {
        m_offset = VectorI();
        Draggable::drag_release();
    }

    bool mouse_click(int x, int y, sf::Vector2i matrix_location) {
        auto pos = VectorI(x, y) - matrix_location;
        pos /= scale();
        if (!m_click_mask.has_position(pos)) return false;
        if (m_click_mask(pos))
            return Draggable::mouse_click(x, y);
        return false;
    }

    void set_scale(int i) {
        if (i < 1) {
            throw InvArg("no");
        }
        m_scale = i;
    }
    int scale() const { return m_scale; }

    void set_frame(const sf::IntRect & rect)
        { m_tx_bounds = rect; }
private:
    void draw_(const VectorI & offset, WidgetRenderer & renderer) const final {
        renderer.render_rectangle_pair(bounds(offset), m_tx_bounds, m_foreground_image->item_key(), this);
    }
public:
    bool is_requesting_drop() const final
        { return !is_being_dragged(); }

private:
    void update_drag_position(int x, int y) final {
        m_offset = VectorI(x, y);
    }

    VectorI m_offset;
    ConstSubGrid<bool> m_click_mask;
    SharedImagePtr m_foreground_image;
    sf::IntRect m_tx_bounds;
    int m_scale = 4;
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

    void set_frames(const sf::IntRect & foreground,
                    const sf::IntRect & background);

    void set_mask(ConstSubGrid<bool>);

    void set_scale(int scale)
        { m_foreground_image.set_scale(scale); }

    const DrawableWithWidgetRenderer * get_hold_request() const {
        if (m_foreground_image.is_requesting_drop()) return nullptr;
        return &m_foreground_image;
    }

private:
    static const std::string k_no_file_filename;

    void set_location_(int x, int y) override;

    VectorI m_location;

    ForegroundDraggableImage m_foreground_image;
    SharedImagePtr m_background_image;
    sf::IntRect m_background_frame;
};

class ImageTextSelectionEntry final : public SelectionEntryFrame {
public:
    enum Orientation { k_image_top, k_image_bottom, k_image_left, k_image_right };
    struct SharedParams {
        Orientation orientation = k_image_top;
        int scale = 1;
        int text_width  = 180;
        int text_height = 60 ;
        bool show_full_text_on_hover = true;
    };

    struct IndividualParams {
        // how do I do multiple frames for a single image without going
        // overboard here?
        SharedImagePtr foreground;
        SharedImagePtr background;

        sf::IntRect foreground_tx_bounds;
        sf::IntRect background_tx_bounds;

        ConstSubGrid<bool> click_mask;
        UString string;
    };

    void setup(const SharedParams &, const IndividualParams &);

    void stylize(const StyleMap & map) final {
        TextArea::set_required_text_fields(
            m_full_text, map.find(styles::k_global_font),
            map.find(Frame::to_key(Frame::k_widget_text_style)),
            "TestFrame::stylize");
        BareFrame::stylize(map);
    }

    void process_event(const Event & event) final {
        BareFrame::process_event(event);
        if (!event.is_type<MouseMove>()) return;
        sf::IntRect text_bounds(m_text.location(), VectorI(m_text.width(), m_text.height()));
        if (text_bounds.contains(to_vector(event.as<MouseMove>()))) {
            m_show_full_text = true;
            m_full_text.set_location(m_text.location());
        } else {
            m_show_full_text = false;
        }
    }

    bool mouse_is_over(const MouseLocation & loc) {
        return    loc.x >  location().x && loc.y > location().y
               && loc.x <= location().x + width ()
               && loc.y <= location().y + height();
    }

    void draw(WidgetRenderer & target) const final {
        if (m_show_full_text) {
            m_full_text.draw_to(target);
        } else {
            m_text.draw(target);
        }
        m_image.draw(target);
    }

    Tuple<const DrawableWithWidgetRenderer *, const Widget *> get_hold_request() const
        { return std::make_tuple(m_image.get_hold_request(), &m_image); }

private:

    void update_geometry() final {
        BareFrame::update_geometry();
    }

    void on_frame_geometry_update() final {

    }

    TextArea m_text;
    DraggableImageWidget m_image;

    bool m_show_full_text = false;
    Text m_full_text;
};

// --------------------------------- testing ----------------------------------

struct LoadIconsRt;

class TestFrame final : public Frame {
public:
    void setup(const LoadIconsRt & icon_info);

    bool is_requesting_exit() const { return m_request_exit; }

    void process_event(const Event & event) final {
        Frame::process_event(event);
        // really not sure what to do style wise
        // if (ptr && ptr->method()) is considered problematic since
        // short-circuiting is not immediately appearent
        // if (ptr) if (ptr->method) {
        //     ...
        // }
        if (m_held_item) /* and then */ if (m_held_item->is_requesting_drop()
            && event.is_type<MouseRelease>())
        {
            IconTilePtr * hovering_over = nullptr;
            for (auto & ptr : m_owned_widgets) {
                if (ptr->mouse_is_over(event.as<MouseRelease>())) {
                    hovering_over = &ptr;
                    break;
                }
            }
            assert(m_frame_being_dragged);
            if (hovering_over && m_frame_being_dragged != hovering_over) {
                std::swap(*hovering_over, *m_frame_being_dragged);
                // I need to re-add widgets :c
                setup_widgets();
            }
            m_held_item           = nullptr;
            m_held_items_parent   = nullptr;
            m_frame_being_dragged = nullptr;
        }
        for (auto & drag_icon : m_owned_widgets) {
            auto [wants_to_be_held, parent] = drag_icon->get_hold_request();
            if (!wants_to_be_held) continue;
            m_frame_being_dragged = &drag_icon;
            m_held_items_parent = parent;
            m_held_item = wants_to_be_held;
            break;
        }
    }

    void draw(WidgetRenderer & target) const {
        Frame::draw(target);
        if (m_held_item) {
            assert(m_held_items_parent);
            m_held_item->draw(*m_held_items_parent, target);
        }
    }

private:
    void setup_widgets() {
        auto adder = begin_adding_widgets();
        int x = 0;
        constexpr const int x_limit = 2;
        for (auto & widget_ptr : m_owned_widgets) {
            adder.add(*widget_ptr);
            if (++x == x_limit) {
                adder.add_line_seperator();
                x = 0;
            }
        }
        adder.add_horizontal_spacer().add(m_exit);
    }

    using IconTilePtr = std::unique_ptr<ImageTextSelectionEntry>;
    std::vector<IconTilePtr> m_owned_widgets;

    IconTilePtr * m_frame_being_dragged = nullptr;
    const Widget * m_held_items_parent = nullptr;
    const DrawableWithWidgetRenderer * m_held_item = nullptr;

    // test frame will have to own the mask grid
    std::shared_ptr<const Grid<bool>> m_mask_ptr;

    TextButton m_exit;
    bool m_request_exit = false;
};

namespace icon_fields {

// anything less opaque (alpha of smaller value) is not part of the mask
constexpr const int k_opacity_limit = 127;
constexpr const int k_expansion_size = 2;

constexpr const int k_icon_size = 32;

} // end of icon_fields namespace -> into <anonymous>

struct LoadIconsRt {
    SharedImagePtr icons;
    SharedImagePtr icon_shadows;
    std::shared_ptr<const Grid<bool>> mask_ptr;
    std::vector<sf::IntRect> icon_frames;
    std::vector<sf::IntRect> icon_shadows_frames;
};

LoadIconsRt load_icons(const std::string & filename, SfmlFlatEngine &,
                       int icon_size, int expansion_size,
                       int opacity_limit = icon_fields::k_opacity_limit);

} // end of <anonymous> namespace

int main() {

    SfmlFlatEngine engine;
    TestFrame test_frame;
    sf::RenderWindow win;

    engine.assign_target_and_states(win, sf::RenderStates::Default);
    engine.load_global_font("font.ttf");
    test_frame.setup(load_icons
        ("images/project-utumno-excerpt.png", engine,
         icon_fields::k_icon_size, icon_fields::k_expansion_size,
         icon_fields::k_opacity_limit));
    engine.stylize(test_frame);
    test_frame.check_for_geometry_updates();

    win.create(sf::VideoMode
        (std::max(200, test_frame.width ())
        ,std::max(200, test_frame.height()))
        /* title */ ,"");
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
        win.clear(sf::Color(40, 180, 40));
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
    case k_event_id_of<MousePress>: {
        auto pos = to_vector(event.as<MousePress>());
        m_foreground_image.mouse_click(pos.x, pos.y, m_location);
        }
        break;
    case k_event_id_of<MouseMove>: {
        auto pos = to_vector(event.as<MouseMove>());
        m_foreground_image.mouse_move(pos.x, pos.y);
        }
        break;
    case k_event_id_of<MouseRelease>:
        m_foreground_image.drag_release();
        break;
    default: break;
    }
}

VectorI DraggableImageWidget::location() const { return m_location; }

int DraggableImageWidget::width() const { return m_foreground_image.width(); }

int DraggableImageWidget::height() const { return m_foreground_image.height(); }

void DraggableImageWidget::stylize(const StyleMap &) {}

void DraggableImageWidget::update_geometry() {
    // not any real visible changes
    // probably not having the stale state problem
#   if 0
    int x = location().x, y = location().y;
    int w = width(), h = height();
    std::cout << "diw geo update " << x << ", " << y << " size "
              << w << ", " << h << std::endl;
#   endif
}

void DraggableImageWidget::draw(WidgetRenderer & target) const {
    if (m_foreground_image.is_requesting_drop()) {
        m_foreground_image.draw(location(), target);
    } else if (!m_foreground_image.is_requesting_drop())  {
        sf::IntRect bounds;
        set_top_left(bounds, m_location);
        bounds.width = m_foreground_image.width();
        bounds.height = m_foreground_image.height();
        draw_to(target, bounds, m_background_frame, m_background_image->item_key());
    }
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

void DraggableImageWidget::set_frames
    (const sf::IntRect & foreground, const sf::IntRect & background)
{
    m_foreground_image.set_frame(foreground);
    m_background_frame = background;
}

void DraggableImageWidget::set_mask(ConstSubGrid<bool> mask) {
    m_foreground_image.set_click_matrix(mask);
    flag_needs_whole_family_geometry_update();
}

/* private */ void DraggableImageWidget::set_location_(int x, int y) {
    m_location = VectorI(x, y);
}

/* private static */ const std::string DraggableImageWidget::k_no_file_filename = "";

// ----------------------------------------------------------------------------

void ImageTextSelectionEntry::setup(const SharedParams & shrparams, const IndividualParams & params) {
    // presently ignored parameter:
    // shared:
    // - show_full_text_on_hover

    m_full_text.set_string(params.string);
    m_full_text.set_limiting_line(shrparams.text_width);

    m_image.set_images(params.foreground, params.background);
    m_image.set_frames(params.foreground_tx_bounds, params.background_tx_bounds);
    m_image.set_mask(params.click_mask);
    m_image.set_scale(shrparams.scale);

    m_text.set_string(params.string);
    m_text.set_limiting_line(shrparams.text_width);
    m_text.set_fixed_height(shrparams.text_height);

    bool should_add_text = !m_text.string().empty();
    switch (shrparams.orientation) {
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

void TestFrame::setup(const LoadIconsRt & icon_info) {
    using SharedParams     = ImageTextSelectionEntry::SharedParams;
    using IndividualParams = ImageTextSelectionEntry::IndividualParams;

    SharedParams shared_params;
    shared_params.scale = 5;
    const auto & const_shared_params = shared_params;
    auto mk_icon = [&const_shared_params, &icon_info]
        (int frame, const UString & string)
    {
        IndividualParams params;
        params.background = icon_info.icon_shadows;
        params.foreground = icon_info.icons;
        params.string = string;
        params.foreground_tx_bounds = icon_info.icon_frames.at(frame);
        params.background_tx_bounds = icon_info.icon_shadows_frames.at(frame);
        params.click_mask = make_sub_grid
            (*icon_info.mask_ptr, get_top_left(params.foreground_tx_bounds)
            ,params.foreground_tx_bounds.width, params.foreground_tx_bounds.height);
        auto rv = std::make_unique<ImageTextSelectionEntry>();
        rv->setup(const_shared_params, params);
        return rv;
    };

    m_owned_widgets.emplace_back(mk_icon
        (7, U"Wand of Squadilla - Here's some more text to test the limits behavior of text."));
    m_owned_widgets.emplace_back(mk_icon
        (6, U"Wand of Squadilla - Here's some more text to test the limits behavior of text."));
    m_owned_widgets.emplace_back(mk_icon
        (5, U"Wand of Squadilla - Here's some more text to test the limits behavior of text."));
    m_owned_widgets.emplace_back(mk_icon
        (4, U"Wand of Squadilla - Here's some more text to test the limits behavior of text."));

    // not related to icons
    m_exit.set_string(U"Exit App");
    m_exit.set_press_event([this]() { m_request_exit = true; });

    // someone *needs* to own the entire click mask grid
    m_mask_ptr = icon_info.mask_ptr;

    setup_widgets();
}

template <typename Func>
inline void do_n_times(int n, Func && f) {
    for (int i = 0; i != n; ++i) f();
}

// NOT STATED IN DOCUMENTATION
// by "RGBA" most significant is the rightmost letter
static const sf::Color k_transparency(0x0);
static const auto k_shadow_pallete = {
    sf::Color(0xD0D0D0FF),
    sf::Color(0xFFFFFFFF),
    sf::Color(0xD0D0D0FF),

    k_transparency,
    k_transparency,

    sf::Color(0x202020FF),
    sf::Color(0x000000FF),
    sf::Color(0x202020FF)
};

static const auto k_neighbors =
    { VectorI(1, 0), VectorI(-1, 0), VectorI(0, 1), VectorI(0, -1) };

template <typename T, typename Predicate>
inline std::enable_if_t<std::is_invocable_v<Predicate, const T &>, bool>
    any_are_edge
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

template <typename T>
inline bool any_are_edge
    (const Grid<T> & grid, VectorI r, const T & inside_value)
{
    return any_are_edge(grid, r, [&inside_value](const T & obj) { return obj == inside_value; });
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

inline Grid<bool> expand_mask
    (const Grid<bool> & mask, std::vector<VectorI> edge_pixels_copy,
     std::size_t expansion_size = icon_fields::k_expansion_size,
     std::size_t reserve_offset = 0)
{
    auto temp_mask = mask;
    do_n_times(expansion_size, [&]() {
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

inline void mark_pixel(sf::Color & pixel, VectorI r) {
    int idx = (r.x + r.y % int(k_shadow_pallete.size())) % int(k_shadow_pallete.size());
    pixel = *(k_shadow_pallete.begin() + idx);
}

inline Grid<bool> make_icon_shadows(const Grid<bool> & shadow_mask, int icon_size, int expansion_size) {
    Grid<bool> icon_shadows;
    const int shadow_size = icon_size + expansion_size*2;
    icon_shadows.set_size((shadow_mask.width () / icon_size)*shadow_size,
                          (shadow_mask.height() / icon_size)*shadow_size,
                          false);
    for (VectorI icon_r; icon_r.y != shadow_mask.height() / icon_size; ++icon_r.y) {
    for (icon_r.x = 0  ; icon_r.x != shadow_mask.width () / icon_size; ++icon_r.x) {
        auto icon_tile   = make_sub_grid(shadow_mask , icon_r*icon_size, icon_size, icon_size);
        auto shadow_tile = make_sub_grid(icon_shadows, icon_r*shadow_size, shadow_size, shadow_size);
        // note the following:
        assert(icon_tile.width() + expansion_size*2 == shadow_tile.width());
        VectorI shadow_offset = VectorI(1, 1)*expansion_size;
        for (VectorI r; r != icon_tile.end_position(); r = icon_tile.next(r)) {
            if (!icon_tile(r)) continue;
            shadow_tile(r + shadow_offset) = true;
        }
    }}
    return icon_shadows;
}

inline Grid<bool> grow_shadow_mask(const Grid<bool> & shadow_mask, int expansion_size) {
    std::vector<VectorI> edge_pixels;
    // guess reserve
    edge_pixels.reserve( shadow_mask.size() / 4 );
    for (VectorI r; r != shadow_mask.end_position(); r = shadow_mask.next(r)) {
        if (!shadow_mask(r)) continue;
        if (any_are_edge(shadow_mask, r, true)) {
            edge_pixels.push_back(r);
        }
    }

    clean_up_edge_pixels(edge_pixels);
    return expand_mask(shadow_mask, edge_pixels, expansion_size,
                       std::max(shadow_mask.width(), shadow_mask.height())*4);
}

inline Grid<sf::Color> make_shadow_image(const Grid<bool> & shadow_mask, int icon_size, int expansion_size) {
    auto icon_shadows = make_icon_shadows(shadow_mask, icon_size, expansion_size);
    icon_shadows = grow_shadow_mask(icon_shadows, expansion_size);

    {
    sf::Image img;
    img.create(unsigned(icon_shadows.width()), unsigned(icon_shadows.height()));
    for (VectorI r; r != icon_shadows.end_position(); r = icon_shadows.next(r)) {
        img.setPixel(unsigned(r.x), unsigned(r.y), icon_shadows(r) ? sf::Color::Black : sf::Color::White);
    }
    img.saveToFile("/media/ramdisk/firstmask.png");
    }
    Grid<sf::Color> image;
    image.set_size(icon_shadows.width(), icon_shadows.height(), k_transparency);
    for (VectorI r; r != image.end_position(); r = image.next(r)) {
        if (!icon_shadows(r)) continue;
        mark_pixel(image(r), r);
    }
    return image;
}

inline std::vector<sf::IntRect> make_frames_for_size(int width_in_frames, int height_in_frames, int frame_size) {
    std::vector<sf::IntRect> rv;
    rv.reserve(width_in_frames*height_in_frames);
    for (VectorI icon_r; icon_r.y != height_in_frames; ++icon_r.y) {
    for (icon_r.x = 0  ; icon_r.x != width_in_frames ; ++icon_r.x) {
        rv.emplace_back(icon_r*frame_size, VectorI(1, 1)*frame_size);
    }}
    return rv;
}

LoadIconsRt load_icons
    (const std::string & filename, SfmlFlatEngine & engine, int icon_size, int expansion_size,
     int opacity_limit)
{
    auto image_grid = load_image(filename);

    Grid<bool> mask;
    // guess reserve
    mask.set_size(image_grid.width(), image_grid.height(), false);
    for (VectorI r; r != image_grid.end_position(); r = image_grid.next(r)) {
        if (image_grid(r).a < opacity_limit) continue;
        mask(r) = true;
    }

    // only do full icons
    // I want to do the smallest step at a time

    Grid<sf::Color> icon_shadows = make_shadow_image(mask, icon_size, expansion_size);
    int width_in_frames  = image_grid.width () / icon_size;
    int height_in_frames = image_grid.height() / icon_size;
    std::vector<sf::IntRect> icon_frames
        = make_frames_for_size(width_in_frames, height_in_frames, icon_size);
    std::vector<sf::IntRect> icon_shadow_frames = make_frames_for_size
        (width_in_frames, height_in_frames, icon_size + expansion_size*2);

    LoadIconsRt rv;
    rv.icon_frames = std::move(icon_frames);
    rv.icon_shadows_frames = std::move(icon_shadow_frames);
    rv.mask_ptr = std::make_shared<const Grid<bool>>( std::move(mask) );
    rv.icons = engine.make_image_from(image_grid);
    rv.icon_shadows = engine.make_image_from(icon_shadows);

    // temp
    sf::Image img;
    img.create(unsigned(icon_shadows.width()), unsigned(icon_shadows.height()));
    for (VectorI r; r != icon_shadows.end_position(); r = icon_shadows.next(r)) {
        img.setPixel(unsigned(r.x), unsigned(r.y), icon_shadows(r));
    }
    img.saveToFile("/media/ramdisk/shadows.png");

    return rv;
}

} // end of <anonymous> namespace
