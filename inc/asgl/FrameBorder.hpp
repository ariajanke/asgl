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
    ~LineSeperator() override;

    void process_event(const Event &) override {}

    VectorI location() const override { return VectorI(); }

    int width() const override { return 0; }

    int height() const override { return 0; }

    void stylize(const StyleMap &) override {}

    void draw(WidgetRenderer &) const override {}

private:
    void set_location_(int, int) override {}
    void on_geometry_update() override {}
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

    void draw(WidgetRenderer &) const override {}

private:
    void set_location_(int x, int y) override;

    void on_geometry_update() override {}

    VectorI m_location;
    int m_width;
};

} // end of detail namespace

// ----------------------------------------------------------------------------

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
        bool skip_other_events      = false;
        // maybe omitted
        bool should_update_geometry = false;
    };

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
    virtual void request_size(int w, int h) = 0;

    /** Called by frame, whenever it's time for resizing/relocating widgets. */
    virtual void on_geometry_update() = 0;

    /** All frame decoration needs to specify how it's drawn. */
    virtual void draw(WidgetRenderer &) const = 0;

    /** @returns the minimum width for the decoration */
    virtual int minimum_width() const = 0;

    /** @returns the number of pixels available for widgets.
     *
     *  @note This function is used for widget placement computations.
     */
    virtual int width_available_for_widgets() const = 0;

    virtual void set_click_inside_event(ClickFunctor &&) = 0;

    void assign_flags_updater(WidgetFlagsUpdater *);

    /** @returns the default decoration used by frames, which will throw
     *           exceptions complaining about it decoration being unset.
     */
    static FrameDecoration & null_decoration();

protected:
    void update_drag_position(int x, int y) final;

    void set_needs_geometry_update_flag();

private:
    WidgetFlagsUpdater * m_flags_receiver = &WidgetFlagsUpdater::null_instance();
};

/** A helper class for Frame. The object manages the border graphics, events,
 *  and provides information for widget placement.
 *
 *  Naturally the client only needs to concern themselves mostly with the
 *  title, styling, and click event function.
 */
class FrameBorder final : public FrameDecoration {
public:
    VectorI widget_start() const override;

    VectorI location() const override;

    int width() const override;

    int height() const override;

    EventResponseSignal process_event(const Event &) override;

    void set_location(int x, int y) override;

    // takes styles from frame
    void stylize(const StyleMap &) override;

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

    void set_click_inside_event(ClickFunctor && bfunc) override {
        set_register_click_event(std::move(bfunc));
    }

    void update_geometry();

    /** @return the pixels needed to be set aside for rendering the title's
     *          width
     *  @note   accommodations for the title's height can be made from the
     *          difference between widget_start and location vertical distance.
     */
    int title_width_accommodation() const noexcept;

    /** @return the pixel width available for widgets */
    int width_available_for_widgets() const override;

    void draw(WidgetRenderer &) const override;

    void request_size(int w, int h) override
        { set_size(w, h); }

    void on_geometry_update() override
        { update_geometry(); }

    int minimum_width() const override
        { return title_width_accommodation(); }

private:
    int title_height() const noexcept;

    void check_should_update_drag(const Event &);

    static ClickResponse do_default_click_event();

    int outer_padding() const noexcept;

    int m_outer_padding = styles::k_uninit_size;

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
