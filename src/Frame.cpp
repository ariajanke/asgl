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

#include <common/Util.hpp>

#include <asgl/Frame.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <stdexcept>
#if 0
#include <iostream>
#endif
#include <cassert>

namespace {

using VectorI = asgl::BareFrame::VectorI;
using WidgetPlacementVector = asgl::BareFrame::WidgetPlacementVector;
using Widget = asgl::Widget;
using asgl::detail::HorizontalSpacer;
using asgl::detail::LineSeperator;

class WidgetPlacerAdapter {
public:
    virtual ~WidgetPlacerAdapter() {}
    // should include *all* widgets
    virtual void on_line_end(int widgets_covered, int ns_width) = 0;
    virtual void operator () (Widget *, int x, int y) const = 0;
    virtual bool is_line_seperator(const Widget &) const = 0;
    virtual bool is_horizontal_spacer(const Widget &) const = 0;

    virtual void debug_buff_pad() {}
    virtual void debug_on_new_line() {}
};

class PartialWidgetPlacerAdapter : public WidgetPlacerAdapter {
public:
    bool is_line_seperator(const Widget & widget) const final;

    bool is_horizontal_spacer(const Widget & widget) const final;

    void assign_line_seperator(const LineSeperator & ls)
        { m_line_seperator = &ls; }

    void assign_horizontal_spacers_vector(const std::vector<HorizontalSpacer> & vec)
        { m_spacers_vector = &vec; }

protected:
    auto horizontal_spacers_begin() { return m_spacers_vector->cbegin(); }
    auto horizontal_spacers_end  () { return m_spacers_vector->cend  (); }

private:
    const LineSeperator * m_line_seperator = nullptr;
    const std::vector<HorizontalSpacer> * m_spacers_vector = nullptr;
};

class HorizontalSpacerUpdater final : public PartialWidgetPlacerAdapter {
public:
    using WidgetConstIter = std::vector<Widget *>::const_iterator;
    using HorzIter        = std::vector<HorizontalSpacer>::iterator;

    ~HorizontalSpacerUpdater() final;

    void on_line_end(int widgets_covered, int ns_width) final;

    void assign_horizontal_spacers_vector
        (std::vector<asgl::detail::HorizontalSpacer> & vec);

    void set_widget_iterators(const std::vector<Widget *> & widgets);

    void set_horizontal_space(int sp) { m_horz_space = sp; }

    static bool point_to_same(WidgetConstIter widitr, HorzIter horzitr)
        { return (*widitr) == &*horzitr; }

    void operator () (Widget *, int, int) const final {}

private:
    WidgetConstIter m_widget_itr;
    WidgetConstIter m_widget_end;
    int m_horz_space = 0;
    HorzIter m_horz_itr;
};

void do_placements(WidgetPlacementVector &);

inline bool is_zero_sized(const Widget & widget)
    { return widget.width() == 0 && widget.height() == 0; }

void run_placer_algo
    (WidgetPlacerAdapter & placer, const std::vector<Widget *> & widgets,
     int width_for_widgets, int padding_between_widgets);

} // end of <anonymous> namespace

