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

#include <asgl/Widget.hpp>

#include <functional>

namespace sf { class Event; }

namespace asgl {

class FocusWidget;

namespace detail {

class FrameFocusHandler;

class FocusWidgetAtt {
    friend class asgl::detail::FrameFocusHandler;
    static void notify_focus_gained(FocusWidget &);
    static void notify_focus_lost  (FocusWidget &);
};

}

/** A focus widget is a widget which receives special attention (usually from
 *  the keyboard). An example of a focus widget is a textbox.
 *
 *  Some widget receive focus because it moved towards them through pressing
 *  the tab key or the mouse clicking inside them.
 *
 *  Focus widget may contain other focus widgets (where focus is passed to the
 *  relevent widget). These special widgets should cycle back the their first
 *  member on the call to notify_focus_lost.
 *  @see notify_focus_lost
 */
class FocusWidget : public Widget {
public:
    friend class detail::FocusWidgetAtt;
    ~FocusWidget() override;

    /** This function is called for any special event processing specific for
     *  to this widget being the focus widget.
     */
    virtual void process_focus_event(const Event &) = 0;

    /** Checks if this widget is requesting focus, and then turns the request
     *  off.
     *  @return true if requesting focus, false otherwise
     */
    bool reset_focus_request();

protected:
    /** A widget may request focus. Frames will respond to requests and reset
     *  them on process_event.
     */
    void request_focus() { m_request_focus = true; }

    virtual void notify_focus_gained() = 0;

    virtual void notify_focus_lost() = 0;

    bool has_focus() const { return m_has_focus; }

private:
    bool m_request_focus = false;
    bool m_has_focus     = false;
};

namespace detail {

class FrameFocusHandler {
public:
    using FocusChangeFunc = std::function<bool(const Event &)>;

    /** @brief Sets a function object, when it returns true, it advances the
     *         focus.
     */
    void set_focus_advance(FocusChangeFunc &&);

    /** @brief Sets a function object, when it returns true, it advances the
     *         focus.
     */
    void set_focus_advance(const FocusChangeFunc & = default_focus_advance);

    /** @brief Sets a function object, when it returns true, it advances the
     *         focus.
     */
    void set_focus_regress(FocusChangeFunc &&);

    /** @brief Sets a function object, when it returns true, it advances the
     *         focus.
     */
    void set_focus_regress(const FocusChangeFunc & = default_focus_regress);


    /** @brief Checks for events that trigger a focus advance. This also sends
     *         focus events to the current focus widget.
     */
    void process_event(const Event &);

    /** This function provides a point for Frames to deposit all focus widgets
     *  to.
     */
    void take_widgets_from(std::vector<FocusWidget *> &);

    /** @brief Clears all focus widgets, essentially disabling focus events.
     *         Useful for nested frames.
     */
    void clear_focus_widgets();

    /** @brief By default, the frame will advance focus when the user presses
     *         the tab key.
     *  @return true if the frame should advance focus, false otherwise
     */
    static bool default_focus_advance(const Event &);

    /** @brief By default, the frame will regress focus when the user presses
     *         the tab + shift key.
     *  @return true if the frame should regress focus, false otherwise
     */
    static bool default_focus_regress(const Event &);

private:
    FocusChangeFunc m_advance_func = default_focus_advance;
    FocusChangeFunc m_regress_func = default_focus_regress;

    std::vector<FocusWidget *> m_focus_widgets;
    std::vector<FocusWidget *>::iterator m_current_position;
};

} // end of detail namespace

} // end of asgl namespace
