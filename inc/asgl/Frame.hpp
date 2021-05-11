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

#include <asgl/FrameBorder.hpp>
#include <asgl/FocusWidget.hpp>

#include <vector>

namespace asgl {

class WidgetAdder {
public:
    WidgetAdder() {}
    WidgetAdder(BareFrame *, detail::LineSeperator *);
    WidgetAdder(const WidgetAdder &) = delete;
    WidgetAdder(WidgetAdder &&);

    /** @note does nothing when an exception is being thrown */
    ~WidgetAdder() noexcept(false);

    WidgetAdder & operator = (const WidgetAdder &) = delete;
    WidgetAdder & operator = (WidgetAdder &&);

    WidgetAdder & add(Widget &);
    WidgetAdder & add_horizontal_spacer();
    WidgetAdder & add_line_seperator();

    void swap(WidgetAdder &);

private:
    std::vector<Widget *> m_widgets;
    std::vector<detail::HorizontalSpacer> m_horz_spacers;
    detail::LineSeperator * m_the_line_sep = nullptr;

    BareFrame * m_parent = nullptr;
};

/** @brief A frame is a collection of widgets. This class provides an interface
 *         which allows controling the entire collection widgets as one group.
 *
 *  @warning This is not intented for regular bordered, top level frames. For
 *           a class that serves that purpose, @see the asgl::Frame class.
 *
 *  The Frame class controls widget's placement. The exact size of the frame
 *  is mostly out of conrol and is subject to the constraints set by the
 *  individual widgets.
 *
 *  By default a frame is at position (0, 0).
 *  @n
 *  Widget organization: @n
 *  Widgets are placed into "lines" from left to right in the order that they
 *  are added (much like English text). @n
 *  There exists so far two special classes to handle layout:
 *  - LineSeperator, functions like a newline
 *  - HorizontalSpacer, blank spaces, expands to fill the line, but does not
 *    affect the size of other widgets
 *
 *  Ownership Model: @n
 *  BareFrame does not own any widget (except special types spacers and line
 *  seperators). All widgets must exist for the duration that the frame does,
 *  with one exception. The widgets maybe deleted only as the frame is being
 *  destroyed. For example, and inheriting class whose direct members are being
 *  used as its widget (for this base class). @n
 *  @n
 *  It is quite possible to add widgets which are dynamically allocated, they
 *  are just not owned. @n
 *  @n
 *  The following is example code showing how frame can be used compose widgets
 *  into a gui. @n
 *  @code
class DialogBox final : public asgl::Frame {
public:
    DialogBox() {
        begin_adding_widgets()
            add(m_face).
            add(m_dialog).
            add_line_seperator().
            // move the button to the right
            add_horizontal_spacer().
            add(m_ok);
        // ...
    }

    // ...

private:
    // ... some type aliases to get rid of needing asgl::
    ImageWidget m_face;
    TextArea m_dialog;
    TextButton m_ok;
};
    @endcode
 *
 *  Frames uses a restricted version of the composite pattern.
 *  @note On copying/swaping: most contents cannot be copied or swapped due to
 *        the nature of the ownership model. So to copy a user frame, one
 *        suggestion is to include a call where the widgets are added following
 *        that copy/swap construction.
 * @code
class DialogBox final : public asgl::Frame {
public:
    DialogBox()
        { init_widgets(); }

    DialogBox(const DialogBox & lhs):
        Frame(lhs)
        { init_widgets(); }

    void init_widgets() {
        begin_adding_widgets().
            add(m_face).
            add(m_dialog).
            add_line_seperator().
            // move the button to the right
            add_horizontal_spacer().
            add(m_ok);
            // ...
    }
};
 @endcode
 *  @n
 *  Design Issue: @n
 *  Thankfully this is isolated to this class and clients may ignore this
 *  issue, as the class functions just fine with it.
 *  LineSeperator/HorizontalSpacer
 *  requires switch on type for geometric computations to work for Frame
 *  old-solution used switch-on type with enums
 */
class BareFrame : public FlagsReceivingWidget {
public:
    using UString      = std::u32string;
    using ClickFunctor = FrameDecoration::ClickFunctor;

    // style stuff should be pushed into "BorderedFrame"

    BareFrame & operator = (const BareFrame &);
    BareFrame & operator = (BareFrame &&);

    // <---------------------- Frame as a component -------------------------->

    /** Processes an event. If the frame is draggable and has a title it can
     *  move with the user's mouse cursor. This function also sends events to
     *  all its widgets.
     *  @param evnt
     */
    void process_event(const Event &) override;

    /** Gets the pixel location of the frame.
     *  @return returns a vector for location in pixels
     */
    VectorI location() const override;

    int width() const override;

    int height() const override;

    // <------------------ Frame specific functionality ---------------------->

    /** @brief Provides an interface where all widgets maybe added.
     *  @see   Frame::begin_adding_widgets(const StyleMap &)
     *  @note  The difference between calling this and the overloaded function,
     *         is that this function will not completely finalize the widgets.
     *         Finalization will have to be accomplished a different way, like
     *         calling the overload of this function from the parent frame.
     */
    WidgetAdder begin_adding_widgets();

    /** @warning Intended to be called by WidgetAdder only.
     *  @note    Function enables a "secret" handshake between the widget adder
     *           and this frame via the_line_sep param. And will throw an
     *           exception if the pointer values do not match.
     *  @param   the_line_sep must match this instances line seperator
     */
    void finalize_widgets(
        std::vector<Widget *> &&, std::vector<detail::HorizontalSpacer> &&,
        detail::LineSeperator * the_line_sep);

