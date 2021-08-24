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
#include <common/Util.hpp>
#include <common/SfmlVectorTraits.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Sleep.hpp>

#include <iostream>
#include <random>

#include <cassert>

#include "select-menu/select-menu-gen.hpp"
#include "select-menu/load-icons.hpp"

namespace {

using namespace cul::exceptions_abbr;
using cul::Tuple;
using cul::Grid;
using namespace asgl;

// ------------------------------ generalization ------------------------------
// some collapseing is in order these hierarchies are too tall imo

class DraggableIconWidget : public FocusWidget {
public:
    virtual const DrawableWithWidgetRenderer * get_hold_request() const = 0;
};

class IconTextSelectionEntry : public SelectionEntryFrame {
public:
    enum Orientation {
        k_image_top, k_image_bottom, k_image_left, k_image_right,
        k_orientation_count
    };

    void stylize(const StyleMap & map) final {
        TextArea::set_required_text_fields(
            m_full_text, map.find(styles::k_global_font),
            map.find(asgl::to_key(frame_styles::k_widget_text_style)),
            "TestFrame::stylize");
        BareFrame::stylize(map);
    }

    void process_event(const Event & event) final {
        BareFrame::process_event(event);
        if (!event.is_type<MouseMove>()) return;
        auto text_bounds = cul::compose(m_text.location(), m_text.size());
        if (is_contained_in(event.as<MouseMove>(), text_bounds)) {
            m_show_full_text = true;
            m_full_text.set_location(m_text.location());
        } else {
            m_show_full_text = false;
        }
    }

    void draw(WidgetRenderer & target) const final {
        if (m_show_full_text) {
            m_full_text.draw_to(target);
        } else {
            m_text.draw(target);
        }

        icon_widget().draw(target);
    }

    Tuple<const DrawableWithWidgetRenderer *, const Widget *> get_hold_request() const final
        { return std::make_tuple(icon_widget().get_hold_request(), &icon_widget()); }

    const UString & string() const { return m_text.string(); }

    void set_string(const UString & str) {
        auto temp = str;
        m_text.set_string(std::move(str));
    }

    void set_string(UString && str) {
        m_text.set_string(std::move(str));
        // setting string may change the text area's size
        flag_needs_whole_family_geometry_update();
    }

protected:
    void add_widgets(Orientation orientation);

    void add_widgets();

    virtual DraggableIconWidget & icon_widget() = 0;

    virtual const DraggableIconWidget & icon_widget() const = 0;

    void setup_text(const UString & ustr, int width, int height) {
        m_full_text.set_string(ustr);
        m_full_text.set_limiting_line(width);

        m_text.set_string(ustr);
        m_text.set_limiting_line(width);
        m_text.set_fixed_height(height);
    }

private:
#   if 0
    void process_focus_event(const Event &) final {}

    void notify_focus_gained() final {}

    void notify_focus_lost() final {}
#   endif
    TextArea m_text;
    bool m_show_full_text = false;
    Text m_full_text;

