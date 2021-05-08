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

#include <type_traits>

#if 0 // WIP
#   include <array>
#endif

#include <common/TypeList.hpp>
#include <common/StorageUnion.hpp>

template <typename T> struct MultiTypeUpcastPair;
template <typename T> struct MultiTypeConstUpcastPair;

/** @brief A MultiType is an extension of StorageUnion.
 *
 *  It can store a single object at a time of any type indictated in the given
 *  parameter type pack. It includes functions that enables the client to
 *  perform C++ style casts (static_cast and dynamic_cast, though renamed since
 *  these are C++ keywords). @n
 *  In addition to the compile time component of refering and constructing by
 *  typenames, a runtime option is available in the from of a type integer id.
 *  The id for a typename can be found at compile, going in reverse however is
 *  not possible. @n
 *  A validity test function is provided, since by default, no object is
 *  contructed. @n
 *  @note I realize that std::varient is available in C++17...
 *
 *  @tparam Types "parameter type pack"
 */
template <typename ... Types>
class MultiType {
public:
    using MtTypeList = TypeList<Types...>;
    using Error = std::runtime_error;

    static constexpr const int k_no_type = TypeList<Types...>::k_not_any_type;
    static constexpr const int k_type_count = sizeof...(Types);

    template <typename Type>
    struct GetTypeId {
        static const int k_value = MtTypeList::template GetTypeId<Type>::k_value;
    };

    template <typename Type>
    struct HasType {
        static const bool k_value = MtTypeList::template HasType<Type>::k_value;
    };

    template <typename T>
    using UpcastPair = MultiTypeUpcastPair<T>;

    template <typename T>
    using ConstUpcastPair = MultiTypeConstUpcastPair<T>;

    /** No object is constructed by default, and the MultiType is "invalid". */
    MultiType(): m_current_type(k_no_type) {}

    template <typename T,
              typename = typename std::enable_if<HasType<T>::k_value>::type>
    explicit MultiType(const T & obj):
        m_current_type(GetTypeId<T>::k_value)
    { new (&m_store) T(obj); }

    /** Like any copy constructor, this one will copy the object that lives in
     *  rhs.
     *  @param rhs multiType whose object to copy
     */
    MultiType(const MultiType & rhs);

    ~MultiType();

    /** Like any assignment operator, this will copy the object that lives in
     *  rhs, however it will first destruct any object that happens to be
     *  living in this MultiType.
     *  @param rhs multiType whose object to copy
     *  @return Returns reference to this.
     */
    MultiType & operator = (const MultiType & rhs);

    /** @note simular to dynamic_cast in a sense
     *  @return Returns a pointer to the stored object, it'll return nullptr in
     *          two cases: no object is stored; the stored object is not of
     *          type T.
     */
    template <typename T>
    T * as_pointer() noexcept;

    /** @copydoc as_pointer() */
    template <typename T>
    const T * as_pointer() const noexcept;

    /** @see as_pointer()
     *  @throw  If the as_pointer function would return nullptr for the given
     *          type T, then this function will throw an exception (since it
     *          cannot return).
     *  @return Returns a reference to the stored object.
     */
    template <typename T>
    T & as();

    /** @copydoc as() */
    template <typename T>
    const T & as() const;

    /** Destructs any current object and constructs a new one with the given
     *  arguements for type T.
     *  @tparam T type of the new object to construct
     *  @param args arguments passed to new object's constructor
     *  @return Returns a reference to the newly created object.
     */
    template <typename T, typename ... ArgTypes>
    T & reset(ArgTypes &&... args);

    /** Deletes object the current object living in the multitype, which causes
     *  its type id to become 'invalid'.
     */
    void unset();

    /** @tparam T
     *  @return Returns true if the stored object is of type T.
     */
    template <typename T>
    bool is_type() const noexcept;

    /** @return Returns current type as a runtime friendly integer.
     *          The integer's value corresponds to the id in the
     *          TypeList's id (not any RTTI/other information).
     */
    int type_id() const noexcept { return m_current_type; }

