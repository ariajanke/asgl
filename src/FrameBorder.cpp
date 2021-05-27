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

#include <asgl/FrameBorder.hpp>
#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <common/Util.hpp>

#include <cassert>

namespace {

using namespace cul::exceptions_abbr;

} // end of <anonymous> namespace

namespace asgl {

namespace detail {

// <----------------------- Private types for frame -------------------------->
// implemenation here, to keep vtable out of every translation unit
// (revealed by clang static analysis)

void HorizontalSpacer::set_location_(int x_, int y_)
    { m_location = Vector(x_, y_); }

Vector HorizontalSpacer::location() const
    { return m_location; }

Size HorizontalSpacer::size() const
    { return Size(m_width, 0); }

void HorizontalSpacer::set_width(int w) {
    assert(w >= 0);
    m_width = w;
}

} // end of detail namespace

// ----------------------------------------------------------------------------

void FrameDecoration::assign_flags_updater(WidgetFlagsReceiver * ptr)
    { m_flags_receiver = ptr ? ptr : &WidgetFlagsReceiver::null_instance(); }

void FrameDecoration::inform_is_child() {
    m_is_child = true;
    on_inform_is_child();
}

/* protected */ void FrameDecoration::update_drag_position(int x, int y) {
    set_location(x, y);
    set_needs_geometry_update_flag();
}

/* protected */ void FrameDecoration::set_needs_geometry_update_flag()
    { m_flags_receiver->receive_whole_family_upate_needed(); }

// ----------------------------------------------------------------------------

TitleBar::TitleBar():
    m_bar_style(to_key(frame_styles::k_title_bar_style))
{ check_invarients(); }

void TitleBar::set_location(int x, int y) {
    set_top_left_of(m_bounds, x, y);
    update_title_location_and_height();
    check_invarients();
}

Vector TitleBar::location() const { return top_left_of(m_bounds); }

Size TitleBar::size() const { return size_of(m_bounds); }

Rectangle TitleBar::bounds() const { return cul::compose(location(), size()); }

int TitleBar::request_width(int w_) {
    // this maybe poor design
    // accepting a width and then not taking it...
    if (!is_visible()) return w_;
    m_bounds.width = std::max(w_, minimum_width());
    update_title_location_and_height();
    check_invarients();
    return m_bounds.width;
}

int TitleBar::minimum_width() const { return m_title.width(); }

void TitleBar::set_string(const UString & str) {
    m_title.set_string(str);
    m_bounds.width = std::max(m_bounds.width, m_title.width());
    update_title_location_and_height();
    check_invarients();
}

void TitleBar::draw(WidgetRenderer & target) const {
    using namespace cul::exceptions_abbr;
    if (!is_visible()) {
        throw RtError("TitleBar::draw: should not be called when not visible.");
    }
    target.render_rectangle(m_bounds, m_bar_item, this);
    m_title.draw_to(target);
}

bool TitleBar::is_visible() const { return !m_title.string().empty(); }

void TitleBar::stylize(const StyleMap & smap) {
    using namespace styles;
    TextArea::set_required_text_fields(
        m_title, smap.find(styles::k_global_font),
        smap.find(to_key(frame_styles::k_title_text_style)),
        "TitleBar::stylize");

    using std::make_tuple;
    Widget::Helpers::handle_required_fields("TitleBar::stylize", {
        make_tuple(&m_bar_item, "title bar",
                   smap.find(m_bar_style, to_key(frame_styles::k_title_bar_style))),
    });
    m_bounds.width = std::max(m_bounds.width, m_title.width());
    update_title_location_and_height();
    check_invarients();
}

/* private */ void TitleBar::check_invarients() const {
    assert(m_bounds.width  >= m_title.width ());
    assert(m_bounds.height >= m_title.height());
    assert(size_of(m_bounds) == Size() || (m_bounds.width != 0 && m_bounds.height != 0));
}

/* private */ void TitleBar::update_title_location_and_height() {
    m_bounds.height = (m_title.height() * 3) / 2;
    m_title.set_location(
        Vector(m_bounds.width -  m_title.width ()
              ,m_bounds.height - m_title.height()) / 2
        + top_left_of(m_bounds));
}

// ----------------------------------------------------------------------------
// ---                             FrameBorder                              ---

Vector FrameBorder::widget_start() const {
    auto rv = Vector(1, 1)*m_inner_padding;
    rv += Vector(1, 1)*(is_child() ? 0 : m_outer_padding);
    return rv + top_left_of(m_widget_bounds);
}

Vector FrameBorder::location() const { return m_title_bar.location(); }

Size FrameBorder::size() const {
    return Size(m_widget_bounds.width
               ,m_widget_bounds.height + m_title_bar.size().height);
}

FrameBorder::EventResponseSignal FrameBorder::process_event
    (const Event & event)
{
    switch (event.type_id()) {
    case k_event_id_of<MousePress>: {
        EventResponseSignal rv;
        const auto & mousepress = event.as<MousePress>();
        mouse_click(mousepress.x, mousepress.y, m_title_bar.bounds());
        if (is_contained_in(mousepress, m_widget_bounds)) {
            rv.skip_other_events = (m_click_in_frame() == k_skip_other_events);
        }
        check_invarients();
        return rv;
        }
    case k_event_id_of<MouseRelease>:
        drag_release();
        break;
    case k_event_id_of<MouseMove>:
        mouse_move(event.as<MouseMove>().x, event.as<MouseMove>().y);
        break;
    default: break;
    }

    check_invarients();
    return EventResponseSignal();
}

void FrameBorder::set_location(int frame_x, int frame_y) {
    m_title_bar.set_location(frame_x, frame_y);
    update_geometry();
    check_invarients();
}

void FrameBorder::stylize(const StyleMap & smap) {
    using namespace styles;
    using namespace frame_styles;
    m_title_bar.stylize(smap);

    {
    auto gv = Widget::Helpers::optional_padding(
        smap.find(m_padding_style, styles::k_global_padding), "FrameBorder::stylize");
    if (gv) m_inner_padding = m_outer_padding = *gv;
    }
    smap.find(m_widget_body_style, to_key(k_widget_body_style));
    using std::make_tuple;
    Widget::Helpers::handle_required_fields("FrameBorder::stylize", {
        make_tuple(&m_border_item, "frame border",
                   smap.find(to_key(k_title_bar_style))),
        make_tuple(&m_widget_body_item, "widget body",
                   smap.find(m_widget_body_style, to_key(k_widget_body_style))),
    });
    update_geometry();
    int total_pad = (m_outer_padding + m_inner_padding)*2;
    m_widget_bounds.width  = std::max(total_pad, m_title_bar.size().width);
    m_widget_bounds.height = total_pad;
    check_invarients();
}

Size FrameBorder::request_size(int w, int h) {
    Size accepted_size;
    accepted_size.height = h;
    accepted_size.width  = std::max(                  w, m_width_minimum);
    accepted_size.width  = std::min(accepted_size.width, m_width_maximum);
    auto total_pad = (m_outer_padding + m_inner_padding)*2;
    int requested_title_width = accepted_size.width + total_pad;
    int accepted_title_width  = m_title_bar.request_width(requested_title_width);
    if (requested_title_width != accepted_title_width) {
        accepted_size.width = accepted_title_width - total_pad;
    }
    m_widget_bounds.width  = accepted_size.width  + total_pad;
    m_widget_bounds.height = accepted_size.height + total_pad;

    update_geometry();
    check_invarients();
    return accepted_size;
}

void FrameBorder::draw(WidgetRenderer & target) const {
    if (is_child()) {
        if (!m_title_bar.is_visible()) return;
        m_title_bar.draw(target);
        target.render_rectangle(m_widget_bounds, m_widget_body_item, this);
    } else {
        if (m_outer_padding) {
            target.render_rectangle(m_widget_bounds, m_border_item, this);
        }
        target.render_rectangle(inner_rectangle(), m_widget_body_item, this);
        if (m_title_bar.is_visible()) m_title_bar.draw(target);
    }
}

int FrameBorder::maximum_width_for_widgets() const { return m_width_maximum; }

void FrameBorder::set_click_inside_event(ClickFunctor && func)
    { m_click_in_frame = std::move(func); }

void FrameBorder::set_title(const UString & str) {
    m_title_bar.set_string(str);
    update_geometry();
    check_invarients();
}

void FrameBorder::reset_register_click_event()
    { m_click_in_frame = do_default_click_event; }

void FrameBorder::set_width_minimum(int i) {
    Widget::Helpers::verify_non_negative(i, "set_width_minimum", "minimum width");
    m_width_minimum = i;
}

void FrameBorder::set_width_maximum(int i) {
    Widget::Helpers::verify_non_negative(i, "set_width_maximum", "maximum width");
    m_width_maximum = i;
}

void FrameBorder::set_border_padding(int pad) {
    Widget::Helpers::verify_non_negative(pad, "set_border_padding", "padding");
    m_inner_padding = m_outer_padding = pad;
    m_padding_style  = StyleKey();
}

void FrameBorder::set_style(FrameStyle e, ItemKey item) {
    using namespace frame_styles;
    switch (e) {
    case k_widget_body_style:
        m_widget_body_style = StyleKey();
        m_widget_body_item  = item;
        break;
    case k_title_bar_style:
    case k_border_size_style:
    case k_widget_text_style:
    case k_title_text_style:
        throw InvArg("FrameBorder::set_style: unimplemented styles.");
    default:
        throw InvArg("FrameBorder::set_style: style enum must be any valid "
                     "enum value except for count.");
    }
}

/* private */ void FrameBorder::update_geometry() {
    set_top_left_of(m_widget_bounds, location() + Vector(0, m_title_bar.size().height));
}

/* private */ int FrameBorder::width_max_with_title() const {
    return std::max(m_title_bar.size().width, m_width_maximum);
}

/* private */ Rectangle FrameBorder::inner_rectangle() const {
    auto inner_loc = top_left_of(m_widget_bounds) + Vector(1, 1)*m_outer_padding;
    auto inner_size = size_of(m_widget_bounds);
    inner_size.width  -= m_outer_padding*2;
    inner_size.height -= m_outer_padding*2;
    return cul::compose(inner_loc, inner_size);
}

/* private */ void FrameBorder::check_invarients() const {
    assert(m_title_bar.is_visible() ? m_title_bar.size().width <= m_widget_bounds.width : true);
    assert(m_click_in_frame);
    assert(m_widget_bounds.width  >= (m_outer_padding + m_inner_padding)*2);
    assert(m_widget_bounds.height >= (m_outer_padding + m_inner_padding)*2);
    assert(m_title_bar.location().y + m_title_bar.size().height <= m_widget_bounds.top);
}

} // end of asgl namespace
