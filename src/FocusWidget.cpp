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

#include <asgl/FocusWidget.hpp>

#include <SFML/Window/Event.hpp>

#include <cassert>

using asgl::detail::FocusWidgetAtt;

namespace asgl {

namespace detail {

/* static */ void FocusWidgetAtt::notify_focus_gained(FocusWidget & fwidget) {
    fwidget.m_has_focus = true;
    fwidget.notify_focus_gained();
}

/* static */ void FocusWidgetAtt::notify_focus_lost(FocusWidget & fwidget) {
    fwidget.m_has_focus = false;
    fwidget.notify_focus_lost();
}

} // end of detail namespace

FocusWidget::~FocusWidget() {}

bool FocusWidget::reset_focus_request() {
    bool rv = m_request_focus;
    m_request_focus = false;
    return rv;
}

namespace detail {

void FrameFocusHandler::set_focus_advance(FocusChangeFunc && func) {
    m_advance_func = std::move(func);
}

void FrameFocusHandler::set_focus_advance(const FocusChangeFunc & func) {
    auto temp = func;
    set_focus_advance(std::move(temp));
}

void FrameFocusHandler::set_focus_regress(FocusChangeFunc && func) {
    m_regress_func = std::move(func);
}

void FrameFocusHandler::set_focus_regress(const FocusChangeFunc & func) {
    auto temp = func;
    set_focus_regress(std::move(temp));
}

void FrameFocusHandler::process_event(const Event & event) {
    if (m_focus_widgets.empty()) return;

    auto widgets_end = m_focus_widgets.end();
    auto new_focus = widgets_end;

    if (m_current_position != m_focus_widgets.end())
        { (**m_current_position).process_focus_event(event); }

    // explicit requests for focus will override regress/advance events
    for (auto itr = m_focus_widgets.begin(); itr != widgets_end; ++itr) {
        if ((**itr).reset_focus_request() && new_focus == widgets_end) {
            new_focus = itr;
            // do not break
        }
    }

    if (new_focus == widgets_end) {
        auto old_itr = m_current_position;
        if (m_advance_func(event)) {
            if (m_current_position == m_focus_widgets.end()) {
                m_current_position = m_focus_widgets.begin();
            } else if (++m_current_position == m_focus_widgets.end()) {
                m_current_position = m_focus_widgets.begin();
            }
        } else if (m_regress_func(event)) {
            if (m_current_position == m_focus_widgets.begin()) {
                m_current_position = m_focus_widgets.end() - 1;
            } else {
                --m_current_position;
            }
        }
        if (old_itr != m_current_position) {
            if (old_itr != m_focus_widgets.end())
                FocusWidgetAtt::notify_focus_lost(**old_itr);
            FocusWidgetAtt::notify_focus_gained(**m_current_position);
        }
    } else {
        if (m_current_position != m_focus_widgets.end())
            { FocusWidgetAtt::notify_focus_lost(**m_current_position); }
        FocusWidgetAtt::notify_focus_gained(**new_focus);
        m_current_position = new_focus;
    }
}

void FrameFocusHandler::take_widgets_from(std::vector<FocusWidget *> & widgets) {
    m_focus_widgets.clear();
    m_focus_widgets.swap(widgets);
    m_current_position = m_focus_widgets.end();
}

void FrameFocusHandler::clear_focus_widgets() {
    m_focus_widgets.clear();
    m_current_position = m_focus_widgets.end();
}

/* static */ bool FrameFocusHandler::default_focus_advance(const Event & event) {
    if (event.is_type<KeyPress>()) {
        auto keyev = event.as<KeyPress>();
        return keyev.key == keys::k_tab && !keyev.shift;
    }
    return false;
}

/* static */ bool FrameFocusHandler::default_focus_regress(const Event & event) {
    if (event.is_type<KeyRelease>()) {
        auto keyev = event.as<KeyRelease>();
        return keyev.key == keys::k_tab && keyev.shift;
    }
    return false;
}

} // end of detail namespace

} // end of asgl namespace
