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
#include <iostream>
#include <cassert>

namespace {

using VectorI = asgl::BareFrame::VectorI;

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
    decoration().set_location(x, y);
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
    if (gv.should_update_geometry) {
        finalize_widgets();
    }

    check_invarients();
}

void BareFrame::stylize(const StyleMap & smap) {

    decoration().stylize(smap);
    m_padding = Helpers::verify_padding(
        smap.find(styles::k_global_padding), "Frame::stylize");

    for (Widget * widget_ptr : m_widgets)
        widget_ptr->stylize(smap);

    set_needs_geometry_update_flag();
    check_for_geometry_updates();
    check_invarients();
}

VectorI BareFrame::location() const { return decoration().location(); }

int BareFrame::width() const { return decoration().width(); }

int BareFrame::height() const { return decoration().height(); }

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

    assign_flags_updater(this);
    iterate_children_f([this](Widget & widget) {
        widget.assign_flags_updater(this);
    });

    set_needs_geometry_update_flag();
    check_invarients();
}

void BareFrame::set_register_click_event(ClickFunctor && f)
    { decoration().set_click_inside_event(std::move(f)); }

void BareFrame::reset_register_click_event()
    { decoration().set_click_inside_event([]() { return FrameDecoration::ClickResponse(); }); }

void BareFrame::set_padding(int pixels)
    { m_padding = pixels; }

void BareFrame::check_for_geometry_updates() {
    if (!needs_geometry_update()) return;

    finalize_widgets();

    unset_needs_geometry_update_flag();
}

void BareFrame::draw(WidgetRenderer & target) const {
    decoration().draw(target);
    for (const auto * widget_ptr : m_widgets) {
        widget_ptr->draw(target);
    }    
}

/* private */ void BareFrame::finalize_widgets() {
    // all size work (needed to be done first so we know where to place widgets)
    issue_auto_resize();

    place_widgets_to_locations();

    on_geometry_update();

    // v we don't need this below on a regular geometry update v
    Widget & as_widget = *this;
    // sadly I can't reveal (all of) the children of this frame without
    // passing this, or incurring a dynamic allocation cost (micro optimizing?)
    m_focus_handler.check_for_child_widget_updates(as_widget);
    // ^ we don't need this below on a regular geometry update ^

    check_invarients();
}

void BareFrame::place_widgets_to_locations() {
    auto & deco = decoration();
    const int start_x = deco.widget_start().x + padding();
    int x = start_x;
    int y = deco.widget_start().y + padding();

    int line_height = 0;
    int pad_fix     = 0;
    auto advance_locals_to_next_line = [&]() {
        y += line_height + padding();
        x = start_x;
        line_height = 0;
        pad_fix = 0;
    };
    const int k_right_limit = location().x + width();
    for (Widget * widget_ptr : m_widgets) {
        assert(widget_ptr);
        // horizontal overflow
        if (is_line_seperator(widget_ptr)) {
            advance_locals_to_next_line();
            continue;
        }
        if (x + get_widget_advance(widget_ptr) > k_right_limit) {
            advance_locals_to_next_line();
            // this widget_ptr is placed as the first element of the line
        }
        if (is_horizontal_spacer(widget_ptr))
            x += pad_fix;

        widget_ptr->set_location(x, y);

        line_height = std::max(widget_ptr->height(), line_height);

        // horizontal advance
        x += get_widget_advance(widget_ptr);
        pad_fix = -m_padding;
    }

    for (Widget * widget_ptr : m_widgets) {
        if (auto * frame_ptr = dynamic_cast<BareFrame *>(widget_ptr))
            frame_ptr->place_widgets_to_locations();
    }
}

void BareFrame::swap(BareFrame & lhs) {
    std::swap(m_padding, lhs.m_padding);
}

/* private */ VectorI BareFrame::compute_size_to_fit() const {
    int total_width  = 0;
    int line_width   = 0;
    int total_height = 0;
    int line_height  = 0;
    int pad_fix      = 0;

    for (Widget * widget_ptr : m_widgets) {
        assert(widget_ptr);
        if (is_horizontal_spacer(widget_ptr)) {
            pad_fix = -padding();
            continue;
        }
        if (is_line_seperator(widget_ptr)) {
            total_width   = std::max(line_width, total_width);
            line_width    = 0;
            total_height += line_height + padding();
            line_height   = 0;
            pad_fix       = 0;
            continue;
        }
        int width  = widget_ptr->width ();
        int height = widget_ptr->height();
        BareFrame * widget_as_frame = nullptr;
        // should I issue auto-resize here?
        if (   width == 0 && height == 0
            && (widget_as_frame = dynamic_cast<BareFrame *>(widget_ptr)))
        {
            auto gv = widget_as_frame->compute_size_to_fit();
            width  = gv.x;
            height = gv.y;
        }
        line_width  += get_widget_advance(widget_ptr) + pad_fix;
        line_height  = std::max(line_height, height);
        pad_fix = 0;
    }

    if (line_width != 0) {
        total_width   = std::max(total_width, line_width);
        total_height += line_height + padding();
    }
    // we want to fit for the title's width and height also
    // accommodate for the title
    auto & deco = decoration();
    total_height += (deco.widget_start() - deco.location()).y;
    total_width = std::max(total_width, deco.minimum_width() + padding()*2);
    if (!m_widgets.empty()) {
        // padding for borders + padding on end
        // during normal iteration we include only one
        total_width  += padding()*3;
        total_height += padding()*3;
    }
    assert(total_width  >= 0);
    assert(total_height >= 0);
    return VectorI(total_width, total_height);
}

