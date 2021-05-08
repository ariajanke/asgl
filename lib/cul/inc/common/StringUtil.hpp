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

#include <string>
#include <utility>
#include <functional>
#include <limits>

#include <common/ConstString.hpp>
#include <common/Util.hpp>

// <------------------------- Implementation Detail -------------------------->
// <------------------- Public Interface is further below -------------------->

template <typename IterType, typename RealType>
using EnableStrToNumIter = typename std::enable_if<
    std::is_base_of<std::forward_iterator_tag,
                    typename std::iterator_traits<IterType>::iterator_category>::value
    && std::is_arithmetic<RealType>::value,
    bool
>;

template <typename IterType, typename RealType>
using EnableStrToNumPtr = typename std::enable_if<
    std::is_pointer<IterType>::value &&
    std::is_arithmetic<RealType>::value,
    bool
>;

template <typename IterType, typename RealType>
using EnableStrToNum = typename std::enable_if<
    (std::is_pointer<IterType>::value ||
     std::is_base_of<std::forward_iterator_tag,
                     typename std::iterator_traits<IterType>::iterator_category>::value)
    && std::is_arithmetic<RealType>::value,
bool
>;

// <---------------------------- String Utilities ---------------------------->

// These utilities should cover functionality that is NOT easily done via the 
// STL

// dupelicate exists in tmap (TileSet)
// however, merging them is complicated, tmap is meant to be seperate
/* linked */ void fix_path
    (const std::string & referee, const std::string & referer,
     std::string & dest_path);

/** @brief Splits a string segments via a seperator classifier function.
 *  @note  Resulting segments will <em>not</em> contain the seperator 
 *         characters.
 *  @note  if your functor take incompatible types as parameters, you could
 *         receive hard to read conversion errors
 *  @tparam is_seperator must be take the form: bool (*)(decltype(*IterType()))
 *                       returning true will be taken to mean that the given
 *                       character is a seperator character
 *  @tparam IterType     Iterator type (hopefully this can be deduced)
 *  @tparam Func         Segment processing functor's type (hopefully this can 
 *                       be deduced)
 *  @param beg begining of the string to split
 *  @param end one past the end of the string to split
 *  @param f Functor that must take the form
 */
template <auto is_seperator, typename IterType, typename Func>
void for_split(IterType beg, IterType end, Func && f);

/** @brief Container version of for_split, behaves exactly the same as:
 *         for_split(std::begin(cont), std::end(cont), std::move(f));
 */
template <auto is_seperator, typename ContainerType, typename Func>
void for_split(const ContainerType & cont, Func && f);

/** @brief Moves iterator pair closer together, skipping all trailing (on both 
 *  sides) characters where is_tchar(*itr) is true.
 *  The iterators are moved the fewest possible spaces until the constraint is 
 *  satisfied.
 *  @note if is_tchar(*itr) where itr is all iterators in [beg end) then
 *        beg is set end at the end of this call.
 *  @param beg modified to point to the begining of a segment where 
 *         is_tchar(*beg) is false
 *  @param end modified to point the end of a segment such that 
 *         is_tchar(*(end - 1)) is false
 */
template <auto is_tchar, typename IterType>
void trim(IterType & beg, IterType & end);

/** @brief Converts a string to a number, assuming that it's negative for 
 *         signed types. (Useful for integers)
 *  @note this will assume the string has been stripped of its sign, as such
 *        if it is present, it'll return false (numeric conversion fails)
 *  @param begin start/beginning of string as an iterator
 *  @param end one past the end of the string as an iterator
 *  @param out if conversion is sucessful, then the output is written to this 
 *             parameter reference, if it's a signed type, it's written as a 
 *             negative number
 *  @param k_base numeric base of conversion, numeric characters that fall 
 *         outside of this base will cause the conversion to fail
 *  @returns true if conversion succeeds, false otherwise
 */
template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number_assume_negative
    (IterType begin, IterType end, RealType & out,
     const int k_base = 10) noexcept;

/** @brief Converts a string to a number, based on builtin prefixes.
 *  @n
 *  Numbers are read, sign first, prefix, and lastly numeric magnitude (which 
 *  may include a dot for digits less than one)
 *  The prefixes are as follows:
 *  - 0x for hexadecimal
 *  - 0o for octal (that's a lowercase O)
 *  - 0b for binary
 *  - no prefix for decimal
 *  @note 089 will be read as the decimal number: 89.
 */
template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number_multibase
    (IterType begin, IterType end, RealType & out) noexcept;

/** @brief Converts a string to a number, based on builtin prefixes.
 *         Version for containers. (must be default constructable type)
 */
template <typename ContType, typename RealType>
typename EnableStrToNum<decltype(std::begin(ContType())), RealType>::type
/* bool */ string_to_number_multibase
    (const ContType & cont, RealType & out) noexcept
{ return string_to_number_multibase(std::begin(cont), std::end(cont), out); }

/** @brief Converts a string to a number, without a prefix. This function will
 *         check for a sign, and is mostly present for backward compatibility's 
 *         sake.
 *  @param k_base numeric base to convert
 */