namespace asgl {

WidgetAdder::WidgetAdder
    (BareFrame * frame_, detail::LineSeperator * sep_):
    m_the_line_sep(sep_),
    m_parent(frame_)
{
    static constexpr const char * k_null_parent =
        "WidgetAdder::WidgetAdder: [library error] Parent must not be null, "
        "and line seperator must refer to something.";
    if (!m_parent || !m_the_line_sep) {
        throw std::invalid_argument(k_null_parent);
    }
}

WidgetAdder::WidgetAdder(WidgetAdder && rhs)
    { swap(rhs); }

WidgetAdder::~WidgetAdder() noexcept(false) {
    if (std::uncaught_exceptions() > 0) return;
    if (!m_parent) return;
    m_parent->finalize_widgets(
        std::move(m_widgets), std::move(m_horz_spacers),
        m_the_line_sep);
}

WidgetAdder & WidgetAdder::operator = (WidgetAdder && rhs) {
    swap(rhs);
    return *this;
}

WidgetAdder & WidgetAdder::add(Widget & widget) {
    // we check later whether or not this frame is trying to include itself
    // see Frame::finalize_widgets(
    // std::vector<Widget *> &&, std::vector<detail::HorizontalSpacer> &&,
    // detail::LineSeperator *, const StyleMap *)
    m_widgets.push_back(&widget);
    return *this;
}

WidgetAdder & WidgetAdder::add_horizontal_spacer() {
    // A little word on the C++ standard:
    // According to: http://stackoverflow.com/questions/5410035/when-does-a-stdvector-reallocate-its-memory-array
    // From C++ standard 23.2.4.2
    // <quote>
    // It is guaranteed that no reallocation takes place during insertions that
    // happen after a call to reserve() until the time when an insertion would
    // make the size of the vector greater than the size specified in the most
    // recent call to reserve().
    // </quote>

    // update and revalidate all pointers to spacers as necessary
    using namespace detail;
    if (   m_horz_spacers.size() == m_horz_spacers.capacity()
        && !m_horz_spacers.empty())
    {
        std::vector<HorizontalSpacer> new_spacer_cont;
        // reserve and copy
        new_spacer_cont.reserve(1 + m_horz_spacers.capacity()*2);
        (void)std::copy(m_horz_spacers.begin(), m_horz_spacers.end(),
                        std::back_inserter(new_spacer_cont));
        // revalidate pointers in the widget container
        HorizontalSpacer * new_itr = &new_spacer_cont[0];
        const HorizontalSpacer * old_itr = &m_horz_spacers[0];
        for (Widget *& widget_ptr : m_widgets) {
            if (widget_ptr == old_itr) {
                widget_ptr = new_itr++;
                ++old_itr;
            }
        }
        // safely get the end() pointer
        assert(new_itr == (new_spacer_cont.data() + new_spacer_cont.size()));
        m_horz_spacers.swap(new_spacer_cont);
    }
    m_horz_spacers.push_back(HorizontalSpacer());
    m_widgets.push_back(&m_horz_spacers.back());

    return *this;
}

WidgetAdder & WidgetAdder::add_line_seperator() {
    m_widgets.push_back(m_the_line_sep);
    return *this;
}

void WidgetAdder::swap(WidgetAdder & rhs) {
    m_widgets.swap(rhs.m_widgets);
    m_horz_spacers.swap(rhs.m_horz_spacers);
    std::swap(m_the_line_sep, rhs.m_the_line_sep);
    std::swap(m_parent, rhs.m_parent);
}

// ----------------------------------------------------------------------------

/* protected */ BareFrame::BareFrame() {
    // constructor needs to be very careful, many methods rely on decoration()
    // which at this point, that further derived "shell" of the instance hasn't
    // been instantiated yet
    check_invarients();
}

/* protected */ BareFrame::BareFrame(const BareFrame & lhs):
    m_padding(lhs.m_padding)
{}

/* protected */ BareFrame::BareFrame(BareFrame && lhs)
    { swap(lhs); }

/* protected */ BareFrame::~BareFrame() {}

/* protected */ void BareFrame::set_location_(int x, int y) {
    VectorI delta = VectorI(x, y) - location();

    auto & deco = decoration();
    deco.set_location(x, y);
    deco.update_geometry();

#   if 1
    // seems to work beyond an initial "wobble"
    for (auto * widget_ptr : m_widgets) {
        assert(widget_ptr);
        auto loc = widget_ptr->location();
        widget_ptr->set_location(loc.x + delta.x, loc.y + delta.y);
    }
    m_widget_extremes = BareFrame::get_measurements(m_widget_placements);
    m_widget_placements.clear();
#   endif
    check_invarients();
}

void BareFrame::process_event(const Event & event) {
    auto gv = decoration().process_event(event);
    if (!gv.skip_other_events) {
        for (Widget * widget_ptr : m_widgets) {
            widget_ptr->process_event(event);
        }
        // perhaps I should process focus requests after the fact to give
        // widgets the opportunity to make a request after an event
        m_focus_handler.process_event(event);
    }
    check_invarients();
}

void BareFrame::stylize(const StyleMap & smap) {
    decoration().stylize(smap);
    m_padding = Helpers::verify_padding(
        smap.find(styles::k_global_padding), "Frame::stylize");

    for (Widget * widget_ptr : m_widgets)
        widget_ptr->stylize(smap);

    check_invarients();
}

VectorI BareFrame::location() const {
    auto deco_loc = decoration().location();
    auto ex_loc = m_widget_extremes.recorded_location();
    return VectorI(std::min(deco_loc.x, ex_loc.x), std::min(deco_loc.y, ex_loc.y));
}

int BareFrame::width() const
    { return std::max(decoration().width(), m_widget_extremes.recorded_width() + padding()*2); }

int BareFrame::height() const
    { return std::max(decoration().height(), m_widget_extremes.recorded_height() + padding()*2); }

WidgetAdder BareFrame::begin_adding_widgets()
    { return WidgetAdder(this, &m_the_line_seperator); }

void BareFrame::finalize_widgets(std::vector<Widget *> && widgets,
    std::vector<detail::HorizontalSpacer> && spacers,
    detail::LineSeperator * the_line_sep)
{
    static constexpr const char * k_must_know_line_sep =
        "Frame::finalize_widgets: caller must know the line seperator to call "
        "this function. This is meant to be called by a Widget Adder only.";
    if (the_line_sep != &m_the_line_seperator) {
        throw std::invalid_argument(k_must_know_line_sep);
    }

    static constexpr const char * k_cannot_contain_this =
        "Frame::finalize_widgets: This frame may not contain itself.";
    for (auto * widget : widgets) {
        if (auto * frame_widget = dynamic_cast<BareFrame *>(widget)) {
            if (frame_widget->contains(this)) {
                throw std::invalid_argument(k_cannot_contain_this);
            }
        }
    }

    m_widgets     .swap(widgets);
    m_horz_spacers.swap(spacers);

    // note this marks this instance as a widget which "owns itself"
    assign_flags_receiver(this);
    iterate_children_f([this](Widget & widget) {
        widget.assign_flags_receiver(this);
    });

    flag_needs_whole_family_geometry_update();
    check_invarients();
}

void BareFrame::set_register_click_event(ClickFunctor && f)
    { decoration().set_click_inside_event(std::move(f)); }

void BareFrame::reset_register_click_event()
    { decoration().set_click_inside_event([]() { return FrameDecoration::ClickResponse(); }); }

void BareFrame::set_padding(int pixels)
    { m_padding = pixels; }

void BareFrame::check_for_geometry_updates() {
    if (needs_whole_family_geometry_update()) {
        update_size();
        // v we don't need this below on a regular geometry update v
        Widget & as_widget = *this;
        // sadly I can't reveal (all of) the children of this frame without
        // passing this, or incurring a dynamic allocation cost (micro optimizing?)

        // I can try a give and take sort of deal if I *really* want to, that could
        // avoid a use of "this"
        m_focus_handler.check_for_child_widget_updates(as_widget);
        // ^ we don't need this below on a regular geometry update ^
    }
    unset_flags();
}

void BareFrame::draw(WidgetRenderer & target) const {
    decoration().draw(target);
    for (const auto * widget_ptr : m_widgets) {
        widget_ptr->draw(target);
    }    
}

void BareFrame::get_widget_placements(WidgetPlacementVector & vec, const int k_horz_space) const {
    vec.reserve(m_widgets.size());
    vec.clear();

    class WidgetPlacer final : public PartialWidgetPlacerAdapter {
    public:
        void on_line_end(int, int) final {}
        void operator () (Widget * widget_ptr, int x, int y) const final {
            assert(!dynamic_cast<LineSeperator *>(widget_ptr));
            assert(dest && widget_ptr);
            dest->emplace_back(widget_ptr, VectorI(x, y) + start);
#           if 0
            std::cout << "{" << x << "}" << std::flush;
            if (has_been_done.end() == has_been_done.find(widget_ptr)) {
                has_been_done.insert(widget_ptr);
            } else {
                throw std::runtime_error(":(");
            }
#           endif
        }
        void debug_buff_pad() final {
#           if 0
            std::cout << "-";
#           endif
        }
        void debug_on_new_line() final {
#           if 0
            std::cout << ".";
#           endif
        }
        WidgetPlacementVector * dest = nullptr;
        VectorI start;
#       if 0
        mutable std::set<Widget *> has_been_done;
#       endif
    };

    WidgetPlacer placer;
    placer.assign_line_seperator(m_the_line_seperator);
    placer.assign_horizontal_spacers_vector(m_horz_spacers);
    placer.dest = &vec;
    placer.start = decoration().widget_start() + VectorI(1, 1)*padding();
#   if 0
    std::cout << "placement (" << k_horz_space << ") ";
#   endif
    run_placer_algo(placer, m_widgets, k_horz_space, padding());
#   if 0
    std::cout << std::endl;
#   endif
}

void BareFrame::swap(BareFrame & lhs) {
    std::swap(m_padding, lhs.m_padding);
}

/* private */ bool BareFrame::contains(const Widget * wptr) const noexcept {
    for (const auto * widget : m_widgets) {
        if (widget == wptr) return true;
        if (const auto * frame = dynamic_cast<const BareFrame *>(widget)) {
            if (frame->contains(wptr)) return true;
        }
    }
    return false;
}

/* private */ void BareFrame::iterate_children_(const ChildWidgetIterator & itr) {
    for (auto * widget : m_widgets) {
        itr(*widget);
        widget->iterate_children(itr);
    }
}

/* private */ void BareFrame::iterate_children_const_
    (const ChildConstWidgetIterator & itr) const
{
    for (const auto * widget : m_widgets) {
        itr(*widget);
        widget->iterate_children(itr);
    }
}
#if 0
void print(const std::vector<VectorI> & vec) {
    for (const auto & r : vec) {
        std::cout << "(" << r.x << ", " << r.y << ")"
                  << ((&r == &vec.back()) ? "" : ",");
    }
    std::cout << std::endl;
}

void print_w(const std::vector<HorizontalSpacer> & vec) {
    for (const auto & r : vec) {
        std::cout << "[" << r.width() << "]"
                  << ((&r == &vec.back()) ? "" : ",");
    }
    std::cout << std::endl;
}
#endif
/* private */ void BareFrame::update_size() {
    for (auto * widget : m_widgets) {
        assert(widget);
        widget->update_size();
    }

    auto & deco = decoration();
#   if 0
    std::vector<VectorI> old_locations;
    for (auto * widget : m_widgets) if (!dynamic_cast<LineSeperator *>(widget) && widget) old_locations.emplace_back(widget->location());
    std::cout << "old "; print(old_locations);
#   endif
#   if 0
    std::cout << "old "; print_w(m_horz_spacers);
#   endif
    // place without spacers
    {
    int available_width = deco.maximum_width_for_widgets();
    //for (auto & hs : m_horz_spacers) hs.set_width(0);
    get_widget_placements(m_widget_placements, available_width);
    }

    // redo placement adjusted for minimum needed width
    WidgetBoundsFinder widget_extremes = BareFrame::get_measurements(m_widget_placements);
    update_horizontal_spacers(widget_extremes.recorded_width());
    get_widget_placements(m_widget_placements, widget_extremes.recorded_width());
#   if 0
    std::vector<VectorI> new_locations;
    for (const auto & tuple : m_widget_placements) new_locations.emplace_back(std::get<1>(tuple));
    std::cout << "new "; print(new_locations);
#   endif
#   if 0
    std::cout << "new "; print_w(m_horz_spacers);
#   endif
    // request size (padding needs to be included)
    int requested_width = widget_extremes.recorded_width() + padding()*2;
    auto accepted_size = deco.request_size
        (requested_width, widget_extremes.recorded_height() + padding()*2);

    // if the request failed, we end up having to play by the frame
    // decorations' rules
    if (accepted_size.width != requested_width) {
        int width_for_widgets = accepted_size.width - padding()*2;
        update_horizontal_spacers(width_for_widgets);
        get_widget_placements(m_widget_placements, width_for_widgets);
        widget_extremes = BareFrame::get_measurements(m_widget_placements);
    }

    // last updates
    m_widget_extremes = widget_extremes;
    deco.update_geometry();
    do_placements(m_widget_placements);
}

/* private */ void BareFrame::check_invarients() const {
#   if 0
    assert(/*!is_nan(width ()) &&*/ width () >= 0.f);
    assert(/*!is_nan(height()) &&*/ height() >= 0.f);
#   endif
}

/* private */ void BareFrame::update_horizontal_spacers(const int k_horz_space) {
    // k_horz_space describes the amount of space available for widgets
    assert(k_horz_space != FrameDecoration::k_no_width_limit_for_widgets);

    HorizontalSpacerUpdater spacer_updater;
    spacer_updater.assign_line_seperator(m_the_line_seperator);
    spacer_updater.assign_horizontal_spacers_vector(m_horz_spacers);
    spacer_updater.set_widget_iterators(m_widgets);
    spacer_updater.set_horizontal_space(k_horz_space);
    run_placer_algo(spacer_updater, m_widgets, k_horz_space, padding());
}

/* private static */ BareFrame::WidgetBoundsFinder BareFrame::get_measurements
    (const WidgetPlacementVector & widget_placements)
{
    WidgetBoundsFinder widget_extremes;
    for (auto [widget_ptr, loc] : widget_placements) {
        // the line seperator reports itself as being at the origin
        // regardless of placement, this can result in inaccurate bounds
        assert(widget_ptr);
        if (is_zero_sized(*widget_ptr)) continue;
        widget_extremes.record_widget_bounds(loc, *widget_ptr);
    }
    return widget_extremes;
}

// ----------------------------------------------------------------------------

// anchor vtable for clang
SimpleFrame::~SimpleFrame() {}

} // end of asgl namespace

