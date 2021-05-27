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

#include <asgl/Widget.hpp>

#include <algorithm>

namespace {

using namespace cul::exceptions_abbr;

} // end of <anonymous> namespace

namespace asgl {

ChildWidgetIterator::~ChildWidgetIterator() {}

ChildConstWidgetIterator::~ChildConstWidgetIterator() {}

WidgetRenderer::~WidgetRenderer() {}

/* static */ WidgetFlagsReceiver & WidgetFlagsReceiver::null_instance() {
    class NullFlagsUpdater final : public asgl::WidgetFlagsReceiver {
        void receive_whole_family_upate_needed() {}
        void receive_individual_update_needed(Widget *) {}
    };
    static NullFlagsUpdater inst;
    return inst;
}

WidgetFlagsReceiver::~WidgetFlagsReceiver() {}

// ----------------------------------------------------------------------------

Widget::~Widget() {}

void Widget::set_location(int x, int y) {
    // setting location should not affect the size of the widget, and this is
    // strictly enforced
    auto w = width(), h = height();
    set_location_(x, y);
    if (w == width() && h == height()) return;
    throw RtError("Widget::set_location: re-positioning the widget should not "
                  "change its size.");
}

void Widget::iterate_children(const ChildWidgetIterator & itr)
    { iterate_children_(itr); }

void Widget::iterate_children(const ChildConstWidgetIterator & itr) const
    { iterate_children_const_(itr); }

void Widget::assign_flags_receiver(WidgetFlagsReceiver * ptr)
    { m_flags_receiver = ptr ? ptr : &WidgetFlagsReceiver::null_instance(); }

/* protected */ void Widget::iterate_children_(const ChildWidgetIterator &) {}

/* protected */ void Widget::iterate_children_const_
    (const ChildConstWidgetIterator &) const {}

/* protected */ void Widget::draw_to
    (WidgetRenderer & target, const Rectangle & rect, ItemKey key) const
{ target.render_rectangle(rect, key, this); }

/* protected */ void Widget::draw_to
    (WidgetRenderer & target, const Rectangle & lrect,
     const Rectangle & rrect, ItemKey key) const
{ target.render_rectangle_pair(lrect, rrect, key, this); }

/* protected */ void Widget::draw_to
    (WidgetRenderer & target, const Triangle & tri, ItemKey key) const
{ target.render_triangle(tri, key, this); }

/* protected */ void Widget::flag_needs_whole_family_geometry_update()
    { m_flags_receiver->receive_whole_family_upate_needed(); }

/* protected */ void Widget::flag_needs_individual_geometry_update()
    { m_flags_receiver->receive_individual_update_needed(this); }

/* static */ void Widget::Helpers::handle_required_fields
    (const char * caller, std::initializer_list<FieldFindTuple> && fields)
{
    for (auto & [style_ptr, name, field] : fields) {
        if (!style_ptr) {
            throw InvArg("Widget::Helpers::handle_required_fields: all "
                         "item pointers must point to something.");
        }
        if (!field && *style_ptr == ItemKey()) {
            throw RtError(std::string(caller)
                  + ": map missing required field named \""
                  + std::string(name) + "\".");
        }
    }
    for (auto & [style_ptr, name, field] : fields) {
        if (*style_ptr != ItemKey()) continue;
        *style_ptr = verify_item_key_field(*field, caller, name);
    }
}

/* static */ ItemKey Widget::Helpers::verify_item_key_field
    (const StyleField & field, const char * full_caller, const char * key_name)
{
    if (auto * rv = field.as_pointer<ItemKey>()) {
        return *rv;
    }
    throw InvArg(std::string(full_caller) + ": field \""
                 + std::string(key_name) + "\" must be an item key.");
}

/* static */ void Widget::Helpers::verify_non_negative
    (int x, const char * full_caller, const char * dim_name)
{
    if (x >= 0) return;
    throw InvArg(std::string(full_caller) + ": " + std::string(dim_name)
                 + " must be a non-negative integer.");
}

/* static */ const int * Widget::Helpers::optional_padding
    (const StyleField * field, const char * full_caller)
{
    auto make_error = [full_caller](const char * msg)
        { return RtError(std::string(full_caller) + ": " + msg); };
    if (!field) return nullptr;
    if (!field->is_type<int>()) {
        throw make_error("padding style must be an integer.");
    } else if (field->as<int>() < 0) {
        throw make_error("padding must be a non-negative integer.");
    }
    return &field->as<int>();
}

/* static */ int Widget::Helpers::verify_padding
    (const StyleField * field, const char * full_caller)
{
    auto rv = optional_padding(field, full_caller);
    if (rv) return *rv;
    throw RtError(std::string(full_caller) + ": could not find global padding "
                  "(or any alts to padding for this call.");
}

// ----------------------------------------------------------------------------

void WidgetFlagsReceiverWidget::receive_whole_family_upate_needed()
    { m_geo_update_flag = true; }

void WidgetFlagsReceiverWidget::receive_individual_update_needed(Widget * wid) {
    if (wid) {
        m_individuals.push_back(wid);
        return;
    }
    throw InvArg("FlagsReceivingWidget::receive_individual_update_needed: "
                 "widget pointer must not be null.");
}

/* protected */ void WidgetFlagsReceiverWidget::unset_flags() {
    if (m_geo_update_flag) {
        // if whole family flag was set, then geometry updates cannot take
        // place here in this function, but rather in the more well defined
        // BareFrame class
        m_geo_update_flag = false;
        m_individuals.clear();
        return;
    }
    // I've failed to find evidence that std sort uses std::less,
    // therefore I must pass it explicitly
    std::sort(m_individuals.begin(), m_individuals.end(), std::less<Widget *>());
    Widget * before = nullptr;
    for (auto * widget : m_individuals) {
        if (widget == before) continue;
        // a different way to redo geometry
        auto loc = widget->location();
        auto old_size = widget->size();
        widget->set_location(loc.x, loc.y);
        if (old_size != widget->size())
            { throw make_size_changed_error("unset_flags"); }
        before = widget;
    }
    m_individuals.clear();
}

/* protected */ bool WidgetFlagsReceiverWidget::
    needs_whole_family_geometry_update() const
    { return m_geo_update_flag; }

/* private static */ RtError WidgetFlagsReceiverWidget::make_size_changed_error(const char * caller) noexcept {
    return RtError("FlagsReceivingWidget::" + std::string(caller)
                   + ": Widgets must not change size on individual updates "
                     "(call \"receive_whole_family_upate_needed\" instead.)");
}

} // end of asgl namespace
