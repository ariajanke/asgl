/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#pragma once

#include <common/Grid.hpp>

template <bool k_is_const_t, typename T>
class SubGridIteratorImpl;

/** @brief SubGrid class allows selecting a writable/readable reference to a 
 *         chunk of a grid.
 *  @note That the specializations (const SubGrid<T> &), (ConstSubGrid<T> &),
 *        and (const ConstSubGrid<T> &) all essentially describe the same type, 
 *        set of behaviors, and restrictions.
 *  It is possible to create sub grids from other sub grids. It is possible to 
 *  create constant sub grids from
 */
template <bool k_is_const_t, typename T>
class SubGridImpl {
    struct Dummy {};
public:
    friend class SubGridImpl<!k_is_const_t, T>;
    using ParentPointer      = typename std::conditional<k_is_const_t, const Grid<T> *, Grid<T> *>::type;
    using ParentReference    = typename std::conditional<k_is_const_t, const Grid<T> &, Grid<T> &>::type;
    using Element            = typename Grid<T>::Element;
    using Reference          = typename Grid<T>::ReferenceType;
    using ConstReference     = typename Grid<T>::ConstReferenceType;
    using Iterator           = SubGridIteratorImpl<k_is_const_t, T>;
    using ConstIterator      = SubGridIteratorImpl<true, T>;
    using VectorI            = sf::Vector2i;

    static constexpr const bool k_is_const = k_is_const_t;

    /** special sentinel value for width, and height parameters meaning "extend
     *  the length to the end of the parent" (whether sub grid or root parent 
     *  grid)
     */
    static constexpr const int  k_rest_of_grid = -1;

    /** The default sub grid, is zero sized and has no parent. This is 
     *  essentially useless.
     */
    SubGridImpl() {}
    
    /** Constructs a constant sub grid from a writable sub grid.
     *  This constructor is disabled for other const sub grid in favor of the
     *  copy constructor.
     *  @note point of interest: This construct is "deleted" by way of selecting
     *        a type for this constructor in the other template case. This type
     *        is then made inaccessible by making it private.
     */
    SubGridImpl(typename std::conditional<k_is_const, const SubGridImpl<false, T> &, Dummy>::type);

    SubGridImpl(const SubGridImpl &) = default;
    
    SubGridImpl(SubGridImpl &&) = default;

    /** @brief Constructs a usable sub grid, optionally with a width and height
     *  @param parent container
     *  @param width_ width of the new sub grid, by default k_rest_of_grid
     *                extends this grid out to the width of the parent
     *  @param height_ height of the new sub grid, by default k_rest_of_grid
                       extends this grid out to the height of the parent
     *  @throws if the width or height does not fit within the parent
     */
    SubGridImpl(ParentReference parent,
                int width_ = k_rest_of_grid, int height_ = k_rest_of_grid);
    
    /** @brief Constructs a usable sub grid, optionally with a width and height
     *  @param parent container
     *  @param offset is the start of the sub grid relative to the parent
     *  @param width_ width of the new sub grid, by default k_rest_of_grid
     *                extends this grid out to the width of the parent
     *  @param height_ height of the new sub grid, by default k_rest_of_grid
                       extends this grid out to the height of the parent
     *  @throws if the width or height does not fit within the parent minus the 
     *          offset
     */
    SubGridImpl(ParentReference, VectorI offset,
                int width_ = k_rest_of_grid, int height_ = k_rest_of_grid);

    SubGridImpl & operator = (const SubGridImpl &) = default;
    
    SubGridImpl & operator = (SubGridImpl &&) = default;

    /** @returns a constant reference to the parent container */
    const Grid<T> & parent() const { return *m_parent; }

    template <bool k_is_const_ = k_is_const_t>
    typename std::enable_if<!k_is_const_, Reference>::type operator () (const VectorI & r) { return element(r.x, r.y); }

    ConstReference operator () (const VectorI & r) const { return element(r.x, r.y); }

    template <bool k_is_const_ = k_is_const_t>
    typename std::enable_if<!k_is_const_, Reference>::type operator () (int x, int y) { return element(x, y); }