/* private */ bool BareFrame::is_horizontal_spacer
    (const Widget * widget) const
{
    if (m_horz_spacers.empty()) return false;
    return !(widget < &m_horz_spacers.front() || widget > &m_horz_spacers.back());
}

/* private */ bool BareFrame::is_line_seperator(const Widget * widget) const
    { return widget == &m_the_line_seperator; }

/* private */ int BareFrame::get_widget_advance(const Widget * widget_ptr) const {
    bool is_special_widget = is_line_seperator(widget_ptr) ||
                             is_horizontal_spacer(widget_ptr);
    return widget_ptr->width() + (is_special_widget ? 0 : m_padding);
}

/* private */ void BareFrame::issue_auto_resize() {
    // ignore auto resize if the frame as a width/height already set
    for (Widget * widget_ptr : m_widgets)
        widget_ptr->issue_auto_resize();

    issue_auto_resize_for_frame();

    auto size_ = compute_size_to_fit();
    auto & deco = decoration();
    deco.request_size(size_.x, size_.y);
    deco.on_geometry_update();

    // update horizontal spacer sizes
    update_horizontal_spacers();
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

/* private */ void BareFrame::iterate_children_(ChildWidgetIterator & itr) {
    for (auto * widget : m_widgets) {
        itr.on_child(*widget);
        widget->iterate_children(itr);
    }
}

/* private */ void BareFrame::iterate_children_const_(ChildWidgetIterator & itr) const {
    for (const auto * widget : m_widgets) {
        itr.on_child(*widget);
        widget->iterate_children(itr);
    }
}

/* private */ void BareFrame::on_geometry_update() {
    // this means there are two calls to this function during the course
    // of geometric computations
    decoration().on_geometry_update();
    for (auto * widget : m_widgets) {
        widget->on_geometry_update();
    }
    unset_needs_geometry_update_flag();
}

/* private */ void BareFrame::check_invarients() const {
#   if 0
    assert(/*!is_nan(width ()) &&*/ width () >= 0.f);
    assert(/*!is_nan(height()) &&*/ height() >= 0.f);
#   endif
}

/* private */ void BareFrame::update_horizontal_spacers() {
    const int k_horz_space = decoration().width_available_for_widgets();
    const int k_start_x    = 0;
    assert(k_horz_space >= 0);
    // horizontal spacers:
    // will have to find out how much horizontal space is available per line
    // first. Next, if there are any horizontal spacers, each of them carries
    // an equal amount of the left over space.
    int x = k_start_x;
    int pad_fix = 0;
    auto line_begin = m_widgets.begin();
    for (auto itr = m_widgets.begin(); itr != m_widgets.end(); ++itr) {
        Widget * widget_ptr = *itr;
        assert(widget_ptr);
        // if the widget follow another non spacer is a spacer than no
        // padding is added
        if (is_horizontal_spacer(widget_ptr)) {
            x += pad_fix;
            pad_fix = 0;
            continue;
        }
        pad_fix = -m_padding;
        int horz_step = get_widget_advance(widget_ptr);

        // horizontal overflow or end of widgets
        if (x + horz_step > k_horz_space || is_line_seperator(widget_ptr)) {
            // at the end of the line, set the widths for the horizontal
            // spacers
            line_begin = set_horz_spacer_widths
                (line_begin, itr, std::max(k_horz_space - x, 0), m_padding);

            // advance to new line
            x = k_start_x;
            pad_fix = 0;
        } // end of horizontal overflow handling

        // horizontal advance
        x += horz_step;
    } // looping through widgets

    if (line_begin == m_widgets.end()) return;

    set_horz_spacer_widths
        (line_begin, m_widgets.end(), std::max(k_horz_space - x, 0), m_padding);
}

/* private */ BareFrame::WidgetItr BareFrame::set_horz_spacer_widths
    (WidgetItr beg, WidgetItr end, int left_over_space, int padding)
{
    assert(left_over_space >= 0.f);
    int horz_spacer_count = 0;
    for (auto jtr = beg; jtr != end; ++jtr) {
        if (is_horizontal_spacer(*jtr))
            ++horz_spacer_count;
    }

    // if no horizontal spacers, space cannot be split up
    if (horz_spacer_count == 0) return end;

    // distribute left over space to spacers
    int width_per_spacer = (left_over_space / horz_spacer_count) - padding;
    width_per_spacer = std::max(0, width_per_spacer);
    for (auto jtr = beg; jtr != end; ++jtr) {
        if (!is_horizontal_spacer(*jtr)) continue;
        auto horz_spacer = dynamic_cast<HorizontalSpacer *>(*jtr);
        assert(horz_spacer);
        // always move on the next state
        horz_spacer->set_width(width_per_spacer);
    }

    return end;
}

// ----------------------------------------------------------------------------

// anchor vtable for clang
SimpleFrame::~SimpleFrame() {}

} // end of asgl namespace
