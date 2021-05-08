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

template <std::size_t LHS, std::size_t RHS>
struct StaticMax {
    static const std::size_t k_value = LHS > RHS ? LHS : RHS;
};

// zero arguements
template <typename ... Types>
struct StorageUnion {
    static const std::size_t k_alignment = std::alignment_of<char>::value;

    /** by C++ standards, any class is at least one byte (from Stroustrup's C++
     *  book 3rd edition, no I forgot where...) */
    static const std::size_t k_size = 1;

    /** A type of space, where any type in Types can live. */
    using Type = typename std::aligned_storage<k_size, k_alignment>::type;
};

// one arguement
template <typename InType, typename ... Types>
struct StorageUnion<InType, Types...> : public StorageUnion<Types...> {
    static const std::size_t k_alignment = StaticMax
        <std::alignment_of<InType>::value, StorageUnion<Types...>::k_alignment>
        ::k_value;

    static const std::size_t k_size = StaticMax
        <sizeof(InType), StorageUnion<Types...>::k_size>::k_value;

    /** A type of space, where any type in Types can live. */
    using Type = typename std::aligned_storage<k_size, k_alignment>::type;
};

// one arguement
template <typename BaseType, typename ... Types>
struct IsBaseOfMulti {
    static const bool k_value = true;
};

// two arguement
template <typename BaseType, typename OtherType, typename ... Types>
struct IsBaseOfMulti<BaseType, OtherType, Types...> :
    public IsBaseOfMulti<BaseType, Types...>
{
    static const bool k_value =
           std::is_base_of<BaseType, OtherType>::value
        && IsBaseOfMulti<BaseType, Types...>::k_value;
};