    ConstReference operator () (int x, int y) const { return element(x, y); }

    std::size_t size() const noexcept { return std::size_t(m_width*m_height); }

    bool is_empty() const noexcept { return m_width == 0 && m_height == 0; }

    /** @return grid width in number of elements */
    int width() const noexcept { return m_width; }

    /** @return grid height in number of elements */
    int height() const noexcept { return m_height; }

    /** @returns true if position is inside the grid */
    bool has_position(int x, int y) const noexcept;

    /** @returns true if position is inside the grid */
    bool has_position(const sf::Vector2i & r) const noexcept
        { return has_position(r.x, r.y); }

    sf::Vector2i next(const sf::Vector2i &) const noexcept;

    sf::Vector2i end_position() const noexcept;

    SubGridImpl<true, T> make_sub_grid
        (int width = k_rest_of_grid, int height = k_rest_of_grid) const;

    SubGridImpl<true, T> make_sub_grid
        (VectorI offset, int width = k_rest_of_grid, int height = k_rest_of_grid) const;

    template <bool k_is_const_ = k_is_const_t>
    typename std::enable_if<!k_is_const_, SubGridImpl<false, T>>::type make_sub_grid
        (int width = k_rest_of_grid, int height = k_rest_of_grid);

    template <bool k_is_const_ = k_is_const_t>
    typename std::enable_if<!k_is_const_, SubGridImpl<false, T>>::type make_sub_grid
        (VectorI offset, int width = k_rest_of_grid, int height = k_rest_of_grid);

    bool sub_grid_will_fit
        (VectorI offset,
         int width = k_rest_of_grid, int height = k_rest_of_grid) const noexcept;

    void swap(SubGridImpl<k_is_const_t, T> &) noexcept;

    Iterator begin();

    Iterator end();

    ConstIterator begin() const;

    ConstIterator end() const;
    
private:
    template <bool k_is_const_ = k_is_const_t>
    typename std::enable_if<!k_is_const_, Reference>::type
        element(int x, int y)
    {
        verify_position_ok(x, y);
        return (*m_parent)(x + m_offset.x, y + m_offset.y);
    }

    ConstReference element(int x, int y) const;

    const Element * begin_ptr() const;

    const Element * end_ptr() const;

    void verify_position_ok(int x, int y) const;

    void verify_sub_grid_will_fit(VectorI offset, int width_, int height_) const;

    void verify_invarients() const;

    static VectorI verify_offset(ParentReference, VectorI);

    static int verify_size(int max, int size, const char * name);

    VectorI m_offset;
    int m_width = 0;
    int m_height = 0;
    ParentPointer m_parent = nullptr;
};

template <typename T>
using SubGrid = SubGridImpl<false, T>;

template <typename T>
using ConstSubGrid = SubGridImpl<true, T>;

// ------------------------ make_sub_grid for Grid type ------------------------

template <typename T>
SubGrid<T> make_sub_grid
    (Grid<T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return SubGrid<T>(parent, offset, width_, height_); }

template <typename T>
SubGrid<T> make_sub_grid
    (Grid<T> & parent, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return SubGrid<T>(parent, sf::Vector2i(), width_, height_); }

template <typename T>
ConstSubGrid<T> make_sub_grid
    (const Grid<T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return ConstSubGrid<T>(parent, offset, width_, height_); }

template <typename T>
ConstSubGrid<T> make_sub_grid
    (const Grid<T> & parent, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return ConstSubGrid<T>(parent, sf::Vector2i(), width_, height_); }

template <typename T>
ConstSubGrid<T> make_const_sub_grid
    (Grid<T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return ConstSubGrid<T>(parent, offset, width_, height_); }

template <typename T>
ConstSubGrid<T> make_const_sub_grid
    (Grid<T> & parent, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return ConstSubGrid<T>(parent, sf::Vector2i(), width_, height_); }

template <typename T>
ConstSubGrid<T> make_const_sub_grid
    (const Grid<T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return make_sub_grid(parent, offset, width_, height_); }

template <typename T>
ConstSubGrid<T> make_const_sub_grid
    (const Grid<T> & parent, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return make_sub_grid(parent, width_, height_); }

