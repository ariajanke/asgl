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

#include <asgl/StyleMap.hpp> // we need to know ItemKey
#include <asgl/Event.hpp>

#include <SFML/Graphics/Rect.hpp>

#include <vector>

namespace asgl {

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
    virtual void operator () (Widget &) const {}

protected:
    ChildWidgetIterator() {}
};

/** @brief Much like ChildWidgetIterator but for constant widgets instead.
 *
 */
class ChildConstWidgetIterator {
public:
    virtual ~ChildConstWidgetIterator();
    virtual void operator () (const Widget &) const {}

protected:
    ChildConstWidgetIterator() {}
};

class WidgetFlagsReceiver {
public:
    virtual ~WidgetFlagsReceiver();
    virtual void receive_whole_family_upate_needed() = 0;

    // this design requires passing a "this" pointer
    virtual void receive_individual_update_needed(Widget *) = 0;

    static WidgetFlagsReceiver & null_instance();
};

class TextBase;

// ----------------------------------------------------------------------------

using TriangleTuple = std::tuple<sf::Vector2i, sf::Vector2i, sf::Vector2i>;

class WidgetRenderer {
public:
    virtual ~WidgetRenderer();

    virtual void render_rectangle(const sf::IntRect   &, ItemKey, const void * widget_spec_ptr) = 0;
    virtual void render_triangle (const TriangleTuple &, ItemKey, const void * widget_spec_ptr) = 0;
    virtual void render_text(const TextBase &) = 0;
};

/** A frame needs four things from a widget, in order to position the widget
 *  and setup the frame.
 *
 *  === COMPOSITE PATTERN ===
 *  A Widget is a component, by default add_component (and future composite
 *  functions throws exceptions).
 *
 *  The following is a code snippet which maybe useful for copying when deriving
 *  from Widget:
 *  @code

class MyWidget final : public Widget {
public:
    void process_event(const Event &) override;

    VectorI location() const override;

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;

    void update_geometry() override;

    void draw(WidgetRenderer &) const override;

private:
    void set_location_(int x, int y) override;
};

 *  @endcode
 *
 *  @note on design:
 *  Exposing the individual geometry flag from the widget (as boolean) is not
 *  desired design especially the ability to reset the flag, as widget should
 *  not own its own flags/signals which it sent the flags receiver. @n
 *  Worse still, it allows clients to cancel and be exposed to more flagging
 *  logic than neccessary at the widget level. @n
 *  I *could* pass a boolean owned by the widget, but this incurs the same
 *  risks and problems as just passing the "this" pointer. @n
 *  Conclusion: just pass the this pointer, the flags receiver, should also be
 *  the owning frame/parent object.
 */
class Widget {
public:
    using VectorI = sf::Vector2i;

    virtual ~Widget();

    virtual void process_event(const Event &) = 0;

    void set_location(int x, int y);

    /** @returns the top left location of the widget */
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
    virtual void update_geometry() = 0;

    /** @brief Called by frame for automatic widget sizing.
     *  Widget computes its own size.
     *  @note This is called after styles are set, so fonts for widgets and
     *        others will be accessible on this call.
     *  @note The default behavior is for the widget to do nothing (no resize)
     *  @note Presently Frame and TextArea both use this function.
     */
    virtual void issue_auto_resize();

    virtual void draw(WidgetRenderer &) const = 0;

    template <typename Func>
    void iterate_children_f(Func &&);

    template <typename Func>
    void iterate_children_const_f(Func &&) const;

    void iterate_children(const ChildWidgetIterator &);
    void iterate_children(const ChildConstWidgetIterator &) const;

    /** Assigns the flags receiver pointer.
     *
     *  @warning This should be a pointer to the owning object, which means
     *           it should exist for the same duration as this widget
     *           (excluding either object's deconstruction)
     *  @param rec The receiver either must be the owning container, or the
     *             nullptr.
     */
    void assign_flags_receiver(WidgetFlagsReceiver * rec);

    struct Helpers {
        using FieldFindTuple = std::tuple<ItemKey *, const char *, const StyleField *>;

