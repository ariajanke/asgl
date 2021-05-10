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

/** Much like the Frame class, only this one maybe instantiated, and added as
 *  a member of a class.
 */
class SimpleFrame;

class FocusWidget;

class WidgetAdder {
public:
    WidgetAdder() {}
    WidgetAdder(Frame *, detail::LineSeperator *);
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

    Frame * m_parent = nullptr;
};

/** @brief A frame is a collection of widgets. This class is meant to provide
 *  an interface which allows controling the entire collection as one group
 *  rather than individuals.
 *
 *  The Frame class controls widget's placement and
 *  resources. The exact size of the frame is mostly out of conrol and is
 *  subject to the constraints set by the individual widgets.
 *  By default a frame is at position (0, 0) and has no title. It will become
 *  draggable if a title is set. @n
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
 *  Frame does not own any widget (except special types spacers and line
 *  seperators). Anything that adds a widget to the Frame is responsilble for
 *  it's deletion. This class is designed, so that inheritors may have widgets
 *  as concrete fields (like any field declared as part of a class). It is
 *  however, quite possible to widgets which are dynamically allocated, they
 *  are just not owned (only 'observed'). @n
 *  @n
 *  The following is example code showing how frame can be used compose widgets
 *  into a gui. @n
 *  @code
class DialogBox final : public ksg::Frame {
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
    // ... some type aliases to get rid of needing ksg::
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
class DialogBox final : public ksg::Frame {
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
class Frame : public FlagsReceivingWidget {
public:
    // refactoring notes:
    // I may need to rewrite this entire class... :c
    // logical seperations
    // widget container
    // frame's graphical wrapper
    // there is also a matter of event processing
    // - for the focus object
    // - for everything else
    using UString = std::u32string;
    enum StyleEnum {
        k_title_bar_style,
        k_widget_body_style, k_border_size_style,
        k_widget_text_style, k_title_text_style,
        k_style_count
    };
    using DefaultStyles = styles::StyleKeysEnum<StyleEnum, k_style_count>;
    inline static StyleKey to_key(StyleEnum e)
        { return DefaultStyles::to_key(e); }

    Frame & operator = (const Frame &);
    Frame & operator = (Frame &&);

    // <---------------------- Frame as a component -------------------------->
private:
    /** Sets the pixel location of the frame.
     *  @param x the x coordinate
     *  @param y the y coordinate
     */
    void set_location_(int x, int y) override;

public:
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

    void automatically_set_size() { set_size(0.f, 0.f); }

    /** Sets the size of the containing frame. This also happens to be the
     *  frame's border (and title) also.
     *  @param w width in pixels
     *  @param h height in pixels
     */
    void set_size(int w, int h);

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
     *  @see Frame::ClickResponse
     */
    template <typename Func>
    void set_register_click_event(Func && f);

    /** Resets the register click event function back to its default value. */
    void reset_register_click_event();

    void set_padding(float pixels);

    void check_for_geometry_updates();

    // <---------------------- Frame border/title stuff ---------------------->

    /** Sets the title of the frame.
     *  @param str the new title of the frame
     */
    void set_title(const UString &);

    /** @brief Sets the font size for the border title.
     *  @param font_size font size in points
     */
    void set_title_size(int font_size);

    /** @brief Sets frame's border size (the margins at the extremes of the
     *         frame). Default is five pixels.
     *  @param pixels size in pixels
     */
    void set_frame_border_size(float pixels);

    /** @brief enable/disables the drag frame by title feature, enabled by
     *         default if a title is present.
     */
    void set_drag_enabled(bool);

    /** @returns true if dragging is enabled (only available if a title is
     *           set!)
     */
    bool has_drag_enabled() const;

    void swap(Frame &);

protected:
    Frame();

    Frame(const Frame &);
    Frame(Frame &&);

    void draw_(WidgetRenderer &) const override;
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

private:
    using WidgetItr = std::vector<Widget *>::iterator;
    using LineSeperator = detail::LineSeperator;
    using HorizontalSpacer = detail::HorizontalSpacer;

    void stylize(const StyleMap &) override;

    WidgetItr set_horz_spacer_widths
        (WidgetItr beg, WidgetItr end, float left_over_space, float padding);

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
    // anything related to the frame's border

    //! unique per instance
    LineSeperator m_the_line_seperator;
    std::vector<HorizontalSpacer> m_horz_spacers;

    FrameBorder m_border;

    detail::FrameFocusHandler m_focus_handler;
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

// ----------------------------------------------------------------------------

inline void Frame::set_title(const UString & title)
    { m_border.set_title(title); }

inline void Frame::set_title_size(int font_size)
    { m_border.set_title_size(font_size); }

inline void Frame::set_drag_enabled(bool b) {
    if (b) m_border.watch_for_drag_events();
    else m_border.ignore_drag_events();
}

inline bool Frame::has_drag_enabled() const
    { return m_border.is_watching_for_drag_events(); }

template <typename Func>
void Frame::set_register_click_event(Func && f)
    { m_border.set_register_click_event(std::move(f)); }

inline void Frame::reset_register_click_event()
    { m_border.reset_register_click_event(); }

} // end of ksg namespace
