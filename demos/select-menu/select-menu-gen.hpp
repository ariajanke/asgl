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

#include <asgl/Frame.hpp>

#include <common/Grid.hpp>

namespace asgl {

class SelectionEntryDecoration final : public BlankDecorationBase {
public:
    void accept_any_size();

    void constrain_to(const SelectionEntryDecoration * ptr);

    Size request_size(int w, int h) final;

    void reset_sizing();

private:
    void check_invarients() const;

    void on_inform_is_child() final {}

    bool m_has_accepted_a_size = false;
    bool m_accepting_any_size = false;
    const SelectionEntryDecoration * m_constraint = nullptr;
};

class DrawableWithWidgetRenderer {
public:
    virtual ~DrawableWithWidgetRenderer() {}

    virtual bool is_requesting_drop() const = 0;

    void draw(const Widget & parent, WidgetRenderer & target) const
        { draw_(parent.location(), target); }

    void draw(const Vector & offset, WidgetRenderer & target) const
        { draw_(offset, target); }

protected:
    virtual void draw_(const Vector &, WidgetRenderer &) const = 0;
};

template <bool kt_is_const, typename T>
class SelectMenuIteratorImpl;

// ----------------------------------------------------------------------------

class SelectionEntryFrame : public BareFrame/*, public FocusReceiver*/ {
public:
    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    void accept_any_size() { m_deco.accept_any_size(); }

    void constrain_to(const SelectionEntryFrame * ptr);

    virtual Tuple<const DrawableWithWidgetRenderer *, const Widget *> get_hold_request() const = 0;

protected:
    SelectionEntryFrame() {}

private:
    FrameDecoration & decoration() final { return m_deco; }

    const FrameDecoration & decoration() const final { return m_deco; }

    SelectionEntryDecoration m_deco;
};

class SelectionMenuBase : public Widget {
public:
    void set_menu_size(int w, int h);

    void set_padding(int i);

    Vector location() const final;

    Size size() const final;

    void stylize(const StyleMap & map) override;

    void update_size() override;

    void process_event(const Event & event) override;

    void draw(WidgetRenderer & target) const override;

    void fix_internal_iterators();

    Vector end_position() const { return m_icons.end_position(); }

    Vector next(Vector r) const { return m_icons.next(r); }

    Size menu_size() const { return Size(m_icons.width(), m_icons.height()); }

    bool has_position(Vector r) const { return m_icons.has_position(r); }

protected:
    template <typename ... Types>
    using Tuple = std::tuple<Types...>;

    template <bool kt_is_const, typename T>
    friend class SelectMenuIteratorImpl;

    struct GridEntry {
        GridEntry() {}
        GridEntry(void * derv_, SelectionEntryFrame * base_);
        operator bool () const noexcept
            { return derived; }
        void * derived = nullptr;
        SelectionEntryFrame * base = nullptr;
    };

    void place(Vector r, void * ptr, SelectionEntryFrame * frame);

    virtual Tuple<GridEntry, GridEntry> on_place_item_on(void *, void *) = 0;

    GridEntry * begin_() { return m_icons.is_empty() ? nullptr : &*m_icons.begin(); }

    GridEntry * end_() { return begin_() + m_icons.size(); }

    const GridEntry * begin_() const { return m_icons.is_empty() ? nullptr : &*m_icons.begin(); }

    const GridEntry * end_() const { return begin_() + m_icons.size(); }

private:
    void iterate_children_(const ChildWidgetIterator &) final;

    void iterate_children_const_(const ChildConstWidgetIterator &) const final;

    void set_location_(int x, int y) final;

    Vector pixel_point_to_grid_cell(Vector r) const;

    Size size_in_padding() const;

    cul::Grid<GridEntry> m_icons;

    GridEntry * m_frame_being_dragged = nullptr;
    const Widget * m_held_items_parent = nullptr;
    const DrawableWithWidgetRenderer * m_held_item = nullptr;

    Vector m_widget_location;
    int m_padding = 0;
    cul::Grid<GridEntry>::Iterator m_non_null_itr = m_icons.end();
};

template <bool kt_is_const, typename T>
class SelectMenuIteratorImpl {
private:
    using GridEntry   = SelectionMenuBase::GridEntry;
    using PointerImpl = std::conditional_t<kt_is_const, const GridEntry *, GridEntry *>;

public:
    class Object {
    public:
        using Pointer = std::conditional_t<kt_is_const, const T *, T *>;

        Object(): m_hidden(&m_intrn) {}

        Object(const Object & rhs) = delete;

        Object(Object && rhs):
            m_hidden(&m_intrn),
            m_intrn(std::move(*rhs.m_hidden))
        {}

        Object & operator = (const Object & rhs) = delete;

        Object & operator = (Object && rhs) {
            *m_hidden = std::move(*rhs.m_hidden);
            return *this;
        }

        operator Pointer () const { return reinterpret_cast<Pointer>(m_hidden->derived); }

        operator bool    () const { return bool(m_hidden->derived); }

    private:
        friend class SelectMenuIteratorImpl;
        explicit Object(PointerImpl hidden): m_hidden(hidden) {}
        PointerImpl m_hidden;
        // This blows up iterator to three pointers in size,
        // but this is needed for the swap function to work
        GridEntry m_intrn;
    };

    // iterator traits for STL
    using value_type        = Object;
    using difference_type   = int;
    using iterator_category = std::random_access_iterator_tag;
    using pointer           = std::conditional_t<kt_is_const, const Object *, Object *>;
    using reference         = std::conditional_t<kt_is_const, const Object &, Object &>;