        static void handle_required_fields
            (const char * caller, std::initializer_list<FieldFindTuple> && fields);

        static ItemKey verify_item_key_field
            (const StyleField &, const char * full_caller, const char * key_name);

        static void verify_non_negative(int, const char * full_caller, const char * dim_name);

        static int verify_padding(const StyleField *, const char * full_caller);
    };

protected:
    virtual void iterate_children_(const ChildWidgetIterator &);

    virtual void iterate_children_const_(const ChildConstWidgetIterator &) const;

    virtual void set_location_(int x, int y) = 0;

    void draw_to(WidgetRenderer &, const sf::IntRect &, ItemKey) const;

    void draw_to(WidgetRenderer &, const TriangleTuple &, ItemKey) const;

    /** Set this flag if you need to resize the whole frame/family of widgets,
     *  in addition to local geometric computations.
     */
    void flag_needs_whole_family_geometry_update();

    /** Set this flag if you *only* need a widget local (no resizing) geometry
     *  update.
     *  @warning The geometry update *must not* resize the widget, doing so
     *           will cause the owning frame/parent object to throw a runtime
     *           exception.
     */
    void flag_needs_individual_geometry_update();

private:
    WidgetFlagsReceiver * m_flags_receiver = &WidgetFlagsReceiver::null_instance();
};

/** Describes a layer of the widget's parent of which individual widgets should
 *  be ignorant.
 *
 */
class WidgetFlagsReceiverWidget : public Widget, public WidgetFlagsReceiver {
public:
    /** Sets a flag that the whole family of widgets needs a geometry update.
     *  @note this should generally only be called by the Widget class, client
     *        coders should not have to worry about this function (at all!)
     */
    void receive_whole_family_upate_needed() final;

    /** Sets a flag that an individual widget needs a geometry update.
     *  @note This function requires that Widget passes this, therefore this
     *        instance really must be the owning parent object.
     *  @param wid pointer to the widget that needs an individual geometry
     *         update
     */
    void receive_individual_update_needed(Widget * wid) final;

protected:
    /** Unsets all geometry update flags for both the whole family and for
     *  individuals.
     *
     *  If the whole family didn't need an update, then this function will
     *  update geometry for individual widgets before clearing their individual
     *  flags.
     */
    void unset_flags();

    /** @returns true if the entire family of widgets needs to update
     *           geometry, false otherwise.
     */
    bool needs_whole_family_geometry_update() const;

private:
    static std::runtime_error make_size_changed_error(const char * caller) noexcept;

    std::vector<Widget *> m_individuals;
    bool m_geo_update_flag = false;
};

template <typename T>
sf::Vector2<T> get_top_left(const sf::Rect<T> &);

template <typename T>
void set_top_left(sf::Rect<T> &, const sf::Vector2<T> &);

template <typename T>
void set_top_left(sf::Rect<T> &, const T & x, const T & y);

// ----------------------------------------------------------------------------

template <typename Func>
class ChildIteratorFunctor final : public ChildWidgetIterator {
public:
    explicit ChildIteratorFunctor(Func && f_): f(std::move(f_)) {}
    void operator () (Widget & widget) const override { f(widget); }
    Func f;
};

template <typename Func>
class ConstChildIteratorFunctor final : public ChildConstWidgetIterator {
public:
    explicit ConstChildIteratorFunctor(Func && f_): f(std::move(f_)) {}
    void operator () (const Widget & widget) const override { f(widget); }
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

template <typename T>
sf::Vector2<T> get_top_left(const sf::Rect<T> & rect)
    { return sf::Vector2<T>(rect.left, rect.top); }

template <typename T>
void set_top_left(sf::Rect<T> & rect, const sf::Vector2<T> & r) {
    rect.left = r.x;
    rect.top  = r.y;
}

template <typename T>
void set_top_left(sf::Rect<T> & rect, const T & x, const T & y) {
    rect.left = x;
    rect.top  = y;
}

} // end of asgl namespace
