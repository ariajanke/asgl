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
#if 0
using asgl::detail::FocusWidgetAtt;
#endif
using FocusContIter = asgl::LinearFocusHandler::FocusContIter;
using namespace cul::exceptions_abbr;

constexpr const bool k_log_change_focus = false;

inline void log_change_focus(const char * msg) {
    if constexpr (!k_log_change_focus) return;
    std::cout << msg << std::endl;
}

FocusContIter find_requesting_focus(FocusContIter beg, FocusContIter end);

// how many times do I have a wrapping algoritm?
// if the answer is more than three... in two different projects...
// time to put it in the cul
template <typename Iter, typename Func>
void wrap_forward(Iter itr, Iter beg, Iter end, Func && f) {
    using namespace cul::fc_signal;
    auto start = itr;
    for (; itr != end; ++itr) {
        auto fc = cul::adapt_to_flow_control_signal(f, itr);
        if (fc == k_break) return;
    }
    for (itr = beg; itr != start; ++itr) {
        auto fc = cul::adapt_to_flow_control_signal(f, itr);
        if (fc == k_break) return;
    }
}

template <typename Iter, typename Func>
void wrap_backward(Iter itr, Iter beg, Iter end, Func && f) {
    using namespace cul::fc_signal;
    auto do_it = [&f, &itr] ()
        { return cul::adapt_to_flow_control_signal(f, itr); };
    auto start = itr;
    for (; itr != beg; --itr) {
        if (do_it() == k_break) return;
    }
    if (beg == end) return;
    if (do_it() == k_break) return;
    itr = end - 1;
    for (; itr != start; --itr) {
        if (do_it() == k_break) return;
    }
}

template <typename Iter>
std::ptrdiff_t wrap_difference(Iter a, Iter b, Iter beg, Iter end) {
    if (a == end || b == end) {
        throw InvArg("wrap_difference: a and b must be in a non empty sequence.");
    }
    auto high = std::max(a, b);
    auto low  = std::min(a, b);
    return std::min(high - low, (end - high) + (low - beg));
}

} // end of <anonymous> namespace