    /** @return Returns true if the MultiType is storing an object, false if
     *          otherwise.
     */
    bool is_valid() const noexcept { return m_current_type != k_no_type; }

    // --------------------- Runtime type ID functions ------------------------

    /** Replaces the current stored object with a new one, like reset. The
     *  key difference is that this function allows a runtime derived type ID
     *  instead of having to know the exact type upfront.
     *  @tparam T a base class of the type which is identified by type_id
     *  @param  type_id compile time constant type ID as defined by this class
     *  @return Returns a pair of pointers, one that points to the base class,
     *          and the other to the leaf class (for given type T).
     */
    template <typename T>
    UpcastPair<T> set_by_type_id_and_upcast(int type_id);

    /** Retrieves the stored object regardless of current type and upcasts it
     *  to the given leaf class type.
     *  @tparam T a base class of the type which is identified by type_id
     *  @param  type_id compile time constant type ID as defined by this class
     *  @return Returns a pair of pointers, one that points to the base class,
     *          and the other to the leaf class (for given type T). Returns a
     *          pair of nullptrs if the stored object does not match the
     *          provided type_id.
     */
    template <typename T>
    UpcastPair<T> get_by_type_id_and_upcast(int type_id);

    //! @copydoc get_by_type_id_and_upcast(int)
    template <typename T>
    ConstUpcastPair<T> get_by_type_id_and_upcast(int type_id) const;

#   if 0
    template <typename ... FuncTypes>
    void visit(const std::tuple<FuncTypes...> &);
#   endif

    // -------------- static_cast and dynamic_cast replacements ---------------

    /** Allows you to perform a static_cast on the object without knowing the
     *  stored object's type ahead of time.
     */
    template <typename T>
    T * static_cast_();

    /** Allows you to perform a static_cast on the object without knowing the
     *  stored object's type ahead of time.
     */
    template <typename T>
    const T * static_cast_() const;

    template <typename T>
    T * dynamic_cast_();

    template <typename T>
    const T * dynamic_cast_() const;

private:
    template <typename T>
    const T * as_back() const noexcept;

    static void copy(int id, void * dest, const void * src);

    using StorageType = typename StorageUnion<Types...>::Type;
    StorageType m_store;
    int m_current_type;
};

template <typename T>
struct MultiTypeUpcastPair {
    // object as leaf type
    void * object_pointer;
    // obect as a base type
    T * upcasted_pointer;
};

template <typename T>
struct MultiTypeConstUpcastPair {
    // object as leaf type
    const void * object_pointer;
    // obect as a base type
    const T * upcasted_pointer;
};

// <------------------------- MultiType Private Helpers ---------------------->

class MultiTypePriv {
    template <typename ... Types>
    friend class MultiType;

    template <typename T>
    using UpcastPair = MultiTypeUpcastPair<T>;

    template <typename T>
    using ConstUpcastPair = MultiTypeConstUpcastPair<T>;

    using Error = std::runtime_error;

#   define MACRO_MULTITYPEPRIV_COUNT_AND_TYPE_ID_ARE_EQUAL_EXPRESSION(count, id) \
        ((count - 1) == (id))
    template <typename TypeList_, int INDEX>
    struct TypeIsHeadType {
        static constexpr const bool k_value =
            MACRO_MULTITYPEPRIV_COUNT_AND_TYPE_ID_ARE_EQUAL_EXPRESSION
            (int(TypeList_::k_count), INDEX);
    };

    template <typename TypeList_>
    static constexpr bool type_is_head_type(int id) {
        return MACRO_MULTITYPEPRIV_COUNT_AND_TYPE_ID_ARE_EQUAL_EXPRESSION
               (TypeList_::k_count, id);
    }
#   undef MACRO_MULTITYPEPRIV_COUNT_AND_TYPE_ID_ARE_EQUAL_EXPRESSION

    // ----------------------------- destruction ------------------------------

    [[noreturn]] static void destruct(TypeList<>, int, void *)
        { throw Error("Can't destruct, unknown type!"); }

