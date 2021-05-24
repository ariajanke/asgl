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
#include <asgl/Frame.hpp>

#include <iostream>

namespace {

using asgl::detail::FocusWidgetAtt;
using FocusContIter = asgl::detail::FrameFocusHandler::FocusContIter;

constexpr const bool k_log_change_focus = false;

inline void log_change_focus(const char * msg) {
    if constexpr (!k_log_change_focus) return;
    std::cout << msg << std::endl;
}

FocusContIter find_requesting_focus(FocusContIter beg, FocusContIter end);

} // end of <anonymous> namespace

namespace asgl {

namespace detail {

/* static */ void FocusWidgetAtt::notify_focus_gained(FocusReceiver & fwidget) {
    fwidget.m_has_focus = true;
    fwidget.notify_focus_gained();
}

/* static */ void FocusWidgetAtt::notify_focus_lost(FocusReceiver & fwidget) {
    fwidget.m_has_focus = false;
    fwidget.notify_focus_lost();
}

} // end of detail namespace -> into ::asgl

FocusReceiver::~FocusReceiver() {}

bool FocusReceiver::reset_focus_request() {
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

    if (m_current_position != m_focus_widgets.end()) {
        (**m_current_position).process_focus_event(event);
        log_change_focus("[asgl] Focus event sent.");
    }

    auto new_focus = find_requesting_focus(m_focus_widgets.begin(), m_focus_widgets.end());
    if (new_focus == m_focus_widgets.end()) {
        auto old_itr = m_current_position;
        if (m_advance_func(event)) {
            if (m_current_position == m_focus_widgets.end()) {
                m_current_position = m_focus_widgets.begin();
            } else if (++m_current_position == m_focus_widgets.end()) {
                // wrap around
                m_current_position = m_focus_widgets.begin();
            }
            log_change_focus("[asgl] Focus widget advanced.");
        } else if (m_regress_func(event)) {
            if (m_current_position == m_focus_widgets.begin()) {
                // wrap around
                m_current_position = m_focus_widgets.end() - 1;
            } else {
                --m_current_position;
            }
            log_change_focus("[asgl] Focus widget regressed.");
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
        log_change_focus("[asgl] Focus request answered.");
    }
}

void FrameFocusHandler::check_for_child_widget_updates(Widget & parent) {
    bool mismatch_detected = false;
    auto itr = m_focus_widgets.begin();
    // O(n) in virtual calls
    parent.iterate_children_f([&mismatch_detected, &itr, this](Widget & child) {
#       if 0
        if (auto * frame = dynamic_cast<BareFrame *>(&child)) {
            frame->turn_off_focus_widgets();
            // note: BareFrame may also be a FocusReceiver
        }
#       endif
        auto * focwid = dynamic_cast<FocusReceiver *>(&child);
        if (!focwid) return;
        if (mismatch_detected) {
            m_focus_widgets.push_back(focwid);
        } else if (   itr == m_focus_widgets.end()
                   || /* (short circuit) or else */ *itr != focwid)
        {
            mismatch_detected = true;
            if (m_current_position != m_focus_widgets.end()) {
                FocusWidgetAtt::notify_focus_lost(**m_current_position);
            }
            m_focus_widgets.erase(itr, m_focus_widgets.end());
            // warning: itr is now unusable!
            m_focus_widgets.push_back(focwid);
        }
        if (!mismatch_detected) ++itr;
    });
    if (mismatch_detected) {
        m_current_position = m_focus_widgets.end();
        log_change_focus("[asgl] Focus widgets have been reset (change detected).");
    }
}

void FrameFocusHandler::clear_focus_widgets() {
    m_focus_widgets.clear();
    m_current_position = m_focus_widgets.end();
    log_change_focus("[asgl] Focus widgets have been cleared.");
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

} // end of detail namespace -> into ::asgl

} // end of asgl namespace

namespace {

FocusContIter find_requesting_focus(FocusContIter beg, FocusContIter end) {
    // explicit requests for focus will override regress/advance events
    auto new_focus = end;
    for (auto itr = beg; itr != end; ++itr) {
        if ((**itr).reset_focus_request() && new_focus == end) {
            new_focus = itr;
            // do not break
        }
    }
    return new_focus;
}

} // end of <anonymous> namespace