    Orientation m_selected_orientation = k_orientation_count;
};

// ------------------------------ specialization ------------------------------



// It's true I don't need an abstract class, but the attempt here is to limit
// the interface

// this class is *really* specific
class ForegroundDraggableImage final :
    public Draggable, public DrawableWithWidgetRenderer
{
public:
    void set_image(SharedImagePtr image)
        { m_foreground_image = image; }

    void set_click_matrix(ConstSubGrid<bool> mask)
        { m_click_mask = mask; }

    int width() const
        { return (m_click_mask.width() + 1)*scale(); }

    int height() const
        { return (m_click_mask.height() + 1)*scale(); }

    Size size() const { return Size(width(), height()); }

    using Draggable::mouse_move;

    void drag_release() {
        m_offset = Vector();
        Draggable::drag_release();
    }

    bool mouse_click(int x, int y, Vector matrix_location) {
        auto pos = Vector(x, y) - matrix_location;
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

    void set_frame(const Rectangle & rect)
        { m_tx_bounds = rect; }

    bool is_requesting_drop() const final
        { return !is_being_dragged(); }

private:
    void draw_(const Vector & offset, WidgetRenderer & renderer) const final {
        renderer.render_rectangle_pair(bounds(offset), m_tx_bounds, m_foreground_image->item_key(), this);
    }

    void update_drag_position(int x, int y) final {
        m_offset = Vector(x, y);
    }

    Rectangle bounds(Vector offset_) const {
        return Rectangle(offset_.x + m_offset.x, offset_.y + m_offset.y,
                           width(), height());
    }

    Vector m_offset;
    ConstSubGrid<bool> m_click_mask;
    SharedImagePtr m_foreground_image;
    Rectangle m_tx_bounds;
    int m_scale = 4;
};

// I want to test this first!
// Let's get the simplest parts finished first!
class DraggableImageWidget final : public DraggableIconWidget {
public:
    struct Parameters {
        SharedImagePtr foreground;
        SharedImagePtr background;

        Rectangle foreground_tx_bounds;
        Rectangle background_tx_bounds;

        ConstSubGrid<bool> click_mask;

        //Size resolution;
    };

    void process_event(const Event & event) override {
        switch (event.type_id()) {
        case k_event_id_of<MousePress>: {
            auto pos = event.as<MousePress>();
            m_foreground_image.mouse_click(pos.x, pos.y, location());
            }
            break;
        case k_event_id_of<MouseMove>: {
            auto pos = event.as<MouseMove>();
            m_foreground_image.mouse_move(pos.x, pos.y);
            }
            break;
        case k_event_id_of<MouseRelease>:
            m_foreground_image.drag_release();
            break;
        default: break;
        }
    }

    Size size() const override { return m_foreground_image.size(); }

    void draw(WidgetRenderer &) const override;

    // these non-virtual methods called by factory

    void set_parameters(const Parameters & params) {
        set_images(params.foreground, params.background);
        set_frames(params.foreground_tx_bounds, params.background_tx_bounds);
        set_mask(params.click_mask);
    }

    void set_images(SharedImagePtr foreground_image,
                    SharedImagePtr background_image);

    void set_frames(const Rectangle & foreground,
                    const Rectangle & background);

    void set_mask(ConstSubGrid<bool>);

    void set_scale(int scale)
        { m_foreground_image.set_scale(scale); }

    // move up (later), and de-virtualize
    // we hardly need to know anything about what's being dragged
    const DrawableWithWidgetRenderer * get_hold_request() const final {
        if (m_foreground_image.is_requesting_drop()) return nullptr;
        return &m_foreground_image;
    }

    void process_focus_event(const Event &) override {}

    Vector location() const override { return m_location; }

private:
    void stylize(const StyleMap &) override {}

    void update_size() override {}

    void notify_focus_gained() override {}

    void notify_focus_lost() override {}

    void set_location_(int x, int y) override;

    ForegroundDraggableImage m_foreground_image;

    SharedImagePtr m_background_image;
    Rectangle m_background_frame;

    Vector m_location;
};

// often I want some extra data to tag along with this class
class ImageTextSelectionEntry : public IconTextSelectionEntry {
public:
    struct SharedParams {
        Orientation orientation = k_image_top;
        int scale = 1;
        int text_width  = 180;
        int text_height = 60 ;
        bool show_full_text_on_hover = true;
    };

    struct IndividualParams : public DraggableImageWidget::Parameters {
        // how do I do multiple frames for a single image without going
        // overboard here?
        UString string;
    };

protected:
    void setup(const SharedParams &, const IndividualParams &);

    ImageTextSelectionEntry() {}

private:
    DraggableImageWidget m_image;

    DraggableIconWidget & icon_widget() final { return m_image; }

    const DraggableIconWidget & icon_widget() const final { return m_image; }
};

// --------------------------------- testing ----------------------------------

struct ItemInfo;
struct GameItem {
    const ItemInfo * id = nullptr;
    int quantity = 0;
};

struct ItemInfo {
    int max_stack =  1;
    int icon_id   = -1;
    UString name;
};

enum {
    k_utumno_helment,
    k_blue_armor,
    k_earth_book,
    k_fancy_book,
    k_cyan_crystal_sphere,
    k_poison_flask,
    k_topaz_ring,
    k_wand_of_squadilla
};

const std::vector<ItemInfo> & item_db() {
    static std::vector<ItemInfo> item_db_;
    if (!item_db_.empty()) return item_db_;
    int icon_num = 0;
    item_db_ = {
        {  1, icon_num++, U"Utumno Helment"      },
        {  1, icon_num++, U"Blue Armor"          },
        {  3, icon_num++, U"Earth Book"          },
        {  3, icon_num++, U"Fancy Book"          },
        {  1, icon_num++, U"Cyan Crystal Sphere" },
        {  5, icon_num++, U"Poison Flask"        },
        { 10, icon_num++, U"Topaz Ring"          },
        {  1, icon_num++, U"Wand of Squadilla"   }
    };
    return item_db_;
}

int item_to_index(const ItemInfo * pointer) {
    if (!pointer) return -1;
    auto rv = pointer - &item_db().front();
    assert(rv > -1 && std::size_t(rv) < item_db().size());
    return rv;
}

class ItemIconEntry final : public ImageTextSelectionEntry {
public:
    using SharedParams = ImageTextSelectionEntry::SharedParams;
    void load_item
        (const LoadIconsRt & icon_info, const GameItem & item, const SharedParams &);

    ItemIconEntry() {}

    ItemIconEntry(const ItemIconEntry & rhs):
        ImageTextSelectionEntry(rhs),
        m_item(rhs.m_item)
    { add_widgets(); }

    ItemIconEntry(ItemIconEntry && rhs):
        ImageTextSelectionEntry(std::move(static_cast<ImageTextSelectionEntry &&>(rhs))),
        m_item(rhs.m_item)
    { add_widgets(); }

    ItemIconEntry & operator = (const ItemIconEntry & rhs) = default;

    ItemIconEntry & operator = (ItemIconEntry && rhs) = default;

    bool can_be_stacked_onto(const ItemIconEntry & other) const {
        if (m_item.id != other.m_item.id) return false;
        if (!m_item.id || !other.m_item.id) {
            throw std::runtime_error("Item ids not set.");
        }
        // if there's room for even one item in the stack, then we say it can
        // be stacked
        return other.m_item.quantity + 1 <= other.m_item.id->max_stack;
    }

    // returns any left over items
    int stack_onto(ItemIconEntry & other) {
        if (!can_be_stacked_onto(other)) return m_item.quantity;
        int max_stack = other.m_item.id->max_stack;
        int in_total = other.m_item.quantity + m_item.quantity;
        other.m_item.quantity = std::min(max_stack, in_total);
        m_item.quantity = in_total - other.m_item.quantity;
        assert(m_item.quantity > -1);

        auto old_size = size(), other_old_size = other.size();
        set_string(game_item_string(m_item));
        other.set_string(game_item_string(other.m_item));
        assert(old_size == size());
        assert(other_old_size == other.size());
        return m_item.quantity;
    }

private:
    static UString game_item_string(const GameItem & item) {
        UString rv;
        rv.reserve(30);
        set_game_item_string(rv, item);
        return rv;
    }
    static void set_game_item_string(UString & string, const GameItem & item) {
        if (item.quantity == 0) {
            string = U"<depleted>";
            return;
        }
        string = item.id->name;
        if (item.quantity > 1) {
            string += U" x";
            auto s = std::to_string(item.quantity);
            for (auto c : s) {
                string += UChar(c);
            }
        }
    }

    GameItem m_item;
};

class ItemMenu final : public SelectionMenu<ItemIconEntry> {
public:
    Tuple<ItemIconEntry *, ItemIconEntry *>
        on_place_item_on(ItemIconEntry & a, ItemIconEntry * b) final;

    // we'd want a "use" item, "drop" item, some "uses" which further populate
    // the grid

    void setup(const LoadIconsRt & icon_info, const std::vector<GameItem> & items);

    void update_inventory(const std::vector<GameItem> &);

    const std::vector<GameItem> & inventory() const { return m_inventory; }

private:
    std::vector<GameItem> m_inventory;
    std::vector<ItemIconEntry> m_owned_icons;

    std::shared_ptr<const Grid<bool>> m_mask_ptr;
};

class TestFrameN final : public Frame {
public:
    bool is_requesting_exit() const { return m_request_exit; }

    // this setup function should be relatively stripped down
    void setup(const LoadIconsRt & icon_info);

private:
    void setup_widgets();

    using IconTilePtr = std::unique_ptr<ImageTextSelectionEntry>;
    std::vector<IconTilePtr> m_owned_widgets;

    // test frame will have to own the mask grid
    std::shared_ptr<const Grid<bool>> m_mask_ptr;

    TextButton m_arrange_alpha;
    TextButton m_exit;
    bool m_request_exit = false;

    ItemMenu m_menu;
};

struct TestWidgetDummy {};

class TestFocusReceiver : public FocusReceiver {
    void process_focus_event(const Event &) final {}

    void notify_focus_gained() final {}

    void notify_focus_lost() final {}
};

template <bool kt_is_focus_widget>
class TestWidgetImpl final :
    public Widget, public std::conditional_t<kt_is_focus_widget, TestFocusReceiver, TestWidgetDummy>
{
public:
    void process_event(const Event &) final {}

    void stylize(const StyleMap &) final {}

    void set_size(int w_, int h_) { set_size_of(m_bounds, w_, h_); }

    void set_size(Size sz_) { set_size_of(m_bounds, sz_); }

    Vector location() const final { return top_left_of(m_bounds); }

    Size size() const final { return size_of(m_bounds); }

    void draw(WidgetRenderer &) const final {}

private:
    void update_size() final {}

    void set_location_(int x, int y) final { set_top_left_of(m_bounds, x, y); }
    Rectangle m_bounds;
};

using TestWidget      = TestWidgetImpl<false>;
using TestFocusWidget = TestWidgetImpl<true >;

class QuadraDirectionalFocusHandler final {
public:
    void process_event(const Event & event) {
        auto * gm = event.as_pointer<GeneralMotion>();
        if (!gm) return;
        auto dir_chosen = to_focus_handler_direction(*gm);
        if (dir_chosen == k_dir_count) return;
    }

    void check_for_child_widget_updates(Widget & root) {
        m_widgets.clear();
        root.iterate_children_f([this](Widget & widget) {
            auto frec = dynamic_cast<FocusReceiver *>(&widget);
            if (!frec) return;
            m_widgets.push_back(JumpEntry(&widget, frec));
        });
        for (auto & entry : m_widgets) {
            for (auto & iter : entry.jump_table)
                iter = m_widgets.end();
        }
    }

private:
    struct JumpEntry;
    using FocusContainer = std::vector<JumpEntry>;
    using FocusIterator  = FocusContainer::iterator;
    enum Direction { k_up, k_down, k_right, k_left, k_dir_count };
    struct JumpEntry {
        JumpEntry() {}
        JumpEntry(Widget * wid_, FocusReceiver * frec_):
            as_widget(wid_), as_receiver(frec_)
        {}
        std::array<FocusIterator, k_dir_count> jump_table;
        Widget * as_widget = nullptr;
        FocusReceiver * as_receiver = nullptr;
    };

    static Direction to_focus_handler_direction(GeneralMotion gm) {
        switch (gm) {
        case general_motion::k_motion_focus_down : return k_down ;
        case general_motion::k_motion_focus_up   : return k_up   ;
        case general_motion::k_motion_focus_right: return k_right;
        case general_motion::k_motion_focus_left : return k_left ;
        default: break;
        }
        return k_dir_count;
    }

    static void do_links(FocusIterator beg, FocusIterator end) {
        for (int half_lengths : { 2, 3, 5, k_extend_to_inf }) {
            for (auto itr = beg; itr != end; ++itr) {
            for (auto jtr = beg; jtr != itr; ++jtr) {
                do_pair(itr, jtr, half_lengths, end);
            }}
        }
    }

    static void do_pair(FocusIterator lhs, FocusIterator rhs, int lhs_half_lengths, FocusIterator end) {
        auto rhs_bounds = rhs->as_widget->bounds();
        auto lhs_bounds = lhs->as_widget->bounds();
        for (auto side : { k_up, k_down, k_left, k_right }) {
            auto directional_rect = get_directional_rectangle(lhs_bounds, lhs_half_lengths, side);
            if (!intersects(directional_rect, rhs_bounds)) continue;
            // test if rhs is closer than the currently selected rectangle
            auto & itr = lhs->jump_table[side];
            if (itr == end) {
                itr = rhs;
                continue;
            }
        }
    }

    static int distance_to(Vector source, Rectangle rect, Direction side) {

    }

    static constexpr const int k_extend_to_inf = -1;
    static Rectangle extend_by_half_lengths(Rectangle rect, int half_lengths) {
        if (half_lengths == k_extend_to_inf) {
            static const constexpr int k_max_int = std::numeric_limits<int>::max();
            // it's not actually possible for an integer rectangle to cover the
            // entire 2d integer plane, so make it as big as possible...
            static const Rectangle k_extreme_bounds
                { k_max_int / 2, k_max_int / 2, k_max_int, k_max_int };
            return k_extreme_bounds;
        }
        auto hextension = (rect.width*half_lengths) / 2;
        auto vextension = (rect.width*half_lengths) / 2;
        rect.left   -= hextension / 2;
        rect.top    -= vextension / 2;
        rect.width  += hextension;
        rect.height += vextension;
        return rect;
    }

    static Rectangle get_directional_rectangle(const Rectangle & rect, int half_lengths, Direction dir) {
        using IntLims = std::numeric_limits<int>;
        auto center = cul::center_of(rect);
        auto extended_rect = extend_by_half_lengths(rect, half_lengths);
        // reuse
        auto & rv = extended_rect;
        switch (dir) {
        case k_down : return Rectangle(extended_rect.left, center.y, extended_rect.width, IntLims::max());
        case k_right: return Rectangle(center.x, extended_rect.top, IntLims::max(), extended_rect.height);
        case k_left :
            // need to be careful not to underflow!
            // min value + max value == -1
            rv.left  = center.x >= 0 ? center.x - IntLims::max() : IntLims::min();
            rv.width = center.x >= 0 ? IntLims::max() : center.x - IntLims::min();
            assert(cul::right_of (rv) == center.x);
            assert(rv.left  < 0);
            assert(rv.width > 0);
            break;
        case k_up   :
            rv.top    = center.y >= 0 ? center.y - IntLims::max() : IntLims::min();
            rv.height = center.y >= 0 ? IntLims::max() : center.y - IntLims::min();
            assert(cul::bottom_of(rv) == center.y);
            assert(rv.top    < 0);
            assert(rv.height > 0);
            break;
        default: throw InvArg("get_directional_rectangle: provided direction "
                              "must be up, down, left, or right.");
        }
        return rv;
    }

    static std::array<Vector, 4> get_extremes(const Rectangle & rect) {
        return { Vector(rect.left             , rect.top              ),
                 Vector(rect.left             , rect.top + rect.height),
                 Vector(rect.left + rect.width, rect.top              ),
                 Vector(rect.left + rect.width, rect.top + rect.height)
        };
    }

    static bool intersects(const Rectangle & lhs, const Rectangle & rhs) {
        using cul::is_contained_in;
        for (auto r : get_extremes(lhs)) {
            if (is_contained_in(r, rhs)) return true;
        }

        for (auto r : get_extremes(rhs)) {
            if (is_contained_in(r, lhs)) return true;
        }
        return false;
    }

    FocusContainer m_widgets;
};
#if 0
struct PruneAndSweepWorkspace;
class PruneAndSweepAdapter {
public:
    // should be experimentally determined?
    static constexpr const int k_do_quadratic_thershold = 10;

    using WorkspacePointer = std::unique_ptr<PruneAndSweepWorkspace>;
    virtual ~PruneAndSweepAdapter() {}
    // check if overlappers intersect/contain any subjects
    // a seperate implementation may be wanted for real numbers rather than
    // just the integers

    virtual Rectangle get_overlapper(int) const = 0;
    virtual Rectangle get_subject(int) const = 0;
    virtual int overlapper_count() const = 0;
    virtual int subject_count() const = 0;
    virtual void operator () (int overlapper_num, int subject_number) const = 0;


    virtual WorkspacePointer give_workspace();
    virtual void take_workspace(WorkspacePointer &&) {}
};

struct PruneAndSweepWorkspace {

};

std::unique_ptr<PruneAndSweepWorkspace> PruneAndSweepAdapter::give_workspace()
    { return std::make_unique<PruneAndSweepWorkspace>(); }

void do_prune_and_sweep(PruneAndSweepAdapter & adapter) {

}
#endif
namespace icon_fields {

// anything less opaque (alpha of smaller value) is not part of the mask
constexpr const int k_opacity_limit = 127;
constexpr const int k_expansion_size = 2;

constexpr const int k_icon_size = 32;

} // end of icon_fields namespace -> into <anonymous>

} // end of <anonymous> namespace

int main() {

    QuadraDirectionalFocusHandler qdfh;
    SfmlFlatEngine engine;
    TestFrameN test_frame;
    sf::RenderWindow win;

    engine.load_global_font("font.ttf");
    test_frame.setup(load_icons
        ("images/project-utumno-excerpt.png", engine,
         icon_fields::k_icon_size, icon_fields::k_expansion_size,
         icon_fields::k_opacity_limit));
    int focus_widget_count = 0;
    test_frame.iterate_children_const_f([&focus_widget_count](const Widget & widget) {
        std::cout << static_cast<const void *>(&widget);
        if (dynamic_cast<const IconTextSelectionEntry *>(&widget)) {
            std::cout << " is an icon text selection entry";
        }
        if (dynamic_cast<const FocusReceiver *>(&widget)) {
            std::cout << " is a focus receiver";
            ++focus_widget_count;
        }
        std::cout << std::endl;
    });
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
        engine.draw(test_frame, win);
#       if 0
        std::default_random_engine rng { 0x827ABE21 };
        test_frame.iterate_children_const_f([&rng, &win](const Widget & widget) {
            if (!dynamic_cast<const asgl::BareFrame *>(&widget)) return;
            auto rnd_u8 = [&rng]()
                { return uint8_t(std::uniform_int_distribution<int>(0, 255)(rng)); };
            auto rnd_color = [&rnd_u8]()
                { return sf::Color(rnd_u8(), rnd_u8(), rnd_u8(), 140); };

            auto bounds = widget.bounds();
            win.draw(DrawRectangle(float(bounds.left), float(bounds.top)
                                  ,float(bounds.width), float(bounds.height)
                                  ,rnd_color()));
        });
#       endif

        win.display();
    }
    return 0;
}

