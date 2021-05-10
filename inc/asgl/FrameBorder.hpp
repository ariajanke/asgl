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

#pragma once

#include <asgl/Draggable.hpp>
#include <asgl/Widget.hpp>
#include <asgl/Text.hpp>

#include <functional>

namespace asgl {

// ----------------------------------------------------------------------------
// ---                  Frame Helpers (FrameBorder Below)                   ---

class Frame;

namespace detail {

/** Sentinal Widget type used to force the parent frame to move a new line
 *  (prempts the "overflow" that too many widgets produce are put onto a line).
 *
 *  Features:
 *  - maintian a uniform interface for widgets and frame construction
 *  - forces the frame to move all following widgets to be added on the next
 *    line (see Frame declaration).
 */
class LineSeperator final : public Widget {
public:
    ~LineSeperator() override;

    void process_event(const Event &) override {}

    VectorI location() const override { return VectorI(); }

    int width() const override { return 0; }

    int height() const override { return 0; }

    void stylize(const StyleMap &) override {}

private:
    void set_location_(int, int) override {}
    void on_geometry_update() override {}
    void draw_(WidgetRenderer &) const override {}
};

class HorizontalSpacer final : public Widget {
public:
    HorizontalSpacer(): m_width(0) {}

    void process_event(const Event &) override { }

    VectorI location() const override;

    int width() const override;

    int height() const override { return 0.f; }

    void set_width(int w);

    void stylize(const StyleMap &) override {}

private:
    void set_location_(int x, int y) override;

    void on_geometry_update() override {}
    void draw_(WidgetRenderer &) const override {}

    VectorI m_location;
    int m_width;
};

} // end of detail namespace

// ----------------------------------------------------------------------------

/** A helper class for Frame. The object manages the border graphics, events,
 *  and provides information for widget placement.
 *  Naturally the client only needs to concern themselves mostly with the
 *  title, styling, and click event function.
 */
class FrameBorder final : public Draggable {
public:
    using VectorI = Widget::VectorI;
    using UString = Text::UString;
    enum ClickResponse {
        k_skip_other_events,
        k_continue_other_events
    };

    struct EventResponseSignal {
        bool skip_other_events      = false;
        bool should_update_geometry = false;
    };

    VectorI widget_start() const noexcept;

    VectorI location() const noexcept;

    int width() const noexcept;

    int height() const noexcept;

    EventResponseSignal process_event(const Event &);

    void set_location(int x, int y);

    // takes styles from frame
    void stylize(const StyleMap &);

    void set_size(int w, int h);

    /** Sets the title of the border.
     *  @note Title bar becomes invisible if empty string is given. If a blank
     *        bar is desired for some reason, giving " " as the title will have
     *        the desired effect.
     *  @note An empty string will also disable dragging.
     */
    void set_title(const UString &);

    /** @brief Sets the font size for the border title.
     *  @param font_size font size in points
     */
    void set_title_size(int font_size);

    /** Sets the function/functor to call in the event that the mouse is
     *  clicked inside the frame.
     *  @param f function that takes no arguments and returns a ClickResponse
     *  If function f returns k_skip_other_events, then no other events which
     *  may occur with this frame will fire. If function f returns
     *  k_continue_other_events, then event processing will continue normally
     *  after f returns.
     *  @see Frame::ClickResponse
     */
    template <typename Func>
    void set_register_click_event(Func && f);

    /** Resets the register click event function back to its default value. */
    void reset_register_click_event();

    void update_geometry();

    /** @return the pixels needed to be set aside for rendering the title's
     *          width
     *  @note   accommodations for the title's height can be made from the
     *          difference between widget_start and location vertical distance.
     */
    int title_width_accommodation() const noexcept;

    /** @return the pixel width available for widgets */
    int width_available_for_widgets() const noexcept;

    void set_border_size(float pixels);

    void draw(WidgetRenderer &) const;

private:
    void update_drag_position(int drect_x, int drect_y) override;

    int title_height() const noexcept;

    void check_should_update_drag(const Event &);

    static ClickResponse do_default_click_event();

    int outer_padding() const noexcept;

    int m_outer_padding = styles::k_uninit_size;
    bool m_recently_dragged = false;

    sf::IntRect m_back, m_title_bar, m_widget_body;
    ItemKey m_back_style, m_title_bar_style, m_widget_body_style;

    Text m_title;

    std::function<ClickResponse()> m_click_in_frame = do_default_click_event;
};

// ----------------------------------------------------------------------------

template <typename Func>
void FrameBorder::set_register_click_event(Func && f) {
    m_click_in_frame = std::move(f);
}

} // end of ksg namespace
