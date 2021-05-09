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

#include <stdexcept>

namespace {

using RtError = std::runtime_error;
using InvArg  = std::invalid_argument;
#if 0
const char * const NOT_A_FRAME_MSG = "This widget is not a frame.";
#endif

asgl::WidgetFlagsUpdater * s_default_flags_updater_ptr = [] () {
    class NullFlagsUpdater final : public asgl::WidgetFlagsUpdater {
        void receive_geometry_needs_update_flag() override {}
        bool needs_geometry_update() const override { return false; }
        void set_needs_redraw_flag() override {}
    };
    static NullFlagsUpdater inst;
    return &inst;
} ();

} // end of <anonymous> namespace

namespace asgl {

ChildWidgetIterator::~ChildWidgetIterator() {}

WidgetRenderer::~WidgetRenderer() {}

WidgetFlagsUpdater::~WidgetFlagsUpdater() {}

// ----------------------------------------------------------------------------

Widget::~Widget() {}

void Widget::set_location(int x, int y) {
    set_needs_geometry_update_flag();
    set_location_(x, y);
}

void Widget::issue_auto_resize() {}

void Widget::iterate_children(ChildWidgetIterator && itr)
    { iterate_children_(itr); }

void Widget::iterate_children(ChildWidgetIterator && itr) const
    { iterate_children_const_(itr); }

void Widget::iterate_children(ChildWidgetIterator & itr)
    { iterate_children_(itr); }

void Widget::iterate_children(ChildWidgetIterator & itr) const
    { iterate_children_const_(itr); }

void Widget::assign_flags_updater(WidgetFlagsUpdater * ptr) {
    m_flags_updater = (ptr == nullptr) ? s_default_flags_updater_ptr : ptr;
}

void Widget::draw(WidgetRenderer & target) const {
    if (!is_visible()) return;
    draw_(target);
}

/* protected */ Widget::Widget():
    m_flags_updater(s_default_flags_updater_ptr)
{}

/* protected */ void Widget::iterate_children_(ChildWidgetIterator &) {}

/* protected */ void Widget::iterate_children_const_(ChildWidgetIterator &) const {}

/* protected */ void Widget::draw_to
    (WidgetRenderer & target, const sf::IntRect & rect, ItemKey key) const
{ target.render_rectangle(rect, key, this); }

/* protected */ void Widget::draw_to
    (WidgetRenderer & target, const TriangleTuple & tri, ItemKey key) const
{ target.render_triangle(tri, key, this); }

/* protected */ void Widget::set_needs_geometry_update_flag()
    { m_flags_updater->receive_geometry_needs_update_flag(); }

/* protected */ void Widget::set_needs_redraw_flag()
    { m_flags_updater->set_needs_redraw_flag(); }

/* static */ void Widget::Helpers::handle_required_fields
    (const char * caller, std::initializer_list<FieldFindTuple> && fields)
{
    using InvArg = std::invalid_argument;
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

/* static */ int Widget::Helpers::verify_padding
    (const StyleField * field, const char * full_caller)
{
    auto make_error = [full_caller](const char * msg)
        { return RtError(std::string(full_caller) + ": " + msg); };
    if (!field) {
        throw make_error("could not find global padding (or any alts to "
                         "padding for this call.");
    } else if (!field->is_type<int>()) {
        throw make_error("padding style must be an integer.");
    } else if (field->as<int>() < 0) {
        throw make_error("padding must be a non-negative integer.");
    }
    return field->as<int>();
}

/* static */ std::shared_ptr<const sf::Font> Widget::Helpers::verify_required_font
    (const StyleField * field, const char * full_caller)
{
    using FontPtr = std::shared_ptr<const sf::Font>;
    auto make_error = [full_caller](const char * msg)
        { return RtError(std::string(full_caller) + ": " + msg); };
    if (!field) {
        throw make_error("could not find required font in style map.");
    } else if (!field->is_type<FontPtr>()) {
        throw make_error("specified style field is not a font.");
    } else if (!field->as<FontPtr>()) {
        throw make_error("font field must point to a font (is nullptr).");
    }
    return field->as<FontPtr>();
}

} // end of asgl namespace

#if 0
namespace ksg {

ChildWidgetIterator::~ChildWidgetIterator() {}

Widget::Widget(): m_visible(true) {}

Widget::~Widget() {}

/* experimental */ void Widget::iterate_children(ChildWidgetIterator && itr)
    { iterate_children_(itr); }

/* experimental */ void Widget::iterate_children(ChildWidgetIterator && itr) const
    { iterate_const_children_(itr); }

/* experimental */ void Widget::iterate_children(ChildWidgetIterator & itr)
    { iterate_children_(itr); }

/* experimental */ void Widget::iterate_children(ChildWidgetIterator & itr) const
    { iterate_const_children_(itr); }
#if 0
void Widget::add_widget(Widget *)
    { throw Error(NOT_A_FRAME_MSG); }
#endif
/* protected experimental */ void Widget::iterate_children_(ChildWidgetIterator &)
    {}

/* protected experimental */ void Widget::iterate_const_children_(ChildWidgetIterator &) const
    {}

#if 0
void Widget::add_children_to(ChildWidgetIterator &) {}

void Widget::add_focus_widgets_to(std::vector<FocusWidget *> &)
    {}
#endif

void Widget::issue_auto_resize() {}

} // end of ksg namespace
#endif