    template <typename ... Types>
    static void destruct(TypeList<Types...>, int id, void * ptr) {
        using HeadType = typename TypeList<Types...>::HeadType;
        if (type_is_head_type<TypeList<Types...>>(id)) {
            HeadType * t = reinterpret_cast<HeadType *>(ptr);
            t->~HeadType();
        } else {
            destruct(typename TypeList<Types...>::InheritedType(), id, ptr);
        }
    }

    // ------------------------------- getting --------------------------------

    template <typename Base, typename Head>
    static typename std::enable_if<std::is_base_of<Base, Head>::value, const Base *>::
    type handle_upcast(const Head * ht) { return static_cast<const Base *>(ht); }

    template <typename Base, typename Head>
    static typename std::enable_if<!std::is_base_of<Base, Head>::value, const Base *>::
    type handle_upcast(const Head *) { return nullptr; }

    template <typename T>
    [[noreturn]] static ConstUpcastPair<T> get_by_id_then_upcast
        (TypeList<>, int, const void *)
    { throw Error("Can't get, type id number not on list!"); }

    template <typename T, typename ... Types>
    static ConstUpcastPair<T> get_by_id_then_upcast
        (TypeList<Types...>, int id, const void * src)
    {
        using HeadType = typename TypeList<Types...>::HeadType;
        if (type_is_head_type<TypeList<Types...>>(id)) {
            auto object_ptr = reinterpret_cast<const HeadType *>(src);
            ConstUpcastPair<T> rv;
            rv.object_pointer = object_ptr;
            // C++ may change the address in the up-cast
            rv.upcasted_pointer = handle_upcast<T>(object_ptr);
            return rv;
        } else {
            return get_by_id_then_upcast<T>
                (typename TypeList<Types...>::InheritedType(), id, src);
        }
    }

    // --------------------------------- copy ---------------------------------

    [[noreturn]] static void copy(TypeList<>, int, void *, const void *) {
        throw Error("Can't copy, unknown type!");
    }

    template <typename ... Types>
    static void copy(TypeList<Types...>, int id, void * dest, const void * src)
    {
        using HeadType = typename TypeList<Types...>::HeadType;
        if (type_is_head_type<TypeList<Types...>>(id)) {
            const HeadType * t = reinterpret_cast<const HeadType *>(src);
            new (dest) HeadType(*t);
        } else {
            copy(typename TypeList<Types...>::InheritedType(), id, dest, src);
        }
    }

    // ----------------------------- construction -----------------------------

    template <typename Base, typename Head>
    static typename std::enable_if<std::is_base_of<Base, Head>::value, Base *>::
    type handle_upcast(Head * ht) { return static_cast<Base *>(ht); }

    template <typename Base, typename Head>
    static typename std::enable_if<!std::is_base_of<Base, Head>::value, Base *>::
    type handle_upcast(Head *) { return nullptr; }

    template <typename T>
    [[noreturn]] static UpcastPair<T> construct_by_id_then_upcast
        (TypeList<>, int, void *)
    { throw Error("Can't construct, type id number not on list!"); }

    template <typename T, typename ... Types>
    static UpcastPair<T> construct_by_id_then_upcast
        (TypeList<Types...>, int id, void * dest)
    {
        using HeadType = typename TypeList<Types...>::HeadType;
        if (type_is_head_type<TypeList<Types...>>(id)) {
            HeadType * ht = new (dest) HeadType();
            UpcastPair<T> rv;
            rv.object_pointer = ht;
            // C++ may change the address in the up-cast
            rv.upcasted_pointer = handle_upcast<T>(ht);
            return rv;
        } else {
            return construct_by_id_then_upcast<T>
                (typename TypeList<Types...>::InheritedType(), id, dest);
        }
    }

    // ------------------------------------------------------------------------

    enum CastType { k_do_dynamic_cast, k_do_static_cast };

    template <CastType CAST_T, typename T>
    static const T * special_cast
        (TypeList<>, int, const void *)
    { return nullptr; }

