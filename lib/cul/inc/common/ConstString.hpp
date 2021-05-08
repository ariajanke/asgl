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

/** This is a c-string class wrapper. It maybe treated much like std::string.
 *  This class is essentially syntax sugar. I find that this "sugar" is better
 *  This class is out-moded by string_view which is too recent for my projects.
 *  :c
 *  at making code easier to read, so in my mind at least it is not "sugar".
 *  @note "sugar" carries a negative conntation, and IMO does not belong in
 *        discussions about readability.
 */
class ConstString {
public:
    ConstString(): m_str("") {}
    ConstString(const char * const str_): m_str(str_) {}

#   define MACRO_OP_DEFINE(tok) \
    bool operator tok (const ConstString & lhs) const \
        { return operator tok (lhs.m_str); }

    MACRO_OP_DEFINE(==) MACRO_OP_DEFINE(!=)
    MACRO_OP_DEFINE(<=) MACRO_OP_DEFINE(>=)
    MACRO_OP_DEFINE(< ) MACRO_OP_DEFINE( >)

#   undef MACRO_OP_DEFINE

#   define MACRO_OP_DEFINE(tok) \
    bool operator tok (const std::string & lhs) const \
        { return operator tok (lhs.c_str()); }

    MACRO_OP_DEFINE(==) MACRO_OP_DEFINE(!=)
    MACRO_OP_DEFINE(<=) MACRO_OP_DEFINE(>=)
    MACRO_OP_DEFINE(< ) MACRO_OP_DEFINE( >)

#   undef MACRO_OP_DEFINE

    bool operator == (const char * lhs) const
        { return compare_result(ConstString(lhs)) == 0; }

    bool operator != (const char * lhs) const
        { return compare_result(ConstString(lhs)) != 0; }

    bool operator <= (const char * lhs) const
        { return compare_result(ConstString(lhs)) <= 0; }

    bool operator >= (const char * lhs) const
        { return compare_result(ConstString(lhs)) >= 0; }

    bool operator <  (const char * lhs) const
        { return compare_result(ConstString(lhs)) < 0; }

    bool operator >  (const char * lhs) const
        { return compare_result(ConstString(lhs)) > 0; }

    const char * as_cstring() const noexcept { return m_str; }

    // "this - rhs"
    int compare_result(const ConstString & rhs) const noexcept;

    unsigned length() const noexcept;
private:
    const char * m_str;
};

inline bool operator == (const char * rhs, const ConstString & lhs)
    { return lhs == rhs; }

inline bool operator != (const char * rhs, const ConstString & lhs)
    { return lhs != rhs; }

inline bool operator <= (const char * rhs, const ConstString & lhs)
    { return lhs > rhs; }

inline bool operator >= (const char * rhs, const ConstString & lhs)
    { return lhs < rhs; }

inline bool operator < (const char * rhs, const ConstString & lhs)
    { return lhs >= rhs; }

inline bool operator > (const char * rhs, const ConstString & lhs)
    { return lhs <= rhs; }