namespace asgl {
#if 0
namespace detail {

/* static */ void FocusWidgetAtt::notify_focus_gained(FocusReceiver & fwidget) {
    fwidget.m_has_focus = true;
    fwidget.notify_focus_gained();
}

/* static */ void FocusWidgetAtt::notify_focus_lost(FocusReceiver & fwidget) {
    fwidget.m_has_focus = false;
    fwidget.notify_focus_lost();
}

/* static */ bool FocusWidgetAtt::is_visible_for_focus_advance
    (const FocusReceiver & fwidget)
{ return fwidget.is_visible_for_focus_advance(); }

} // end of detail namespace -> into ::asgl
#endif
FocusReceiver::~FocusReceiver() {}

bool FocusReceiver::reset_focus_request() {
    bool rv = m_request_focus;
    m_request_focus = false;
    return rv;
}
#if 0
namespace detail {
#endif
void LinearFocusHandler::set_focus_advance(FocusChangeFunc && func) {
    m_advance_func = std::move(func);
}

void LinearFocusHandler::set_focus_advance(const FocusChangeFunc & func) {
    auto temp = func;
    set_focus_advance(std::move(temp));
}

void LinearFocusHandler::set_focus_regress(FocusChangeFunc && func) {
    m_regress_func = std::move(func);
}

void LinearFocusHandler::set_focus_regress(const FocusChangeFunc & func) {
    auto temp = func;
    set_focus_regress(std::move(temp));
}

void LinearFocusHandler::process_event(const Event & event) {
    if (m_focus_widgets.empty()) return;

    if (m_current_position != m_focus_widgets.end()) {
        (**m_current_position).process_focus_event(event);
        log_change_focus("[asgl] Focus event sent.");
    }

    auto new_focus = find_requesting_focus(m_focus_widgets.begin(), m_focus_widgets.end());
    if (new_focus == m_focus_widgets.end()) {
#       if 0
        auto old_itr = m_current_position;
#       endif
        if (m_advance_func(event)) {
#           if 0
            if (m_current_position == m_focus_widgets.end()) {
                m_current_position = m_focus_widgets.begin();
            } else if (++m_current_position == m_focus_widgets.end()) {
                // wrap around
                m_current_position = m_focus_widgets.begin();
            }
#           endif
            update_focus(m_current_position, advance_focus_iterator(m_current_position));
            log_change_focus("[asgl] Focus widget advanced.");
        } else if (m_regress_func(event)) {
#           if 0
            if (m_current_position == m_focus_widgets.begin()) {
                // wrap around
                m_current_position = m_focus_widgets.end() - 1;
            } else {
                --m_current_position;
            }
#           endif
            update_focus(m_current_position, regress_focus_iterator(m_current_position));
            log_change_focus("[asgl] Focus widget regressed.");
        }
#       if 0
        if (old_itr != m_current_position) {
#           if 0
            if (old_itr != m_focus_widgets.end())
                FocusWidgetAtt::notify_focus_lost(**old_itr);
            FocusWidgetAtt::notify_focus_gained(**m_current_position);
#           endif
            if (old_itr != m_focus_widgets.end())
                (**old_itr).notify_focus_lost();
            (**m_current_position).notify_focus_gained();
        }
#       endif
    } else {
#       if 0
        if (!FocusWidgetAtt::is_visible_for_focus_advance(**new_focus)) {
#       endif
        if (!(**new_focus).is_visible_for_focus_advance()) {
            throw RtError("FrameFocusHandler::process_event: A widget was "
                          "requesting focus explicitly while not being "
                          "visible for focus advances.");
        }
#       if 0
        if (m_current_position != m_focus_widgets.end())
            { FocusWidgetAtt::notify_focus_lost(**m_current_position); }
        FocusWidgetAtt::notify_focus_gained(**new_focus);
#       endif
        update_focus(m_current_position, new_focus);
#       if 0
        if (m_current_position != m_focus_widgets.end())
            { (**m_current_position).notify_focus_lost(); }
        (**new_focus).notify_focus_gained();
        m_current_position = new_focus;
#       endif
        log_change_focus("[asgl] Focus request answered.");
    }
    check_for_visibility_loss();
}

void LinearFocusHandler::check_for_child_widget_updates(Widget & parent) {
    bool mismatch_detected = false;
    auto itr = m_focus_widgets.begin();
    // O(n) in virtual calls
    parent.iterate_children_f([&mismatch_detected, &itr, this](Widget & child) {
        auto * focwid = dynamic_cast<FocusReceiver *>(&child);
        if (!focwid) return;
        if (mismatch_detected) {
            m_focus_widgets.push_back(focwid);
        } else if (   itr == m_focus_widgets.end()
                   || /* (short circuit) or else */ *itr != focwid)
        {
            mismatch_detected = true;
            if (m_current_position != m_focus_widgets.end()) {
#               if 0
                FocusWidgetAtt::notify_focus_lost(**m_current_position);
#               endif
                (**m_current_position).notify_focus_lost();
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

void LinearFocusHandler::clear_focus_widgets() {
    m_focus_widgets.clear();
    m_current_position = m_focus_widgets.end();
    log_change_focus("[asgl] Focus widgets have been cleared.");
}

/* static */ bool LinearFocusHandler::default_focus_advance(const Event & event) {
    if (const auto * keyev = event.as_pointer<KeyPress>()) {
        return keyev->key == keys::k_tab && !keyev->shift;
    } else if (const auto * gmot = event.as_pointer<GeneralMotion>()) {
        return *gmot == general_motion::k_advance_focus;
    }
    return false;
}

/* static */ bool LinearFocusHandler::default_focus_regress(const Event & event) {
    if (const auto * keyev = event.as_pointer<KeyPress>()) {
        return keyev->key == keys::k_tab && keyev->shift;
    } else if (const auto * gmot = event.as_pointer<GeneralMotion>()) {
        return *gmot == general_motion::k_regress_focus;
    }
    return false;
}

/* private to FrameFocusHandler */ cul::FlowControlSignal
    LinearFocusHandler::FocusAdvancerFunc::operator ()
    (FocusContIter itr)
{
    using namespace cul::fc_signal;
    if (!m_advanced_at_least_once) {
        m_advanced_at_least_once = true;
        return k_continue;
    }
#   if 0
    if (FocusWidgetAtt::is_visible_for_focus_advance(**itr)) {
#   endif
    if ((**itr).is_visible_for_focus_advance()) {
        m_target_itr = itr;
        return k_break;
    }
    return k_continue;
}

/* private */ FocusContIter LinearFocusHandler::advance_focus_iterator
    (FocusContIter start)
{
    // the "first" advance
    if (start == m_focus_widgets.end()) return m_focus_widgets.begin();
    auto rv = m_focus_widgets.end();
    FocusAdvancerFunc faf(rv);
    wrap_forward(start, m_focus_widgets.begin(), m_focus_widgets.end(), faf);
    return rv;
}

/* private */ FocusContIter LinearFocusHandler::regress_focus_iterator
    (FocusContIter start)
{
    if (m_focus_widgets.empty()) return start;
    // the "first" regress here
    if (start == m_focus_widgets.end()) return m_focus_widgets.end() - 1;
    auto rv = m_focus_widgets.end();
    FocusAdvancerFunc faf(rv);
    wrap_backward(start, m_focus_widgets.begin(), m_focus_widgets.end(), faf);
    return rv;
}

/* private */ void LinearFocusHandler::check_for_visibility_loss() {
    if (m_current_position == m_focus_widgets.end()) return;
#   if 0
    if (FocusWidgetAtt::is_visible_for_focus_advance(**m_current_position)) {
#   endif
    if ((**m_current_position).is_visible_for_focus_advance()) {
        return;
    }

    auto new_pos = [this] () {
        auto foreopt = advance_focus_iterator(m_current_position);
        auto backopt = regress_focus_iterator(m_current_position);
        auto end = m_focus_widgets.end();
        if (foreopt == end) {
            // if it's the end iterator... that's alright
            return backopt;
        } else if (backopt == end) {
            return foreopt;
        }
        auto wrap_diff_with_current = [this](FocusContIter itr)
            { return wrap_difference(itr, m_current_position, m_focus_widgets.begin(), m_focus_widgets.end()); };
        auto forediff = wrap_diff_with_current(foreopt);
        auto backdiff = wrap_diff_with_current(backopt);
        return (backdiff < forediff) ? backopt : foreopt;
    } ();

    if (new_pos == m_current_position) return;
#   if 0
    if (new_pos != m_focus_widgets.end())
        FocusWidgetAtt::notify_focus_lost(**m_current_position);
    FocusWidgetAtt::notify_focus_gained(**new_pos);
#   endif

    update_focus(m_current_position, new_pos);
#   if 0
    if (new_pos != m_focus_widgets.end())
        (**m_current_position).notify_focus_lost();
    (**new_pos).notify_focus_gained();
    m_current_position = new_pos;
#   endif
}
#if 0
} // end of detail namespace -> into ::asgl
#endif

void LinearFocusHandler::update_focus
    (FocusContIter & field, FocusContIter new_value) const
{
    static constexpr const auto k_widget_not_visible_for_focus_msg =
        "LinearFocusHandler::update_focus: Attempted to set new focus to a "
        "receiver which is not visible for focus.";

    if (field != m_focus_widgets.end())
        (**field).notify_focus_lost();
    if (new_value != m_focus_widgets.end()) {
        if (!(**new_value).is_visible_for_focus_advance()) {
            throw InvArg(k_widget_not_visible_for_focus_msg);
        }
        (**new_value).notify_focus_gained();
    }
    field = new_value;
}

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
