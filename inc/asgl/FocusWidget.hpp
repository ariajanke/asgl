/****************************************************************************

    File: FocusWidget.hpp
    Author: Aria Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
