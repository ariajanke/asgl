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

class BareFrame;

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
    ~LineSeperator() final;

    void process_event(const Event &) final {}

    VectorI location() const final { return VectorI(); }

    int width() const final { return 0; }

    int height() const final { return 0; }

    void stylize(const StyleMap &) final {}

    void draw(WidgetRenderer &) const final {}

private:
    void set_location_(int, int) final {}
    void update_size() final {}
};

class HorizontalSpacer final : public Widget {
public:
    HorizontalSpacer(): m_width(0) {}

    void process_event(const Event &) final {}

    VectorI location() const final;

    int width() const final;

    int height() const final { return 0; }

    void set_width(int w);

    void stylize(const StyleMap &) final {}

    void draw(WidgetRenderer &) const final {}

private:
    void set_location_(int x, int y) final;
    void update_size() final {}

    VectorI m_location;
    int m_width;
};

} // end of detail namespace

// ----------------------------------------------------------------------------

/**
 *
 *  The follow code is here for your copying conveince:
 *  @code
class MyDecoration final : public asgl::FrameDecoration {
public:
    VectorI widget_start() const final;

    VectorI location() const final;

    int width() const final;

    int height() const final;

    EventResponseSignal process_event(const asgl::Event &) final;

    void set_location(int frame_x, int frame_y) final;

    void stylize(const asgl::StyleMap &) final;

    void request_size(int w, int h) final;

    void update_geometry() final;

    void draw(asgl::WidgetRenderer &) const final;

    int minimum_width() const final;

    int width_available_for_widgets() const final;

    void set_click_inside_event(ClickFunctor &&) final;
};
 *  @endcode
 */
class FrameDecoration : public Draggable {
public:
    using VectorI = Widget::VectorI;
    using UString = Text::UString;

    enum ClickResponse {
        k_skip_other_events,
        k_continue_other_events
    };
    using ClickFunctor = std::function<ClickResponse()>;

    struct EventResponseSignal {
        bool skip_other_events = false;
    };
    struct AcceptedSize {
        int width  = 0;
        int height = 0;
    };

    static constexpr const int k_no_width_limit_for_widgets
        = std::numeric_limits<int>::max();

    /** Each decoration object must tell the frame where to start placing
     *  widgets.
     *
     *  @returns the actual location for placement, not an offset
     */
    virtual VectorI widget_start() const = 0;

    /** @returns the top left location of the decoration.
     *
     *  @note The frame identifies its location by the highest leftmost point
     *  between this decoration object and its own highest leftmost widget.
     */
    virtual VectorI location() const = 0;

    /** @returns the width of the decoration.
     *
     *  @note The frame identifies its width by whichever is greatest, the
     *        total occupying width of its widgets or the decoration's width.
     */
    virtual int width() const = 0;

    /** @returns the width of the decoration.
     *
     *  @note The frame identifies its height by whichever is greatest, the
     *        total occupying height of its widgets or the decoration's height.
     */
    virtual int height() const = 0;

    virtual EventResponseSignal process_event(const Event &) = 0;

    /** Called by frame whenever relocating, values given will be the frames
     *  location.
     */
    virtual void set_location(int frame_x, int frame_y) = 0;

    /** Decoration may take any styles it needs.
     *
     *  @note "Non-optional" since I can't think of more than one case where
     *        frame decoration does not take styles.
     */
    virtual void stylize(const StyleMap &) = 0;

    /** Called by frame with the smallest need width and height.
     *
     * @note Values given to this function depend on previous feedback from
     *       this instance.
     */
    virtual AcceptedSize request_size(int w, int h) = 0;

    /** Called by frame, whenever it's time for resizing/relocating widgets. */
    virtual void update_geometry() = 0;

    /** All frame decoration needs to specify how it's drawn. */
    virtual void draw(WidgetRenderer &) const = 0;

    /** @returns the number of pixels available for widgets, may return
     *           "k_no_width_limit_for_widgets"
     *
     *  @note This function is used for widget placement computations.
     */
    virtual int maximum_width_for_widgets() const = 0;

    virtual void set_click_inside_event(ClickFunctor &&) = 0;

    void assign_flags_updater(WidgetFlagsReceiver *);

protected:
    void update_drag_position(int x, int y) final;

    void set_needs_geometry_update_flag();

    static VectorI defer_location_to_widgets();

    static int defer_width_to_widgets();

    static int defer_height_to_widgets();

private:
    WidgetFlagsReceiver * m_flags_receiver = &WidgetFlagsReceiver::null_instance();
};

/** A helper class for Frame. The object manages the border graphics, events,
 *  and provides information for widget placement.
 *
 *  Naturally the client only needs to concern themselves mostly with the
 *  title, styling, and click event function.
 */
class FrameBorder final : public FrameDecoration {
public:
    VectorI widget_start() const final;

    VectorI location() const final;

    int width() const final;

    int height() const final;

    EventResponseSignal process_event(const Event &) final;

    void set_location(int x, int y) final;

    // takes styles from frame
    void stylize(const StyleMap &) final;

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

    void set_click_inside_event(ClickFunctor && bfunc) final {
        set_register_click_event(std::move(bfunc));
    }

    void update_geometry() final;

    /** @return the pixels needed to be set aside for rendering the title's
     *          width
     *  @note   accommodations for the title's height can be made from the
     *          difference between widget_start and location vertical distance.
     */
    int title_width_accommodation() const noexcept;

    /** @return the pixel width available for widgets */
    int maximum_width_for_widgets() const final;

    void draw(WidgetRenderer &) const final;

    AcceptedSize request_size(int w, int h) final {
        AcceptedSize sz;
        sz.width = std::max(title_width_accommodation(), w);
        sz.width = std::max(sz.width, m_width_minimum);
        sz.height = h;
        set_size(sz.width, title_height() + sz.height);
        return sz;
    }

    void set_width_minimum(int i) {
        Widget::Helpers::verify_non_negative(i, "set_width_minimum", "minimum width");
        m_width_minimum = i;
    }

private:
    int title_height() const noexcept;

    void check_should_update_drag(const Event &);

    static ClickResponse do_default_click_event();

    int outer_padding() const noexcept;

    int m_outer_padding = styles::k_uninit_size;
    int m_width_maximum = FrameDecoration::k_no_width_limit_for_widgets;
    int m_width_minimum = 0;

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

} // end of asgl namespace
