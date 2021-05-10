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

#include <asgl/StyleMap.hpp>
#include <asgl/Event.hpp>

#include <vector>

namespace sf {
    class Font;
    class Event;
}

namespace asgl {

class FocusWidget;
class Widget;

/** @brief Child widget iterator enables a way to iterate all the child widgets
 *         for some given parent widget.
 *
 *  This class uses double dispatch between Widget's iterate_children_ and
 *  iterate_const_children_ virtual functions, and this classes on_child_ and
 *  on_const_child_ virtual functions.
 */
class ChildWidgetIterator {
public:
    virtual ~ChildWidgetIterator();
    void on_child(Widget & widget) { on_child_(widget); }
    void on_child(const Widget & widget) { on_const_child_(widget); }

protected:
    virtual void on_child_(Widget &) {}
    virtual void on_const_child_(const Widget &) {}
    ChildWidgetIterator() {}
};

class WidgetFlagsUpdater {
public:
    virtual ~WidgetFlagsUpdater();

    virtual void receive_geometry_needs_update_flag() = 0;
    virtual bool needs_geometry_update() const = 0;
    virtual void set_needs_redraw_flag() = 0;
};

#if 0
class SfmlTextObject;
#endif
class TextBase;
using TriangleTuple = std::tuple<sf::Vector2i, sf::Vector2i, sf::Vector2i>;

class WidgetRenderer {
public:
    virtual ~WidgetRenderer();

    virtual void render_rectangle(const sf::IntRect   &, ItemKey, const void * widget_spec_ptr) = 0;
    virtual void render_triangle (const TriangleTuple &, ItemKey, const void * widget_spec_ptr) = 0;
#   if 0
    [[deprecated]] virtual void render_text(const SfmlTextObject &) = 0;
#   endif
    virtual void render_text(const TextBase &) = 0;
};

/** A frame needs four things from a widget, in order to position the widget
 *  and setup the frame.
 *
 *  === COMPOSITE PATTERN ===
 *  A Widget is a component, by default add_component (and future composite
 *  functions throws exceptions).
 */
class Widget {
public:
    using VectorI = sf::Vector2i;

    virtual ~Widget();

    virtual void process_event(const Event &) = 0;

    void set_location(int x, int y);

    virtual VectorI location() const = 0;

    virtual int width() const = 0;

    virtual int height() const = 0;

    /** Sets all common fonts, paddings, colors
     *
     */
    virtual void stylize(const StyleMap &) = 0;

    /** This function is called whenever it's time for the widget to
     *  rearrange its internals and recompute geometry of its various features.
     *
     *  @note This is called after all widgets has had their proper locations
     *        set.
     */
    virtual void on_geometry_update() = 0;

    /** @brief Called by frame for automatic widget sizing.
     *  Widget computes its own size.
     *  @note This is called after styles are set, so fonts for widgets and
     *        others will be accessible on this call.
     *  @note The default behavior is for the widget to do nothing (no resize)
     *  @note Presently Frame and TextArea both use this function.
     */
    virtual void issue_auto_resize();

    template <typename Func>
    void iterate_children_f(Func &&);

    template <typename Func>
    void iterate_children_const_f(Func &&) const;

    void iterate_children(ChildWidgetIterator &&);
    void iterate_children(ChildWidgetIterator &&) const;

    void iterate_children(ChildWidgetIterator &);
    void iterate_children(ChildWidgetIterator &) const;
    void set_visible(bool v) { m_visible = v; }

    bool is_visible() const { return m_visible; }

    void assign_flags_updater(WidgetFlagsUpdater *);

    void draw(WidgetRenderer &) const;

    struct Helpers {
        using FieldFindTuple = std::tuple<ItemKey *, const char *, const StyleField *>;

        static void handle_required_fields
            (const char * caller, std::initializer_list<FieldFindTuple> && fields);

        static ItemKey verify_item_key_field
            (const StyleField &, const char * full_caller, const char * key_name);

        static void verify_non_negative(int, const char * full_caller, const char * dim_name);

        static int verify_padding(const StyleField *, const char * full_caller);

        static std::shared_ptr<const sf::Font> verify_required_font
            (const StyleField *, const char * full_caller);
    };
protected:
    Widget();

    virtual void iterate_children_(ChildWidgetIterator &);

    virtual void iterate_children_const_(ChildWidgetIterator &) const;

    virtual void set_location_(int x, int y) = 0;

    virtual void draw_(WidgetRenderer &) const = 0;

    void draw_to(WidgetRenderer &, const sf::IntRect &, ItemKey) const;

    void draw_to(WidgetRenderer &, const TriangleTuple &, ItemKey) const;

    void set_needs_geometry_update_flag();

    void set_needs_redraw_flag();

private:
    bool m_visible = true; // I'm questioning the usefulness of this
                           // especially this this would end up on *all* controls
    WidgetFlagsUpdater * m_flags_updater = nullptr; // safety set
};

class FlagsReceivingWidget : public Widget, public WidgetFlagsUpdater {
public:
    void receive_geometry_needs_update_flag() final
        { m_geo_update_flag = true; }
    bool needs_geometry_update() const final { return m_geo_update_flag; }

    void set_needs_redraw_flag() final {}

    void unset_needs_geometry_update_flag()
        { m_geo_update_flag = false; }
private:
    bool m_geo_update_flag = false;
};

#if 0
class ResizableWidget : public Widget {
public:
    static constexpr const int k_widget_chooses_size = -1;
    void set_size(int width, int height) {
        set_needs_geometry_update_flag();
        set_size_(width, height);
    }

protected:
    virtual void set_size_(int width, int height) = 0;
};
#endif
template <typename Func>
class ChildIteratorFunctor final : public ChildWidgetIterator {
public:
    explicit ChildIteratorFunctor(Func && f_): f(std::move(f_)) {}
    void on_child_(Widget & widget) override { f(widget); }
    void on_const_child_(const Widget &) {
        throw std::runtime_error(
            "ChildIteratorFunctor::on_const_child_: called constant "
            "on_const_child_, which is not desired by this type.");
    }
    Func f;
};

template <typename Func>
class ConstChildIteratorFunctor final : public ChildWidgetIterator {
public:
    explicit ConstChildIteratorFunctor(Func && f_): f(std::move(f_)) {}
    void on_child_(Widget &) override {
        throw std::runtime_error(
            "ConstChildIteratorFunctor::on_child_: called non-constant "
            "on_child_, which is not desired by this type.");
    }
    void on_const_child_(const Widget & widget) override { f(widget); }

    Func f;
};

template <typename Func>
void Widget::iterate_children_f(Func && f) {
    ChildIteratorFunctor<Func> itr(std::move(f));
    iterate_children(itr);
}

template <typename Func>
void Widget::iterate_children_const_f(Func && f) const {
    ConstChildIteratorFunctor<Func> itr(std::move(f));
    iterate_children(itr);
}

} // end of asgl namespace