    explicit SelectMenuIteratorImpl(PointerImpl itr): m_as_obj(itr) {}

    SelectMenuIteratorImpl(const SelectMenuIteratorImpl & rhs): m_as_obj(rhs.m_as_obj.m_hidden) {}

    SelectMenuIteratorImpl(SelectMenuIteratorImpl && rhs): m_as_obj(std::move(rhs.m_as_obj.m_hidden)) {}

    SelectMenuIteratorImpl & operator = (const SelectMenuIteratorImpl & rhs) {
        m_as_obj.m_hidden = rhs.m_as_obj.m_hidden;
        return *this;
    }

    SelectMenuIteratorImpl & operator = (SelectMenuIteratorImpl && rhs) {
        m_as_obj.m_hidden = std::move(rhs.m_as_obj.m_hidden);
        return *this;
    }

    // does this *really* need to be a reference?
    const Object & operator * () const { return m_as_obj; }

    typename std::enable_if<!kt_is_const, Object &>::type operator * () { return m_as_obj; }

    // all the crap needed for random access iterators

    Object operator [] (int n) const { return Object(*(m_as_obj.m_hidden + n)); }

    SelectMenuIteratorImpl offset(int n) const
        { return SelectMenuIteratorImpl(m_as_obj.m_hidden + n); }

    SelectMenuIteratorImpl & move(int n) {
        m_as_obj.m_hidden += n;
        return *this;
    }

    int compare(const SelectMenuIteratorImpl & rhs) const
        { return m_as_obj.m_hidden - rhs.m_as_obj.m_hidden; }

    SelectMenuIteratorImpl & operator += (int n) { return move(n); }

    SelectMenuIteratorImpl & operator -= (int n) { return move(-n); }

    bool operator < (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) < 0; }

    bool operator > (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) > 0; }

    bool operator <= (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) <= 0; }

    bool operator >= (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) >= 0; }

    bool operator == (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) == 0; }

    bool operator != (const SelectMenuIteratorImpl & rhs) const
        { return compare(rhs) != 0; }

    SelectMenuIteratorImpl & operator ++ () { return move(1); }

    SelectMenuIteratorImpl operator ++ (int) const {
        SelectMenuIteratorImpl temp;
        return (++temp);
    }

    SelectMenuIteratorImpl & operator -- () { return move(-1); }

    SelectMenuIteratorImpl operator -- (int) const {
        SelectMenuIteratorImpl temp;
        return (++temp);
    }

private:
    Object m_as_obj;
};

template <bool kt_is_const, typename T>
asgl::SelectMenuIteratorImpl<kt_is_const, T> operator +
    (const asgl::SelectMenuIteratorImpl<kt_is_const, T> & lhs, int rhs)
{
    auto temp = lhs;
    temp.move(rhs);
    return temp;
}

template <bool kt_is_const, typename T>
asgl::SelectMenuIteratorImpl<kt_is_const, T> operator -
    (const asgl::SelectMenuIteratorImpl<kt_is_const, T> & lhs, int rhs)
{
    auto temp = lhs;
    temp.move(-rhs);
    return temp;
}

template <bool kt_is_const, typename T>
asgl::SelectMenuIteratorImpl<kt_is_const, T> operator +
    (int lhs, const asgl::SelectMenuIteratorImpl<kt_is_const, T> & rhs)
{
    auto temp = rhs;
    temp.move(lhs);
    return temp;
}

template <bool kt_is_const, typename T>
int operator -
    (const asgl::SelectMenuIteratorImpl<kt_is_const, T> & lhs,
     const asgl::SelectMenuIteratorImpl<kt_is_const, T> & rhs)
{ return lhs.compare(rhs); }

// ----------------------------------------------------------------------------

template <typename T>
class SelectionMenu : public SelectionMenuBase {
public:
    static_assert(std::is_base_of_v<SelectionEntryFrame, T>, "");
    using Iterator      = SelectMenuIteratorImpl<false, T>;
    using ConstIterator = SelectMenuIteratorImpl<true , T>;

    void place(Vector r, T * ptr) { SelectionMenuBase::place(r, ptr, ptr); }

    virtual Tuple<T *, T *> on_place_item_on(T & a, T * b) {
        // reminder this code can always be changed!
        return std::make_tuple(b, &a);
    }

    virtual void on_item_pick_up(T &) {}

    // if you rearrange any elements, make sure to flag this widget for an
    // individual geometry update

    Iterator begin() { return Iterator(begin_()); }

    Iterator end() { return Iterator(end_()); }

    ConstIterator begin() const { return ConstIterator(begin_()); }

    ConstIterator end() const { return ConstIterator(end_()); }

protected:
    SelectionMenu() {}

private:
    // explicitly hide this function
    using SelectionMenuBase::place;

    Tuple<GridEntry, GridEntry> on_place_item_on(void * a, void * b) final {
        using std::get;
        if (!a) { throw std::invalid_argument("my caller shouldn't pass null for a!"); }
        T & a_as_derv = *reinterpret_cast<T *>(a);
        T * b_as_derv =  reinterpret_cast<T *>(b);
        auto gv = on_place_item_on(a_as_derv, b_as_derv);
        return std::make_tuple(GridEntry(get<0>(gv), get<0>(gv)),
                               GridEntry(get<1>(gv), get<1>(gv)));
    }
};

} // end of asgl namespace

//using namespace asgl::operators;