// ----------------------- make_sub_grid for SubGrid type ----------------------

template <bool k_is_const_t, typename T>
SubGridImpl<k_is_const_t, T> make_sub_grid
    (SubGridImpl<k_is_const_t, T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(offset, width_, height_); }

template <bool k_is_const_t, typename T>
SubGridImpl<k_is_const_t, T> make_sub_grid
    (SubGridImpl<k_is_const_t, T> & parent,
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_sub_grid
    (const SubGridImpl<k_is_const_t, T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(offset, width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_sub_grid
    (const SubGridImpl<k_is_const_t, T> & parent,
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_const_sub_grid
    (SubGridImpl<k_is_const_t, T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(offset, width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_const_sub_grid
    (SubGridImpl<k_is_const_t, T> & parent,
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_const_sub_grid
    (const SubGridImpl<k_is_const_t, T> & parent, sf::Vector2i offset, 
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(offset, width_, height_); }

template <bool k_is_const_t, typename T>
ConstSubGrid<T> make_const_sub_grid
    (const SubGridImpl<k_is_const_t, T> & parent,
     int width_  = SubGrid<T>::k_rest_of_grid, 
     int height_ = SubGrid<T>::k_rest_of_grid)
{ return parent.make_sub_grid(width_, height_); }

// ----------------------------------------------------------------------------

/** Special iterator class for subgrids.
 *
 *  You every wish to use standard algorithms, and sequence operations on
 *  <em>your</em> subgrid? You're in luck, this bidirectional will enable them.
 */
template <bool k_is_const_t, typename T>
class SubGridIteratorImpl {
public:
    using Element       = T;
    using Pointer       = std::conditional_t<k_is_const_t, const T *, T *>;
    using Reference     = std::conditional_t<k_is_const_t, const T &, T &>;
    using ParentPointer = typename std::conditional<k_is_const_t, const Grid<T> *, Grid<T> *>::type;

    SubGridIteratorImpl() {}

    /** Constructor specific for SubGrid, this is not designed for public calls.
     *  Use the begin and end methods instead.
     *  @param parent pointer to the parent container
     *  @param element_ptr pointer to the current element for the iterator to
     *                     point to
     *  @param subgrid_row_size parent subgrid's width
     *  @param subgrid_pos the row position (column) of the current element
     *                     pointer, which was given as element_ptr
     */
    SubGridIteratorImpl
        (ParentPointer parent, Pointer element_ptr, int subgrid_row_size,
         int subgrid_pos):
        m_ptr(element_ptr), m_row_pos(subgrid_pos), m_row_size(subgrid_row_size),
        m_row_jump(parent->width())
    {}

    SubGridIteratorImpl(const SubGridIteratorImpl &) = default;
    SubGridIteratorImpl(SubGridIteratorImpl &&) = default;

    ~SubGridIteratorImpl() {}

    SubGridIteratorImpl & operator = (const SubGridIteratorImpl &) = default;
    SubGridIteratorImpl & operator = (SubGridIteratorImpl &&) = default;

    SubGridIteratorImpl & operator ++ () { return move_position(1); }

    SubGridIteratorImpl operator ++ (int) { return move_post(1); }

    SubGridIteratorImpl & operator -- () { return move_position(-1); }

    SubGridIteratorImpl operator -- (int) { return move_post(-1); }

    Pointer operator -> () const noexcept { return m_ptr; }

    Reference operator * () const noexcept { return *m_ptr; }

    /** Advances (and regresses) this iterators position by some give number of
     *  places.
     *  @param amount number of places to move this iterator
     *  @returns reference to this
     */
    SubGridIteratorImpl & move_position(int amount)
        { return amount > 0 ? move_forward(amount) : move_backward(-amount); }

    bool operator == (const SubGridIteratorImpl & rhs) const noexcept
        { return is_same(rhs); }

    bool operator != (const SubGridIteratorImpl & rhs) const noexcept
        { return !is_same(rhs); }

    using difference_type   = int;
    using value_type        = Element;
    using pointer           = Pointer;
    using reference         = Reference;
    using iterator_category = std::bidirectional_iterator_tag;

private:
    static constexpr const int k_no_size = 0;

    bool is_same(const SubGridIteratorImpl & rhs) const noexcept;

    SubGridIteratorImpl move_post(int amount);

    SubGridIteratorImpl & move_forward(int amount);

    SubGridIteratorImpl & move_backward(int amount);

    static void verify_non_negative_integer(const char * caller, int amt);

    void verify_can_move_position(const char * caller) const;

    Pointer m_ptr = nullptr;
    int m_row_pos = 0;

    const int m_row_size = k_no_size;
    const int m_row_jump = k_no_size;
};

// <------------------------ END OF PUBLIC INTERFACE ------------------------->

template <bool k_is_const_t, typename T>
SubGridImpl<k_is_const_t, T>::SubGridImpl
    (typename std::conditional<k_is_const, const SubGridImpl<false, T> &, Dummy>::type rhs):
    m_offset(rhs.m_offset),
    m_width (rhs.m_width ),
    m_height(rhs.m_height),
    m_parent(rhs.m_parent)
{ verify_invarients(); }

template <bool k_is_const_t, typename T>
SubGridImpl<k_is_const_t, T>::SubGridImpl
    (ParentReference parent, int width_, int height_):
    SubGridImpl(parent, VectorI(), width_, height_)
{ verify_invarients(); }

template <bool k_is_const_t, typename T>
SubGridImpl<k_is_const_t, T>::SubGridImpl
    (ParentReference parent, VectorI offset, int width_, int height_):
    m_offset(verify_offset(parent, offset)),
    m_width (verify_size(parent.width () - offset.x, width_ , "width" )),
    m_height(verify_size(parent.height() - offset.y, height_, "height")),
    m_parent(&parent)
{ verify_invarients(); }

template <bool k_is_const_t, typename T>
bool SubGridImpl<k_is_const_t, T>::has_position(int x, int y) const noexcept
    { return (x >= 0 && y >= 0 && x < width() && y < height()); }

template <bool k_is_const_t, typename T>
sf::Vector2i SubGridImpl<k_is_const_t, T>::next
    (const sf::Vector2i & r) const noexcept
{
    auto rv = r;
    if (++rv.x == width()) {
        ++rv.y;
        rv.x = 0;
    }
    return rv;
}

template <bool k_is_const_t, typename T>
sf::Vector2i SubGridImpl<k_is_const_t, T>::end_position() const noexcept
    { return VectorI(0, height()); }

template <bool k_is_const_t, typename T>
SubGridImpl<true, T> SubGridImpl<k_is_const_t, T>::make_sub_grid
    (int width_, int height_) const
    { return make_sub_grid(VectorI(), width_, height_); }

template <bool k_is_const_t, typename T>
SubGridImpl<true, T> SubGridImpl<k_is_const_t, T>::
    make_sub_grid(VectorI offset, int width_, int height_) const
{
    SubGridImpl<true, T> rv;
    verify_sub_grid_will_fit(offset, width_, height_);
    rv.m_parent = m_parent;
    rv.m_offset = m_offset + offset;
    rv.m_width  = width_  == k_rest_of_grid ? width () - offset.x : width_ ;
    rv.m_height = height_ == k_rest_of_grid ? height() - offset.y : height_;
    verify_invarients();
    return rv;
}

template <bool k_is_const_t, typename T>
template <bool k_is_const_>
typename std::enable_if<!k_is_const_, SubGridImpl<false, T>>::type
    SubGridImpl<k_is_const_t, T>::make_sub_grid
    (int width_, int height_)
    { return make_sub_grid(VectorI(), width_, height_); }

template <bool k_is_const_t, typename T>
template <bool k_is_const_>
typename std::enable_if<!k_is_const_, SubGridImpl<false, T>>::type
    SubGridImpl<k_is_const_t, T>::make_sub_grid
    (VectorI offset, int width_, int height_)
{
    SubGridImpl<k_is_const_, T> rv;
    verify_sub_grid_will_fit(offset, width_, height_);
    rv.m_parent = m_parent;
    rv.m_offset = m_offset + offset;
    rv.m_width  = width_  == k_rest_of_grid ? width () - offset.x : width_ ;
    rv.m_height = height_ == k_rest_of_grid ? height() - offset.y : height_;
    verify_invarients();
    return rv;
}

template <bool k_is_const_t, typename T>
bool SubGridImpl<k_is_const_t, T>::sub_grid_will_fit
    (VectorI offset, int width_, int height_) const noexcept
{
    return
        (offset.x >= 0 && offset.x < width ()) &&
        (offset.y >= 0 && offset.y < height()) &&
        (width_  <= width () - offset.x || width_ == k_rest_of_grid) &&
        (height_ <= height() - offset.y || width_ == k_rest_of_grid);
}

template <bool k_is_const_t, typename T>
void SubGridImpl<k_is_const_t, T>::swap
    (SubGridImpl<k_is_const_t, T> & rhs) noexcept
{
    std::swap(m_offset, rhs.m_offset);
    std::swap(m_width , rhs.m_width );
    std::swap(m_height, rhs.m_height);
    std::swap(m_parent, rhs.m_parent);
    verify_invarients();
}

template <bool k_is_const_t, typename T>
typename SubGridImpl<k_is_const_t, T>::Iterator SubGridImpl<k_is_const_t, T>::begin() {
    if (is_empty()) return Iterator();
    return Iterator(m_parent, const_cast<Element *>(begin_ptr()), m_width, 0);
}

template <bool k_is_const_t, typename T>
typename SubGridImpl<k_is_const_t, T>::Iterator SubGridImpl<k_is_const_t, T>::end() {
    if (is_empty()) return Iterator();
    return Iterator(m_parent, const_cast<Element *>(end_ptr()), m_width, 0);
}

template <bool k_is_const_t, typename T>
typename SubGridImpl<k_is_const_t, T>::ConstIterator SubGridImpl<k_is_const_t, T>::begin() const {
    if (is_empty()) return ConstIterator();
    return ConstIterator(m_parent, begin_ptr(), m_width, 0);
}

template <bool k_is_const_t, typename T>
typename SubGridImpl<k_is_const_t, T>::ConstIterator SubGridImpl<k_is_const_t, T>::end() const {
    if (is_empty()) return ConstIterator();
    return ConstIterator(m_parent, end_ptr(), m_width, 0);
}

template <bool k_is_const_t, typename T>
typename SubGridImpl<k_is_const_t, T>::ConstReference
    SubGridImpl<k_is_const_t, T>::element(int x, int y) const
{
    verify_position_ok(x, y);
    return (*m_parent)(x + m_offset.x, y + m_offset.y);
}

template <bool k_is_const_t, typename T>
const typename SubGridImpl<k_is_const_t, T>::Element *
    SubGridImpl<k_is_const_t, T>::begin_ptr() const
{
    if (is_empty()) throw std::runtime_error("begin_ptr: must be non-empty subgrid.");
    return &(*m_parent)(m_offset);
}

template <bool k_is_const_t, typename T>
const typename SubGridImpl<k_is_const_t, T>::Element *
    SubGridImpl<k_is_const_t, T>::end_ptr() const
{
    auto beg_ptr = begin_ptr(); // checks for empty
    return beg_ptr + m_parent->width()*m_height;
}

template <bool k_is_const_t, typename T>
/* private */ void SubGridImpl<k_is_const_t, T>::
    verify_position_ok(int x, int y) const
{
    if (has_position(x, y)) return;
    throw std::out_of_range("Position out of range.");
}

template <bool k_is_const_t, typename T>
/* private */ void SubGridImpl<k_is_const_t, T>::verify_sub_grid_will_fit
    (VectorI offset, int width_, int height_) const
{
    if (sub_grid_will_fit(offset, width_, height_)) return;
    throw std::invalid_argument("Sub grid will not fit.");
}

template <bool k_is_const_t, typename T>
/* private static */ sf::Vector2i SubGridImpl<k_is_const_t, T>::
    verify_offset(ParentReference parent, VectorI offset)
{
    if (parent.has_position(offset) || offset == parent.end_position()) return offset;
    throw std::out_of_range("Offset not contained in parent.");
}

template <bool k_is_const_t, typename T>
/* private static */ int SubGridImpl<k_is_const_t, T>::verify_size
    (int max, int size, const char * name)
{
    if (size == k_rest_of_grid) return max;
    if (size >= 0 && size <= max) return size;
    throw std::out_of_range("Sub grid " + std::string(name) +
                            " cannot fit inside the parent container.");
}

template <bool k_is_const_t, typename T>
/* private */ void SubGridImpl<k_is_const_t, T>::verify_invarients() const {
    bool invarients_ok = false;
    if (m_parent) {
        invarients_ok =
            m_offset.x >= 0 && m_offset.x <= m_parent->width () &&
            m_offset.y >= 0 && m_offset.y <= m_parent->height() &&
            m_width  >= 0 && m_offset.x + m_width  <= m_parent->width () &&
            m_height >= 0 && m_offset.y + m_height <= m_parent->height();
    } else {
        invarients_ok = m_width == 0 && m_height == 0 && m_offset == VectorI();
    }
    if (!invarients_ok) {
        throw std::runtime_error("SubGridImpl::verify_invarients: invarients failed.");
    }
}

// ----------------------------------------------------------------------------

template <bool k_is_const_t, typename T>
/* private */ bool SubGridIteratorImpl<k_is_const_t, T>::is_same
    (const SubGridIteratorImpl & rhs) const noexcept
{
    return    m_ptr     == rhs.m_ptr     && m_row_size == rhs.m_row_size
           && m_row_pos == rhs.m_row_pos && m_row_jump == rhs.m_row_jump;
}

template <bool k_is_const_t, typename T>
/* private */ SubGridIteratorImpl<k_is_const_t, T>
    SubGridIteratorImpl<k_is_const_t, T>::move_post(int amount)
{
    auto t = *this;
    move_position(amount);
    return t;
}

template <bool k_is_const_t, typename T>
/* private */ SubGridIteratorImpl<k_is_const_t, T> &
    SubGridIteratorImpl<k_is_const_t, T>::move_forward(int amount)
{
    verify_non_negative_integer("move_forward", amount);
    verify_can_move_position   ("move_forward");

    auto row_changes = (m_row_pos + amount) / m_row_size;
    auto new_row_pos = (m_row_pos + amount) % m_row_size;
    // got to beginning of row, jump n rows, then jump to new position
    m_ptr     = (m_ptr - m_row_pos) + row_changes*m_row_jump + new_row_pos;
    m_row_pos = new_row_pos;

    return *this;
}

template <bool k_is_const_t, typename T>
/* private */ SubGridIteratorImpl<k_is_const_t, T> &
    SubGridIteratorImpl<k_is_const_t, T>::move_backward(int amount)
{
    verify_non_negative_integer("move_backward", amount);
    verify_can_move_position   ("move_backward");

    int row_changes = 0;
    int new_row_pos = 0;
    if (amount > m_row_pos) {
        auto rows_covered = (amount - m_row_pos) / m_row_size;
        row_changes = rows_covered + 1;
        new_row_pos = m_row_size - ((amount - m_row_pos) - rows_covered*m_row_size);
    } else {
        new_row_pos = m_row_pos - amount;
    }
    m_ptr     = (m_ptr - m_row_pos) - row_changes*m_row_jump + new_row_pos;
    m_row_pos = new_row_pos;
    return *this;
}

template <bool k_is_const_t, typename T>
/* private static */ void SubGridIteratorImpl<k_is_const_t, T>::
    verify_non_negative_integer(const char * caller, int amt)
{
    if (amt >= 0) return;
    throw std::invalid_argument(std::string(caller)
                                + ": amount must be a non-negative integer.");
}

template <bool k_is_const_t, typename T>
/* private */ void SubGridIteratorImpl<k_is_const_t, T>::
    verify_can_move_position(const char * caller) const
{
    if (m_row_size != k_no_size && m_row_jump != k_no_size) return;
    throw std::runtime_error(std::string(caller)
                             + ": cannot move iterator's position without "
                               "a subgrid row size, and parent grid width.");
}
