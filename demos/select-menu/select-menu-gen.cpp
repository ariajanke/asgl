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

#include "select-menu-gen.hpp"

#include <cassert>

namespace {

using namespace cul::exceptions_abbr;

} // end of <anonymous> namespace

namespace asgl {

void SelectionEntryDecoration::accept_any_size() {
    m_accepting_any_size = true;
    m_constraint = nullptr;
    check_invarients();
}

void SelectionEntryDecoration::constrain_to(const SelectionEntryDecoration * ptr) {
    if (!ptr) {
        throw InvArg("");
    } else if (this == ptr) {
        throw InvArg("");
    }
    m_constraint = ptr;
    m_accepting_any_size = false;
    check_invarients();
}

Size SelectionEntryDecoration::request_size(int w, int h) {
    assert(m_accepting_any_size || m_constraint);
    if (m_accepting_any_size) {
        m_has_accepted_a_size = true;
        set_size(Size(w, h));
        check_invarients();
        return Size(w, h);
    } else if (m_constraint) {
        if (!m_constraint->m_has_accepted_a_size) {
            throw RtError("");
        }
        auto rv = size_of(m_constraint->bounds());
        set_size(rv);
        check_invarients();
        return rv;
    }
    throw RtError("bad branch");
}

void SelectionEntryDecoration::reset_sizing() {
    m_accepting_any_size = false;
    m_constraint = nullptr;
    check_invarients();
}

/* private */ void SelectionEntryDecoration::check_invarients() const
    { assert(!(m_accepting_any_size && m_constraint)); }

// ----------------------------------------------------------------------------

void SelectionEntryFrame::constrain_to(const SelectionEntryFrame * ptr) {
    if (!ptr) {
        throw InvArg("");
    }
    m_deco.constrain_to(&ptr->m_deco);
}

// ----------------------------------------------------------------------------

void SelectionMenuBase::set_menu_size(int w, int h) {
    m_icons.set_size(w, h, GridEntry());
    m_non_null_itr = m_icons.end();
}

void SelectionMenuBase::set_padding(int i) { m_padding = i; }

Vector SelectionMenuBase::location() const { return m_widget_location; }

Size SelectionMenuBase::size() const {
    if (m_non_null_itr == m_icons.end()) return Size();
    if (!m_non_null_itr->base) {
        throw RtError("SelectionMenuBase::size: iterator seems to have gone bad."
                      "");
    }
    auto icon_size = m_non_null_itr->base->size();
    // test this once "operators" namespace is at play in the cul
#   if 1
    return Size(m_icons.width()*icon_size.width, m_icons.height()*icon_size.height)
           + size_in_padding();
#   else
    return Size(m_icons.width ()*icon_size.width  + size_in_padding().width,
                m_icons.height()*icon_size.height + size_in_padding().height);
#   endif
}

void SelectionMenuBase::stylize(const StyleMap & map) {
    for (auto & tuple : m_icons) {
        if (!tuple.base) continue;
        tuple.base->stylize(map);
    }
}

void SelectionMenuBase::update_size() {
    if (m_icons.is_empty()) return;
    auto itr = m_icons.begin();
    SelectionEntryFrame * constraint = nullptr;
    for (; itr != m_icons.end(); ++itr) {
        if (!*itr) continue;
        constraint = itr->base;
        ++itr;
        break;
    }
    if (!constraint) {
        // an emptied menu is *fine*...
        throw RtError("");
    }
    constraint->accept_any_size();
    for (; itr != m_icons.end(); ++itr) {
        if (!*itr) continue;
        itr->base->constrain_to(constraint);
    }
    for (auto & icon : m_icons) {
        if (!icon) continue;
        auto & widget = static_cast<Widget &>(*icon.base);
        widget.update_size();
    }
}

void SelectionMenuBase::process_event(const Event & event) {
    for (auto & icon : m_icons) {
        if (icon) icon.base->process_event(event);
    }
    using std::get;
    // really not sure what to do style wise
    // if (ptr && ptr->method()) is considered problematic since
    // short-circuiting is not immediately appearent
    // if (ptr) if (ptr->method) {
    //     ...
    // }
    if (m_held_item) /* and then */ if (m_held_item->is_requesting_drop()
        && event.is_type<MouseRelease>())
    {
        GridEntry * hovering_over = nullptr;
        auto cell_point = pixel_point_to_grid_cell(event.as<MouseRelease>());
        if (m_icons.has_position(cell_point)) {
            hovering_over = &m_icons(cell_point);
        }
        assert(m_frame_being_dragged);
        if (hovering_over && m_frame_being_dragged != hovering_over) {
            // it is quite permissible for hovering_over's pointers to be
            // null
            auto gv = on_place_item_on(m_frame_being_dragged->derived,
                                       hovering_over->derived);
            *m_frame_being_dragged = get<0>(gv);
            *hovering_over         = get<1>(gv);
            flag_needs_individual_geometry_update();
        }
        m_held_item           = nullptr;
        m_held_items_parent   = nullptr;
        m_frame_being_dragged = nullptr;
    }

    // search for hold requests
    for (auto & drag_icon : m_icons) {
        if (!drag_icon) continue;
        m_non_null_itr = m_icons.begin() + (&drag_icon - &*m_icons.begin());
        auto [wants_to_be_held, parent] = drag_icon.base->get_hold_request();
        if (!wants_to_be_held) continue;
        m_frame_being_dragged = &drag_icon;
        m_held_items_parent = parent;
        m_held_item = wants_to_be_held;
        break;
    }
}

void SelectionMenuBase::draw(WidgetRenderer & target) const {
    for (auto & entry : m_icons) {
        if (!entry) continue;
        entry.base->draw(target);
    }
    if (m_held_item) {
        assert(m_held_items_parent);
        m_held_item->draw(*m_held_items_parent, target);
    }
}

void SelectionMenuBase::fix_internal_iterators() {
    for (auto & drag_icon : m_icons) {
        if (!drag_icon) continue;
        m_non_null_itr = m_icons.begin() + (&drag_icon - &*m_icons.begin());
    }
    flag_needs_individual_geometry_update();
}

/* protected */ SelectionMenuBase::GridEntry::GridEntry
    (void * derv_, SelectionEntryFrame * base_):
    derived(derv_), base(base_)
{
    if (!(bool(derv_) ^ bool(base_))) return;
    throw InvArg("GridEntry::GridEntry: either both must be set, or both must be null.");
}

/* protected */void SelectionMenuBase::place(Vector r, void * ptr, SelectionEntryFrame * frame) {
    if (bool(ptr) ^ bool(frame)) {
        throw InvArg("SelectionMenuBase::place: both must be set, or both must be null.");
    }
    // if icons is empty or r is out of range, this next access call will
    // throw
    m_icons(r).derived = ptr;
    m_icons(r).base    = frame;

    if (ptr) {
        m_non_null_itr = m_icons.begin() + ( &m_icons(r) - &*m_icons.begin() );
    }
}

/* private */ void SelectionMenuBase::iterate_children_(const ChildWidgetIterator & itr) {
    for (auto & entry : m_icons) {
        if (!entry) continue;
        itr(*entry.base);
        entry.base->iterate_children(itr);
    }
}

/* private */ void SelectionMenuBase::iterate_children_const_(const ChildConstWidgetIterator & itr) const {
    for (auto & entry : m_icons) {
        if (!entry) continue;
        itr(*entry.base);
        entry.base->iterate_children(itr);
    }
}

/* private */ void SelectionMenuBase::set_location_(int x, int y) {
    m_widget_location = Vector(x, y);
    for (Vector r; r != m_icons.end_position(); r = m_icons.next(r)) {
        if (!m_icons(r)) continue;
        auto size = m_icons(r).base->size();
#       if 1
        Vector offset = Vector(x, y)
            + Vector(r.x*size.width, r.y*size.height) + r*m_padding;
#       else
        Vector offset(x + r.x*size.width  + r.x*m_padding,
                      y + r.y*size.height + r.y*m_padding);
#       endif
        m_icons(r).base->set_location(offset.x, offset.y);
    }
}

/* private */ Vector SelectionMenuBase::pixel_point_to_grid_cell(Vector r) const {
    if (m_non_null_itr == m_icons.end()) return Vector(-1, -1);
    r -= location();
    r -= cul::convert_to<Vector>(size_in_padding());
    if (r.x < 0 || r.y < 0) return Vector(-1, -1);
    auto item_size = m_non_null_itr->base->size();
    r.x /= item_size.width;
    r.y /= item_size.height;
    return r;
}

/* private */ Size SelectionMenuBase::size_in_padding() const {
    using std::max;
    return Size(max(0, m_icons.width() - 1), max(0, m_icons.height()))*m_padding;
}


} // end of asgl namespace