    /** Sets the function/functor to call in the event that the mouse is
     *  clicked inside the frame.
     *  @param f function that takes no arguments and returns a ClickResponse
     *  If function f returns k_skip_other_events, then no other events which
     *  may occur with this frame will fire. If function f returns
     *  k_continue_other_events, then event processing will continue normally
     *  after f returns.
     */
    void set_register_click_event(ClickFunctor && f);

    /** Resets the register click event function back to its default value. */
    void reset_register_click_event();

    void set_padding(int pixels);

    void check_for_geometry_updates();

    void swap(BareFrame &);

    void draw(WidgetRenderer &) const override;

protected:
    BareFrame();

    BareFrame(const BareFrame &);
    BareFrame(BareFrame &&);

    ~BareFrame();

    /** @brief Sometimes the most derived frame class will have it's own auto
     *         resize behavior.
     *
     *  The issue_auto_resize behaviors defined in Frame, are mandatory for
     *  this class to work. This method is provided as an "issue_auto_resize"
     *  for classes inheriting from Frame. Added member widgets will have their
     *  "issue_auto_resize" called first.
     *
     *  @note much like "issue_auto_resize" styles are set for widgets on this
     *        call, so that information is provided for any desired geometric
     *        work.
     */
    virtual void issue_auto_resize_for_frame() {}

    /** Sets the pixel location of the frame.
     *  @param x the x coordinate
     *  @param y the y coordinate
     */
    void set_location_(int x, int y) final;

    // this seems like 1000x better solution
    // should be made private in derived class
    virtual FrameDecoration & decoration() = 0;

    virtual const FrameDecoration & decoration() const = 0;

private:
    using WidgetItr = std::vector<Widget *>::iterator;
    using LineSeperator = detail::LineSeperator;
    using HorizontalSpacer = detail::HorizontalSpacer;

    void stylize(const StyleMap &) override;

    WidgetItr set_horz_spacer_widths
        (WidgetItr beg, WidgetItr end, int left_over_space, int padding);

    void update_horizontal_spacers();

    /** @return Calculates the minimum size to fit all widgets. */
    VectorI compute_size_to_fit() const;

    bool is_horizontal_spacer(const Widget *) const;

    bool is_line_seperator(const Widget *) const;

    int get_widget_advance(const Widget * widget_ptr) const;

    void issue_auto_resize() final;

    bool contains(const Widget *) const noexcept;

    void iterate_children_(ChildWidgetIterator &) final;

    void iterate_children_const_(ChildWidgetIterator &) const final;

    void on_geometry_update() final;

    /** Updates sizes and locations for all member widgets including this frame.
     *  Also sets up focus widgets.
     */
    void finalize_widgets();

    void place_widgets_to_locations();

    void check_invarients() const;

    int padding() const { return std::max(0, m_padding); }

    std::vector<Widget *> m_widgets;
    int m_padding = styles::k_uninit_size;

    //! unique per instance
    LineSeperator m_the_line_seperator;
    std::vector<HorizontalSpacer> m_horz_spacers;

    detail::FrameFocusHandler m_focus_handler;
};

/** This class is intended as a top level, bordered frame, possibly with a
 *  title.
 *
 *  When a title is added to a frame, it becomes draggble by default.
 *
 *  This class also defines many default styles used by serveral widgets.
 */
class Frame : public BareFrame {
public:
    enum StyleEnum {
        k_title_bar_style,
        k_widget_body_style, k_border_size_style,
        k_widget_text_style, k_title_text_style,
        k_style_count
    };

    inline static StyleKey to_key(StyleEnum e)
        { return styles::StyleKeysEnum<StyleEnum, k_style_count>::to_key(e); }

    /** Sets the title of the frame. */
    void set_title(const UString &);

    /** @brief enable/disables the drag frame by title feature, enabled by
     *         default if a title is present.
     */
    void set_drag_enabled(bool);

    /** @returns true if dragging is enabled (only available if a title is
     *           set!)
     */
    bool has_drag_enabled() const;

protected:
    Frame()
        { m_border.assign_flags_updater(this); }

    Frame(const Frame & rhs):
        BareFrame(rhs), m_border(rhs.m_border)
    { m_border.assign_flags_updater(this); }

    Frame(Frame && rhs):
        BareFrame(std::move(static_cast<BareFrame &&>(rhs))),
        m_border(std::move(rhs.m_border))
    { m_border.assign_flags_updater(this); }

    Frame & operator = (const Frame &) = delete;
    Frame & operator = (Frame &&) = delete;

    ~Frame() {}

private:
    FrameDecoration & decoration() final { return m_border; }

    const FrameDecoration & decoration() const final { return m_border; }

    FrameBorder m_border;
};

/** A Simple Frame allows creation of frames without being inherited. This can
 *  be useful if the frames being created will have content depending data
 *  rather than what's hardcoded.
 */
class SimpleFrame final : public Frame {
public:
    SimpleFrame() {}
    ~SimpleFrame() override;
};

inline void Frame::set_title(const UString & str)
    { m_border.set_title(str); }

inline void Frame::set_drag_enabled(bool b) {
    if (b) m_border.watch_for_drag_events();
    else   m_border.ignore_drag_events   ();
}

inline bool Frame::has_drag_enabled() const
    { return m_border.is_watching_for_drag_events(); }

} // end of asgl namespace
