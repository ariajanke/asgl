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
#include <bitset>
#include <stdexcept>

template <bool TF_VAL, int A, int B>
struct ConstIntSelect { static const constexpr int k_value = A; };

template <int A, int B>
struct ConstIntSelect<false, A, B> { static const constexpr int k_value = B; };

template <typename T1, typename ... Types>
struct UniqueTypes {
    static constexpr const bool k_value = true;
};

template <typename T1, typename T2, typename ... Types>
struct UniqueTypes<T1, T2, Types...> : UniqueTypes<T2, Types...> {
    static constexpr const bool k_value =
        !std::is_same<T1, T2>::value &&
        UniqueTypes<T2, Types...>::k_value && UniqueTypes<T1, Types...>::k_value;
};

template <typename T> struct TypeTag {};

template <typename ... Types>
struct TypeList {
    static constexpr const int k_not_any_type = -1;
    static const std::size_t k_count = sizeof...(Types);

    template <typename Type>
    struct HasType {
        static constexpr const bool k_value = false;
    };

    template <typename Type>
    struct GetTypeId {
        static constexpr const int k_value = k_not_any_type;
    };

    template <template <typename ...> class Other>
    struct DefineWithListTypes {
        using Type = Other<Types...>;
    };

    template <typename ... OtherTypes>
    struct CombineWith {
        using Type = TypeList<Types..., OtherTypes...>;
    };

    using HeadType = TypeTag<void>;
    using InheritedType = TypeTag<void>;
};

template <typename Head, typename ... Types>
struct TypeList<Head, Types...> : TypeList<Types...> {
    static constexpr const int k_not_any_type = TypeList<Types...>::k_not_any_type;
    template <typename Type>
    struct HasType {
        static constexpr const bool k_value =
            (   std::is_same<Head, Type>::value
             || TypeList<Types...>::template HasType<Type>::k_value);
    };

    template <typename Type>
    struct GetTypeId {
        static constexpr const int k_value =
            ConstIntSelect<std::is_same<Head, Type>::value,
            sizeof...(Types),
            TypeList<Types...>::template GetTypeId<Type>::k_value>::
            k_value;
    };

    template <template <typename ...> class Other>
    struct DefineWithListTypes {
        using Type = Other<Head, Types...>;
    };

    template <typename ... OtherTypes>
    struct CombineWith {
        using Type = TypeList<Head, Types..., OtherTypes...>;
    };

    // trying a new linear approach, to save complier time
    static_assert(!TypeList<Types...>::template HasType<Head>::k_value,
                  "All types in type list, must be unique.");

    static const std::size_t k_count = sizeof...(Types) + 1;
    using HeadType = Head;
    using InheritedType = TypeList<Types...>;
};

template <std::size_t SIZE, typename ... ReadTypes, typename ... HostTypes>
void find_bitset_intersection
    (std::bitset<SIZE> &, TypeList<ReadTypes...>, TypeList<HostTypes...>)
{}

template <std::size_t SIZE, typename Head, typename ... ReadTypes,
          typename ... HostTypes>
void find_bitset_intersection
    (std::bitset<SIZE> & bs, TypeList<Head, ReadTypes...>, TypeList<HostTypes...>)
{
    // this is now evaluated at compile time :D
    static_assert(TypeList<HostTypes...>::template HasType<Head>::k_value,
                  "Head type is not in the component type list.");
    bs.set(TypeList<HostTypes...>::template GetTypeId<Head>::k_value, true);
    find_bitset_intersection
        (bs, TypeList<ReadTypes...>(), TypeList<HostTypes...>());
}

template <bool TF_VAL, typename A, typename B>
struct TypeSelect { using Type = A; };

template <typename A, typename B>
struct TypeSelect<false, A, B> { using Type = B; };