template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number
    (IterType begin, IterType end, RealType & out, const int k_base = 10) noexcept;

template <typename ContType, typename RealType>
typename EnableStrToNum<decltype(std::begin(ContType())), RealType>::type
/* bool */ string_to_number
    (const ContType & cont, RealType & out, const int k_base = 10) noexcept
{ return string_to_number(std::begin(cont), std::end(cont), out, k_base); }

/** @brief Wraps text as if it were monowidth by a set number of characters.
 *  @note Like all features in this library, this was added because it is used
 *        by multiple projects.
 *  @tparam IterType string iterator type, any random access iterator type
 *  @param beg beginning of character sequence to wrap
 *  @param end end (one past the end) of the character sequence to wrap
 *  @param max_chars The maximum number of characters
 *  @param handle_seq called for each wrapped segment
 *                    must take the form: void(IterType beg, IterType end);
 *                    where beg is the beginning of the sequence, and
 *                    end is the "one past the end" of the sequence
 *  @param is_breaking called to determine if a character is breaking or not
 *                     the wrapping algorithm will endeavor wrap the text such
 *                     that non-breaking sequences are not cut by the wrap
 *                     must take the form bool(decltype(*IterType());
 */
template <typename IterType, typename HandleSequenceFunc, typename IsBreakingFunc>
void wrap_string_as_monowidth
    (IterType beg, IterType end, int max_chars,
     HandleSequenceFunc && handle_seq, IsBreakingFunc && is_breaking);

/** @brief A specialized version of the five parameter overload.
 *  @note In this overload, is_breaking is set with a functor that classifies
 *        whitespace characters as breaking. @n
 *        (specifically: newline, return carriage, tab, and space)
 */
template <typename IterType, typename HandleSequenceFunc>
void wrap_string_as_monowidth
    (IterType beg, IterType end, int max_chars,
     HandleSequenceFunc && handle_seq);

// <---------------------------- implementations ----------------------------->

template <auto is_seperator, typename IterType, typename Func>
void for_split(IterType beg, IterType end, Func && f) {
    using namespace fc_signal;
    auto last = beg;
    bool should_set_last = true;
    for (auto itr = beg; itr != end; ++itr) {
        if (is_seperator(*itr) && !should_set_last) {
            if (adapt_to_flow_control_signal(std::move(f), last, itr) == k_break)
                return;
            should_set_last = true;
        }
        if (should_set_last && !is_seperator(*itr)) {
            last = itr;
            should_set_last = false;
        }
    }
    if (!should_set_last) {
        // last call ignores any possible return value
        (void)f(last, end);
    }
}

template <auto is_seperator, typename ContainerType, typename Func>
void for_split(const ContainerType & cont, Func && f)
    { for_split<is_seperator>(std::begin(cont), std::end(cont), std::move(f)); }

// ----------------------------------------------------------------------------

template <auto is_tchar, typename IterType>
void trim(IterType & beg, IterType & end) {
    while (beg != end) {
        if (!is_tchar(*beg)) break;
        ++beg;
    }
    while (beg != end) {
        if (!is_tchar(*(end - 1))) break;
        --end;
    }
}

// ----------------------------------------------------------------------------

template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number_assume_negative
    (IterType begin, IterType end, RealType & out,
     const int k_base) noexcept
{
    if (k_base < 2 || k_base > 16) {
        return false;
#       if 0
        throw std::runtime_error("bool string_to_number(...): "
                                 "This function supports only bases 2 to 16.");
#       endif
    }

    using CharType = typename std::remove_reference<decltype(*begin)>::type;
    static constexpr bool k_is_signed = std::is_signed<RealType>::value;
    static constexpr bool k_is_integer = !std::is_floating_point<RealType>::value;
    static constexpr RealType k_sign_fix = k_is_signed ? -1 : 1;

    auto working = RealType(0);
    auto multi   = RealType(1);
    // the adder is a one digit number that corresponds to a character
    auto adder     = RealType(0);
    bool found_dot = false;

    // main digit reading loop, iterates characters in the selection in reverse
    do {
        switch (*--end) {
        case CharType('.'):
            if (found_dot) return false;
            found_dot = true;
            if (k_is_integer) {
                if (adder <= k_sign_fix*k_base / RealType(2))
                    working = k_sign_fix*RealType(1);
                else
                    working = RealType(0);
            } else {
                working /= multi;
            }
            adder = RealType(0);
            multi = RealType(1);
            continue;
        case CharType('0'): case CharType('1'): case CharType('2'):
        case CharType('3'): case CharType('4'): case CharType('5'):
        case CharType('6'): case CharType('7'): case CharType('8'):
        case CharType('9'):
            adder = k_sign_fix*RealType(*end - CharType('0'));
            break;
        case CharType('a'): case CharType('b'): case CharType('c'):
        case CharType('d'): case CharType('e'): case CharType('f'):
            adder = k_sign_fix*RealType(*end - 'a' + 10);
            break;
        case CharType('A'): case CharType('B'): case CharType('C'):
        case CharType('D'): case CharType('E'): case CharType('F'):
            adder = k_sign_fix*RealType(*end - 'A' + 10);
            break;
        default: return false;
        }
        // detect overflow
        RealType temp = working + adder*multi;
        if ( k_is_signed && temp > working) return false;
        if (!k_is_signed && temp < working) return false;
        multi *= RealType(k_base);
        working = temp;
    }
    while (end != begin);
    
    // write to parameter
    out = working;
    return true;

}