    // should I use SFINAE?
    template <CastType CAST_T, typename T, typename ... Types>
    static const T * special_cast
        (TypeList<Types...>, int id, const void * src)
    {
        static_assert(CAST_T == k_do_static_cast || CAST_T == k_do_dynamic_cast,
                      "Can only cast using dynamic_cast or static_cast.\n"
                      "This maybe a result of a bad enum value.");
        using HeadType = typename TypeList<Types...>::HeadType;;
        if (type_is_head_type<TypeList<Types...>>(id)) {
            const HeadType * ht = reinterpret_cast<const HeadType *>(src);
            if (CAST_T == k_do_static_cast)
                return static_cast<const T *>(ht);
            else
                return dynamic_cast<const T *>(ht);
        } else {
            return special_cast<CAST_T, T>
                (typename TypeList<Types...>::InheritedType(), id, src);
        }
    }

    struct TestA {};
    struct TestB {};
    struct TestC {};

    using TestList = TypeList<TestA, TestB, TestC>;
    static_assert(TestList::GetTypeId<TestA>::k_value == TestList::k_count - 1, "");
    static_assert(TypeIsHeadType<
        TestList,
        TestList::GetTypeId<TestA>::k_value>::k_value, "");
    static_assert(TypeIsHeadType<
        TestList::InheritedType,
        TestList::GetTypeId<TestB>::k_value>::k_value, "");
    static_assert(TypeIsHeadType<
        TestList::InheritedType::InheritedType,
        TestList::GetTypeId<TestC>::k_value>::k_value, "");
    static_assert(TypeIsHeadType<
        TestList::InheritedType::InheritedType::InheritedType,
        TestList::GetTypeId<int>::k_value>::k_value, "");
}; // end of MultiTypePriv helper class

// <-------------------------- MultiType IMPLEMENTATION ---------------------->

template <typename ... Types>
MultiType<Types...>::MultiType(const MultiType & rhs):
    m_current_type(rhs.m_current_type)
{
    if (rhs.m_current_type != k_no_type)
        copy(m_current_type, &m_store, &rhs.m_store);
}

template <typename ... Types>
MultiType<Types...> & MultiType<Types...>::operator = (const MultiType & rhs) {
    if (this == &rhs) return *this;
    unset();
    if (rhs.m_current_type != k_no_type)
        copy(rhs.m_current_type, &m_store, &rhs.m_store);
    m_current_type = rhs.m_current_type;
    return *this;
}

template <typename ... Types>
MultiType<Types...>::~MultiType() { unset(); }

template <typename ... Types>
template <typename T>
T * MultiType<Types...>::as_pointer() noexcept {
    const MultiType<Types ...> * const_this = this;
    return const_cast<T *>(const_this->as_pointer<T>());
}

template <typename ... Types>
template <typename T>
const T * MultiType<Types...>::as_pointer() const noexcept
    { return as_back<T>(); }

template <typename ... Types>
template <typename T>
T & MultiType<Types...>::as() {
    const MultiType<Types ...> * const_this = this;
    return *const_cast<T *>(&const_this->as<T>());
}

template <typename ... Types>
template <typename T>
const T & MultiType<Types...>::as() const {
    const T * rv = as_pointer<T>();
    if (rv) {
        return *rv;
    } else {
        throw Error("Cannot cast to type T, current type does not match.");
    }
}

template <typename ... Types>
template <typename T, typename ... ArgTypes>
T & MultiType<Types...>::reset(ArgTypes &&... args) {
    static_assert(MtTypeList::template HasType<T>::k_value,
                  "MultiType can only reset to a type that is present on its "
                  "TypeList.");
    static_assert(std::is_constructible<T, ArgTypes...>::value,
                  "Object cannot be constructed with these provided arguments "
                  "or needs arguments for construction.");
    unset();
    m_current_type = MtTypeList::template GetTypeId<T>::k_value;
    return *(new (&m_store) T(std::forward<ArgTypes>(args)...));
}