namespace {

// ------------------------------ generalization ------------------------------

// ------------------------------ specialization ------------------------------

void DraggableImageWidget::draw(WidgetRenderer & target) const {
    if (!m_foreground_image.is_requesting_drop() || has_focus())  {
        Rectangle bounds;
        set_top_left_of(bounds, location());
        bounds.width = m_foreground_image.width();
        bounds.height = m_foreground_image.height();
        draw_to(target, bounds, m_background_frame, m_background_image->item_key());
    }
    if (m_foreground_image.is_requesting_drop()) {
        m_foreground_image.draw(location(), target);
    }
}

void DraggableImageWidget::set_images
    (SharedImagePtr foreground_image, SharedImagePtr background_image)
{
    m_foreground_image.set_image(foreground_image);
    m_background_image = background_image;
}

void DraggableImageWidget::set_frames
    (const Rectangle & foreground, const Rectangle & background)
{
    m_foreground_image.set_frame(foreground);
    m_background_frame = background;
}

void DraggableImageWidget::set_mask(ConstSubGrid<bool> mask) {
    m_foreground_image.set_click_matrix(mask);
    flag_needs_whole_family_geometry_update();
}

/* private */ void DraggableImageWidget::set_location_(int x, int y) {
    m_location = Vector(x, y);
}

// ----------------------------------------------------------------------------

void ImageTextSelectionEntry::setup(const SharedParams & shrparams, const IndividualParams & params) {
    // presently ignored parameter:
    // shared:
    // - show_full_text_on_hover

    setup_text(params.string, shrparams.text_width, shrparams.text_height);

    m_image.set_parameters(params);
    m_image.set_scale(shrparams.scale);

    add_widgets(shrparams.orientation);
}

// ----------------------------------------------------------------------------

/* protected */ void IconTextSelectionEntry::add_widgets(Orientation orientation) {
    m_selected_orientation = orientation;

    add_widgets();
}

/* protected */ void IconTextSelectionEntry::add_widgets() {
    bool should_add_text = !m_text.string().empty();
    switch (m_selected_orientation) {
    case k_image_bottom: {
        auto adder = begin_adding_widgets();
        if (should_add_text) {
            adder.add_horizontal_spacer().add(m_text).add_horizontal_spacer()
                 .add_line_seperator();
        }
        adder.add(icon_widget());
        }
        break;
    case k_image_top: {
        auto adder = begin_adding_widgets();
        adder.add(icon_widget());
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
    case k_orientation_count: break;
    }
}

// --------------------------------- testing ----------------------------------

void ItemIconEntry::load_item
    (const LoadIconsRt & icon_info, const GameItem & item,
     const SharedParams & params)
{
    IndividualParams individ_params;
    individ_params.background = icon_info.icon_shadows;
    individ_params.foreground = icon_info.icons;
    set_game_item_string(individ_params.string, item);
    individ_params.foreground_tx_bounds = icon_info.icon_frames.at(item.id->icon_id);
    individ_params.background_tx_bounds = icon_info.icon_shadows_frames.at(item.id->icon_id);
    individ_params.click_mask = make_sub_grid
        (*icon_info.mask_ptr, individ_params.foreground_tx_bounds);
    setup(params, individ_params);
    m_item = item;
}

// ----------------------------------------------------------------------------

Tuple<ItemIconEntry *, ItemIconEntry *>
    ItemMenu::on_place_item_on(ItemIconEntry & a, ItemIconEntry * b)
{
    auto swapped_icons = std::make_tuple(b, &a);
    if (!b) return swapped_icons;
    if (!a.can_be_stacked_onto(*b)) return swapped_icons;
    if (a.stack_onto(*b) == 0) {
        return std::make_tuple(nullptr, b);
    }
    return std::make_tuple(&a, b);
}

void ItemMenu::setup
    (const LoadIconsRt & icon_info, const std::vector<GameItem> & items)
{
    set_menu_size(4, 4);

    using SharedParams = ImageTextSelectionEntry::SharedParams;
    SharedParams params;
    params.scale = 5;
#   if 0
    m_owned_icons.reserve(items.size());
#   endif
    for (auto & item : items) {
        if (!item.id || item.quantity == 0) continue;
        m_owned_icons.emplace_back();
        m_owned_icons.back().load_item(icon_info, item, params);
    }
    Vector place_pos;
    auto itr = m_owned_icons.begin();
    for (; itr != m_owned_icons.end(); place_pos = next(place_pos), ++itr) {
        assert(place_pos != end_position());
        place(place_pos, &*itr);
    }
    // mask needs to live somewhere
    m_mask_ptr = icon_info.mask_ptr;
}

void ItemMenu::update_inventory(const std::vector<GameItem> & items) {

}

// ----------------------------------------------------------------------------

void TestFrameN::setup(const LoadIconsRt & icon_info) {
    m_menu.setup(icon_info, {
        { &item_db()[k_blue_armor         ], 1 },
        { &item_db()[k_topaz_ring         ], 2 },
        { &item_db()[k_topaz_ring         ], 1 },
        { &item_db()[k_poison_flask       ], 1 },
        { &item_db()[k_cyan_crystal_sphere], 1 },
        { &item_db()[k_poison_flask       ], 1 },
        { &item_db()[k_wand_of_squadilla  ], 1 },
    });
    // not related to icons
    m_exit.set_string(U"Exit App");
    m_exit.set_press_event([this]() { m_request_exit = true; });

    m_arrange_alpha.set_string(U"Arrange Alphabetically");

    m_arrange_alpha.set_press_event([this]() {
        std::sort(m_menu.begin(), m_menu.end(),
            [](const ImageTextSelectionEntry * lhs, const ImageTextSelectionEntry * rhs)
        {
            // nullptr treated as "\0xFFFFFFFF\0xFFFF..."
            if (!lhs) return false;
            if (!rhs) return true ;
            return lhs->string() < rhs->string();
        });
        m_menu.fix_internal_iterators();
    });

    // someone *needs* to own the entire click mask grid
    m_mask_ptr = icon_info.mask_ptr;

    set_border_padding(5);
    m_menu.set_padding(5);
    setup_widgets();
}

void TestFrameN::setup_widgets() {
    begin_adding_widgets()
        .add(m_menu).add_line_seperator()
        .add(m_arrange_alpha).add_horizontal_spacer().add(m_exit);
}

} // end of <anonymous> namespace