template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number_multibase
    (IterType begin, IterType end, RealType & out) noexcept
{
    // many DRY violations
    using CharType = typename std::remove_reference<decltype(*begin)>::type;
    bool is_negative = (*begin) == CharType('-');
    static constexpr bool k_is_signed  = std::is_signed<RealType>::value;
    static constexpr bool k_is_integer = !std::is_floating_point<RealType>::value;
    
    if (is_negative) {
        if (!k_is_signed) return false;
        ++begin;
    }
    
    int base = 10;
    if (end - begin > 2 && *begin == CharType('0')) {
        switch(char(*(begin + 1))) {
        case 'x': case 'X': base = 16; break;
        case 'o': case 'O': base =  8; break;
        case 'b': case 'B': base =  2; break;
        default: break;
        }
        if (base != 10) {
            std::advance(begin, 2);
        }
    }
    
    RealType temp;
    if (!string_to_number_assume_negative(begin, end, temp, base)) {
        return false;
    }
    if (!is_negative && k_is_signed) {
        if (k_is_integer && temp == std::numeric_limits<RealType>::min()) {
            return false;
        }
        temp *= RealType(-1);
    }
    out = temp;
    return true;
}

template <typename IterType, typename RealType>
typename EnableStrToNum<IterType, RealType>::type
/* bool */ string_to_number
    (IterType begin, IterType end, RealType & out, const int k_base) noexcept
{
    using CharType = typename std::remove_reference<decltype(*begin)>::type;
    bool is_negative = (*begin) == CharType('-');
    static constexpr bool k_is_signed  = std::is_signed<RealType>::value;
    static constexpr bool k_is_integer = !std::is_floating_point<RealType>::value;
    
    if (is_negative) {
        if (!k_is_signed) return false;
        ++begin;
    }
    
    RealType temp;
    if (!string_to_number_assume_negative(begin, end, temp, k_base)) {
        return false;
    }
    if (!is_negative && k_is_signed) {
        // is signed non-negative integer whose temp value is the min int...
        // this will result in an overflow!
        if (k_is_integer && temp == std::numeric_limits<RealType>::min()) {
            return false;
        }
        temp *= RealType(-1);
    }
    out = temp;
    return true;
}

// ----------------------------------------------------------------------------

class WrapStringAsMonowidthPriv {
public:
    template <typename IterType, typename HandleSequenceFunc, typename IsBreakingFunc>
    static void wrap_string_as_monowidth
        (IterType beg, IterType end, int max_chars,
         HandleSequenceFunc && handle_seq, IsBreakingFunc && is_breaking)
    {
        auto jtr_last = beg;
        auto jtr      = constrain_offset(beg, end, max_chars);
        while (true) {
            if (jtr == end) {
                handle_seq(jtr_last, jtr);
                break;
            }
            auto gv = find_last_from(is_breaking, jtr_last, jtr, end);

            if (gv != end) ++gv;
            if (gv == end) gv = jtr;
            {
            auto sent_beg = jtr_last;
            auto sent_end = gv;
            // need to test for "bad" argument type (by reference)
            if (adapt_to_flow_control_signal
                    (std::move(handle_seq), sent_beg, sent_end) == fc_signal::k_break)
            { return; }
#           if 0
            handle_seq(sent_beg, sent_end);
#           endif
            }
            jtr_last = gv;
            jtr      = constrain_offset(gv, end, max_chars);
        }
    }
private:
    template <typename IterType>
    static IterType constrain_offset(IterType itr, IterType end, int offset)
        { return (offset > end - itr) ? end : itr + offset; }

    template <typename IterType, typename IsBreakingFunc>
    static IterType find_last_from
        (const IsBreakingFunc & is_breaking,
         IterType beg, IterType itr, IterType end)
    {
        while (!is_breaking(*itr)) {
           if (itr == beg) return end;
           --itr;
       }
       return itr;
    }
};

template <typename IterType, typename HandleSequenceFunc, typename IsBreakingFunc>
void wrap_string_as_monowidth
    (IterType beg, IterType end, int max_chars,
     HandleSequenceFunc && handle_seq, IsBreakingFunc && is_breaking)
{
    WrapStringAsMonowidthPriv::wrap_string_as_monowidth
        (beg, end, max_chars, std::move(handle_seq), std::move(is_breaking));
}

template <typename IterType, typename HandleSequenceFunc>
void wrap_string_as_monowidth
    (IterType beg, IterType end, int max_chars,
     HandleSequenceFunc && handle_seq)
{
    using CharType = decltype(*beg);
    wrap_string_as_monowidth(
        beg, end, max_chars, std::move(handle_seq),
        [](CharType c) { return c == CharType(' ' ) || c == CharType('\n') ||
                                c == CharType('\t') || c == CharType('\r'); });
}
