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

using VectorI = asgl::Widget::VectorI;
using Text    = asgl::Text;
using InvArg  = std::invalid_argument;
using RtError = std::runtime_error;

void update_title_geometry(VectorI location,
                           const sf::IntRect & title_bar, Text * title);

} // end of <anonymous> namespace

namespace asgl {

namespace detail {

// <----------------------- Private types for frame -------------------------->
// implemenation here, to keep vtable out of every translation unit
// (revealed by clang static analysis)

// method used as anchor for LineSeperator
LineSeperator::~LineSeperator() {}

void HorizontalSpacer::set_location_(int x_, int y_)
    { m_location = VectorI(x_, y_); }

VectorI HorizontalSpacer::location() const
    { return m_location; }

int HorizontalSpacer::width() const
    { return m_width; }

void HorizontalSpacer::set_width(int w) {
    assert(w >= 0.f);
    m_width = w;
}

} // end of detail namespace

// ----------------------------------------------------------------------------

void FrameDecoration::assign_flags_updater(WidgetFlagsReceiver * ptr)
    { m_flags_receiver = ptr ? ptr : &WidgetFlagsReceiver::null_instance(); }

/* protected */ void FrameDecoration::update_drag_position(int x, int y) {
    set_location(x, y);
    set_needs_geometry_update_flag();
}

/* protected */ void FrameDecoration::set_needs_geometry_update_flag() {
    m_flags_receiver->receive_whole_family_upate_needed();
}

/* protected static */ VectorI FrameDecoration::defer_location_to_widgets() {
    return VectorI(std::numeric_limits<int>::max(),
                   std::numeric_limits<int>::max());
}

/* protected static */ int FrameDecoration::defer_width_to_widgets()
    { return std::numeric_limits<int>::min(); }

/* protected static */ int FrameDecoration::defer_height_to_widgets()
    { return std::numeric_limits<int>::min(); }

// ----------------------------------------------------------------------------
// ---                             FrameBorder                              ---

VectorI FrameBorder::widget_start() const {
    VectorI offset(outer_padding(), outer_padding() + title_height());
    if (!m_title.string().empty()) {
        offset.y += outer_padding();
    }
    return location() + offset;
}

VectorI FrameBorder::location() const
    { return VectorI(m_back.left, m_back.top); }

int FrameBorder::width() const
    { return m_back.width; }

int FrameBorder::height() const
    { return m_back.height; }

FrameBorder::EventResponseSignal FrameBorder::process_event
    (const Event & event)
{
    check_should_update_drag(event);

    EventResponseSignal rv;
    if (   event.is_type<MousePress>()
        && m_back.contains(to_vector(event.as<MousePress>())))
    {
        assert(m_click_in_frame);
        rv.skip_other_events = (m_click_in_frame() == k_skip_other_events);
    }

    return rv;
}

void FrameBorder::set_location(int x, int y) {
    m_back.left = x;
    m_back.top  = y;
}

void FrameBorder::stylize(const StyleMap & smap) {
    using namespace styles;
    TextArea::set_required_text_fields(
        m_title, smap.find(styles::k_global_font),
        smap.find(Frame::to_key(Frame::k_title_text_style)),
        "FrameBorder::stylize");

     m_outer_padding = Widget::Helpers::verify_padding(
        smap.find(styles::k_global_padding), "FrameBorder::stylize");

    using std::make_tuple;
    Widget::Helpers::handle_required_fields("FrameBorder::stylize", {
        make_tuple(&m_back_style, "frame border",
                   smap.find(Frame::to_key(Frame::k_title_bar_style))),
        make_tuple(&m_title_bar_style, "title bar",
                   smap.find(Frame::to_key(Frame::k_title_bar_style))),
        make_tuple(&m_widget_body_style, "widget body",
                   smap.find(Frame::to_key(Frame::k_widget_body_style))),
    });
}

void FrameBorder::set_size(int w, int h) {
    if (w < 0 || h < 0) {
        throw InvArg("FrameBorder::set_size: size values must be non-negative integers.");
    }
    m_back.width  = w;
    m_back.height = h;
}

void FrameBorder::set_title(const UString & title_text) {
    m_title.set_string(title_text);
    if (title_text.empty()) {
        ignore_drag_events();
    } else {
        watch_for_drag_events();
    }
}

void FrameBorder::set_title_size(int font_size) {
    (void)font_size;
    throw std::runtime_error("");
}

void FrameBorder::reset_register_click_event()
    { m_click_in_frame = do_default_click_event; }

void FrameBorder::update_geometry() {
    auto title_is_visible = [this] () { return !m_title.string().empty(); };
    const auto loc = location();
    auto w = m_back.width;
    auto h = m_back.height;
    const auto k_title_bar_height = title_height();
    const auto k_title_bar_pad = title_is_visible() ? outer_padding() : 0;
    if (title_is_visible()) {
        m_title_bar.left = loc.x - outer_padding();
        m_title_bar.top  = loc.y - outer_padding();
        m_title_bar.width = w - outer_padding()*2;
        m_title_bar.height = title_height();

        update_title_geometry(loc, m_title_bar, &m_title);
    }

    m_widget_body.left = loc.x + outer_padding();
    m_widget_body.top  = loc.y + k_title_bar_height + outer_padding() + k_title_bar_pad;

    m_widget_body.width  = w - outer_padding()*2;
    m_widget_body.height = h - (k_title_bar_height + outer_padding()*2 + k_title_bar_pad);
}

int FrameBorder::title_width_accommodation() const noexcept
    { return m_title.string().empty() ? 0 : m_title.width(); }

int FrameBorder::maximum_width_for_widgets() const
    { return std::max(m_width_maximum, title_width_accommodation()); }

void FrameBorder::draw(WidgetRenderer & renderer) const {
    auto title_is_visible = [this] () { return !m_title.string().empty(); };
    renderer.render_rectangle(m_back, m_back_style, this);
    renderer.render_rectangle(m_widget_body, m_widget_body_style, this);

    if (title_is_visible()) {
        renderer.render_rectangle(m_title_bar, m_title_bar_style, this);
        m_title.draw_to(renderer);
    }
}

/* private */ int FrameBorder::title_height() const noexcept
    { return m_title.string().empty() ? 0 : (m_title.height() * 3) / 2; }

/* private */ void FrameBorder::check_should_update_drag
    (const Event & event)
{
    switch (event.type_id()) {
    case k_event_id_of<MousePress>:
        mouse_click(event.as<MousePress>().x, event.as<MousePress>().y, m_title_bar);
        break;
    case k_event_id_of<MouseRelease>:
        drag_release();
        break;
    case k_event_id_of<MouseMove>:
        mouse_move(event.as<MouseMove>().x, event.as<MouseMove>().y);
        break;
    default: break;
    }
}

/* private static */ FrameBorder::ClickResponse
    FrameBorder::do_default_click_event()
    { return k_continue_other_events; }

/* private */ int FrameBorder::outer_padding() const noexcept
    { return std::max(0, m_outer_padding); }

} // end of asgl namespace

namespace {

void update_title_geometry(VectorI location,
                           const sf::IntRect & title_bar, Text * title)
{
    VectorI title_offset(
        (title_bar.width  - title->width ()) / 2,
        (title_bar.height - title->height()) / 2);
    title->set_location(location + title_offset);
}

} // end of <anonymous> namespace
