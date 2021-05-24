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
    void process_event(const Event &) final {}

    Vector location() const final { return Vector(); }

    Size size() const final { return Size(); }

    void stylize(const StyleMap &) final {}

    void draw(WidgetRenderer &) const final {}

private:
    void set_location_(int, int) final {}
    void update_size() final {}
};

class HorizontalSpacer final : public Widget {
public:
    void process_event(const Event &) final {}

    Vector location() const final;

    Size size() const final;

    void set_width(int w);

    void stylize(const StyleMap &) final {}

    void draw(WidgetRenderer &) const final {}

private:
    void set_location_(int x, int y) final;
    void update_size() final {}

    Vector m_location;
    int m_width = 0;
};

} // end of detail namespace

// ----------------------------------------------------------------------------

/**
 *
 *  The follow code is here for your copying conveince:
 *  @code
class MyDecoration final : public asgl::FrameDecoration {
public:
    Vector widget_start() const override;

    Vector location() const override;

    Size size() const override;

    EventResponseSignal process_event(const Event &) override;

    void set_location(int frame_x, int frame_y) override;

    void stylize(const StyleMap &) override;

    Size request_size(int w, int h) override;

    void draw(WidgetRenderer &) const override;

    int maximum_width_for_widgets() const override;

    void set_click_inside_event(ClickFunctor &&) override;

private:
    void on_inform_is_child() override;
};
 *  @endcode
 */
class FrameDecoration : public Draggable {
public:
    enum ClickResponse {
        k_skip_other_events,
        k_continue_other_events
    };
    using ClickFunctor = std::function<ClickResponse()>;

    struct EventResponseSignal {
        bool skip_other_events = false;
    };

    static constexpr const int k_no_width_limit_for_widgets
        = std::numeric_limits<int>::max();

    /** Each decoration object must tell the frame where to start placing
     *  widgets.
     *
     *  @returns the actual location for placement, not an offset
     */
    virtual Vector widget_start() const = 0;

    /** @returns the top left location of the decoration.
     *
     *  @note The frame identifies its location by the highest leftmost point
     *  between this decoration object and its own highest leftmost widget.
     */
    virtual Vector location() const = 0;

    /** @returns the width of the decoration.
     *
     *  @note The frame identifies its width by whichever is greatest, the
     *        total occupying width of its widgets or the decoration's width.
     */
    int width() const { return size().width; }

    /** @returns the width of the decoration.
     *
     *  @note The frame identifies its height by whichever is greatest, the
     *        total occupying height of its widgets or the decoration's height.
     */
    int height() const { return size().height; }

    /** @returns the size of the decoration.
     *
     *  @note The frame identifies its size by whichever size would enclose
     *        both the widgets and the decoration.
     */
    virtual Size size() const = 0;

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
    virtual Size request_size(int w, int h) = 0;

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

    bool is_child() const { return m_is_child; }

    void inform_is_child();

protected:
    void update_drag_position(int x, int y) final;

    virtual void on_inform_is_child() = 0;

    void set_needs_geometry_update_flag();

private:
    bool m_is_child = false;
    WidgetFlagsReceiver * m_flags_receiver = &WidgetFlagsReceiver::null_instance();
};

class TitleBar final {
public:
    TitleBar();

    void set_location(int x, int y);

    Vector location() const;

    Size size() const;

    Rectangle bounds() const;

    int request_width(int w_);

    int minimum_width() const;

    void set_string(const UString & str);

    void draw(WidgetRenderer & target) const;

    bool is_visible() const;

    void stylize(const StyleMap & smap);

private:
    void check_invarients() const;

    void update_title_location_and_height();

    StyleKey m_bar_style; // default value setup by Frame :/
    ItemKey m_bar_item;
    Rectangle m_bounds;
    Text m_title;
};

/** A helper class for Frame. The object manages the border graphics, events,
 *  and provides information for widget placement.
 *
 *  Naturally the client only needs to concern themselves mostly with the
 *  title, styling, and click event function.
 */
class FrameBorder final : public FrameDecoration {
public:
    Vector widget_start() const final;

    Vector location() const final;

    Size size() const final;

    EventResponseSignal process_event(const Event & event) final;

    void set_location(int frame_x, int frame_y) final;

    void stylize(const StyleMap &) final;

    Size request_size(int w, int h) final;

    void draw(WidgetRenderer & target) const final;

    int maximum_width_for_widgets() const final;

    void set_click_inside_event(ClickFunctor && func) final;

    /** Sets the title of the border.
     *  @note Title bar becomes invisible if empty string is given. If a blank
     *        bar is desired for some reason, giving " " as the title will have
     *        the desired effect.
     *  @note An empty string will also disable dragging.
     */
    void set_title(const UString & str);

    /** Resets the register click event function back to its default value. */
    void reset_register_click_event();

    void set_width_minimum(int);

    void set_width_maximum(int);

private:
    void on_inform_is_child() final {}

    void update_geometry();

    int width_max_with_title() const;

    Rectangle inner_rectangle() const;

    void check_invarients() const;

    static ClickResponse do_default_click_event()
        { return k_continue_other_events; }

    Rectangle m_widget_bounds;
    TitleBar m_title_bar;

    int m_outer_padding = 0;
    int m_inner_padding = 0;

    int m_width_maximum = FrameDecoration::k_no_width_limit_for_widgets;
    int m_width_minimum = 0;

    ItemKey m_border_item, m_widget_body_item;

    ClickFunctor m_click_in_frame = do_default_click_event;
};

} // end of asgl namespace