namespace {

bool PartialWidgetPlacerAdapter::is_line_seperator(const Widget & widget) const {
    assert(m_line_seperator);
    return m_line_seperator == &widget;
}

bool PartialWidgetPlacerAdapter::is_horizontal_spacer
    (const Widget & widget) const
{
    assert(m_spacers_vector);
    if (m_spacers_vector->empty()) return false;
    return    &widget >= &m_spacers_vector->front()
           && &widget <= &m_spacers_vector->back ();
}

HorizontalSpacerUpdater::~HorizontalSpacerUpdater() {
    // this is testing algorithm correctness
    assert(m_horz_itr   == horizontal_spacers_end());
    assert(m_widget_itr == m_widget_end);
}

void HorizontalSpacerUpdater::on_line_end
    (int widgets_covered, int ns_width)
{
    auto horz_line_end = m_horz_itr;
    for (; widgets_covered; --widgets_covered) {
        assert(m_widget_itr != m_widget_end);
        if (point_to_same(m_widget_itr, horz_line_end)) {
            ++horz_line_end;
        } else {
            assert(!dynamic_cast<asgl::detail::HorizontalSpacer *>(*m_widget_itr));
        }
        ++m_widget_itr;
    }
    if (m_horz_itr == horz_line_end) return;

    int space_for_spacers = m_horz_space - ns_width;
#   if 0
    std::cout << "line end " << m_horz_space << " " << space_for_spacers;
#   endif
    int left_over_pixels  = space_for_spacers % (horz_line_end - m_horz_itr);
    space_for_spacers /= (horz_line_end - m_horz_itr);
#   if 0
    std::cout << " split into " << space_for_spacers << " with lo " << left_over_pixels << std::endl;
#   endif
    assert(space_for_spacers >= 0);
    for (auto itr = m_horz_itr; itr != horz_line_end; ++itr) {
        bool is_mid = itr == m_horz_itr + (horz_line_end - horz_line_end) / 2;
        itr->set_width(space_for_spacers + (is_mid ? left_over_pixels : 0));
    }

    m_horz_itr = horz_line_end;
}

void HorizontalSpacerUpdater::assign_horizontal_spacers_vector
    (std::vector<HorizontalSpacer> & vec)
{
    PartialWidgetPlacerAdapter::assign_horizontal_spacers_vector(vec);
    m_horz_itr = vec.begin();
}

void HorizontalSpacerUpdater::set_widget_iterators
    (const std::vector<Widget *> & widgets)
{
    m_widget_itr = widgets.begin();
    m_widget_end = widgets.end();
}

void do_placements(WidgetPlacementVector & placements) {
    for (auto [widget_ptr, loc] : placements) {
        assert(widget_ptr);
        widget_ptr->set_location(loc.x, loc.y);
    }
    placements.clear();
}

void run_placer_algo
    (WidgetPlacerAdapter & placer, const std::vector<Widget *> & widgets,
     int width_for_widgets, int padding_between_widgets)
{
    static int i = 0;

    int x = 0;
    int y = 0;
    int count = 0;
    int ns_width = 0;

    int line_height = 0;
    auto advance_to_next_line = [&, padding_between_widgets]() {
        placer.on_line_end(count, ns_width);
        y += line_height + padding_between_widgets;
        ns_width = x = count = line_height = 0;
    };

    bool last_is_regular_widget = false;
    for (Widget * widget_ptr : widgets) {
        assert(widget_ptr);
        ++i;
        if (i == 31) {
            int j = 0;
            ++j;
        }
        if (placer.is_line_seperator(*widget_ptr)) {
            last_is_regular_widget = false;
            // we'll put the seperator on the line which it breaks
            ++count;
            advance_to_next_line();
            placer.debug_on_new_line();
            continue;
        } else if (x != 0 && x + widget_ptr->width() > width_for_widgets) {
            // horizontal overflow
            advance_to_next_line();
        }
        // intra widget padding
        bool this_is_regular_widget = !placer.is_horizontal_spacer(*widget_ptr);
        if (this_is_regular_widget && last_is_regular_widget) {
            x        += padding_between_widgets;
            ns_width += padding_between_widgets;
            placer.debug_buff_pad();
        }
        last_is_regular_widget = this_is_regular_widget;

        // placement and advance
        placer(widget_ptr, x, y);
        x += widget_ptr->width();
        if (this_is_regular_widget) {
            ns_width += widget_ptr->width();
        }
        line_height = std::max(line_height, widget_ptr->height());
        ++count;
    }
    if (count) advance_to_next_line();
}

} // end of <anonymous> namespace