template <typename ... Types>
void MultiType<Types...>::unset() {
    if (m_current_type != k_no_type) {
        MultiTypePriv::destruct(MtTypeList(), m_current_type, &m_store);
        m_current_type = k_no_type;
    }
}

template <typename ... Types>
template <typename T>
bool MultiType<Types...>::is_type() const noexcept
    { return m_current_type == MtTypeList::template GetTypeId<T>::k_value; }

template <typename ... Types>
template <typename T>
typename MultiType<Types...>::template UpcastPair<T> MultiType<Types...>::
    set_by_type_id_and_upcast(int type_id)
{
    if (type_id < 0)
        throw Error("Invalid type id provided, cannot change type.");
    m_current_type = type_id;
    return MultiTypePriv::construct_by_id_then_upcast<T>
        (TypeList<Types...>(), type_id, &m_store);
}

template <typename ... Types>
template <typename T>
typename MultiType<Types...>::template UpcastPair<T> MultiType<Types...>::
    get_by_type_id_and_upcast(int type_id)
{
    auto gv = static_cast<const MultiType<Types...> *>(this)->
        get_by_type_id_and_upcast<T>(type_id);
    UpcastPair<T> rv;
    rv.upcasted_pointer = const_cast<T    *>(gv.upcasted_pointer);
    rv.object_pointer   = const_cast<void *>(gv.object_pointer  );
    return rv;
}
#if 0
template <typename ... TupleTypes>
class DefineSomething {
public:
    using Fp = void(*)(const std::tuple<TupleTypes...>, void *);

    template <typename ... Types>
    static Fp make_entry() {
        return nullptr;
    }

    template <typename Head, typename ... Types>
    static Fp make_entry() {
        return nullptr;
    }
};

template <typename ... Types>
template <typename ... FuncTypes>
void MultiType<Types...>::visit(const std::tuple<FuncTypes...> & tuple_) {
    using Fp = void(*)(const std::tuple<FuncTypes...>, void *);
    std::array<Fp, k_type_count> indirect_call_array;
}
#endif
template <typename ... Types>
template <typename T>
typename MultiType<Types...>::template ConstUpcastPair<T> MultiType<Types...>::
    get_by_type_id_and_upcast(int type_id) const
{
    if (type_id != m_current_type) {
        throw Error("Invalid type id provided, does not match current type, "
                    "use the type_id() method to test before calling this "
                    "function.");
    }
    return MultiTypePriv::get_by_id_then_upcast<T>
        (TypeList<Types...>(), type_id, &m_store);
}

template <typename ... Types>
template <typename T>
T * MultiType<Types...>::static_cast_() {
    const MultiType<Types...> * const_this = this;
    // non-trivial implementation
    return const_cast<T *>(const_this->static_cast_<T>());
}

template <typename ... Types>
template <typename T>
const T * MultiType<Types...>::static_cast_() const {
    return MultiTypePriv::special_cast
           <MultiTypePriv::k_do_static_cast, T>
           (MtTypeList(), m_current_type, &m_store);
}

template <typename ... Types>
template <typename T>
T * MultiType<Types...>::dynamic_cast_() {
    const MultiType<Types...> * const_this = this;
    // non-trivial implementation
    return const_cast<T *>(const_this->dynamic_cast_<T>());
}

template <typename ... Types>
template <typename T>
const T * MultiType<Types...>::dynamic_cast_() const {
    return MultiTypePriv::special_cast
           <MultiTypePriv::k_do_dynamic_cast, T>
           (MtTypeList(), m_current_type, &m_store);
}

template <typename ... Types>
template <typename T>
/* private */ const T * MultiType<Types...>::as_back() const noexcept {
    if (m_current_type == MtTypeList::template GetTypeId<T>::k_value)
        return reinterpret_cast<const T *>(&m_store);
    else
        return nullptr;
}

template <typename ... Types>
/* private static */ void MultiType<Types...>::copy
    (int id, void * dest, const void * src)
{ MultiTypePriv::copy(MtTypeList(), id, dest, src); }
