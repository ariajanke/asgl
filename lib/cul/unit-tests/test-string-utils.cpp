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

#include <common/StringUtil.hpp>
#include <common/TestSuite.hpp>

#include <cstring>

// test the following:
// for_split
// string_to_number
// - (input, double, int)
// trim
// find_first

// these functions should work regardless of iterator/character type

namespace {

bool run_for_split_tests();
bool run_string_to_number_tests();
bool run_trim_tests();
bool run_wrap_tests();

} // end of <anonymous> namespace

int main() {
    auto test_list = {
        run_for_split_tests,
        run_string_to_number_tests,
        run_trim_tests,
        run_wrap_tests
    };
    
    bool all_good = true;
    for (auto f : test_list) {
        if (!f()) all_good = false;
    }
    
    return all_good ? 0 : ~0;
}

namespace {

inline bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
inline bool is_whitespace_u(std::u32string::value_type c) { return is_whitespace(char(c)); }

bool run_for_split_tests() {
    using Iter = std::string::iterator;
    
    using ConstIter = std::string::const_iterator;
    ts::TestSuite suite("for_split");
    suite.test([]() {
        int count = 0;
        std::string samp = "a b c";
        
        for_split<is_whitespace>(samp.begin(), samp.end(), 
            [&count](Iter, Iter)
        { ++count; });
        return ts::test(count == 3);
    });
    suite.test([]() {
        int count = 0;
        std::string samp = "a b c";
        auto beg = &samp[0];
        auto end = beg + samp.length();
        for_split<is_whitespace>(beg, end, 
            [&count](const char * beg, const char * end)
        { count += (end - beg); });
        return ts::test(count == 3);
    });
    suite.test([]() {
        int count = 0;
        std::string samp = "a b c e f";
        
        for_split<is_whitespace>(samp.begin(), samp.end(), 
            [&count](Iter, Iter)
        {
            ++count;
            return (count == 3) ? fc_signal::k_break : fc_signal::k_continue;
        });
        return ts::test(count == 3);
    });
    suite.test([]() {
        int count = 0;
        std::string samp = " a b c  e    f           ";
        for_split<is_whitespace>(samp, [&count](ConstIter, ConstIter)
            { ++count; });
        return ts::test(count == 5);
    });
    return suite.has_successes_only();
}

bool run_string_to_number_tests() {
    ts::TestSuite suite("string_to_number");
    suite.test([]() {
        const char * str = "856";
        int out = 0;
        bool res = string_to_number_assume_negative(str, str + strlen(str), out);
        return ts::test(res && out == -856);
    });
    suite.test([]() {
        const char * str = "123.34";
        float out = 0.f;
        bool res = string_to_number_assume_negative(str, str + strlen(str), out);
        return ts::test(res && magnitude(out + 123.34) < 0.005f);
    });
    suite.test([]() {
        std::string samp = "5786";
        std::size_t out = 0;
        bool res = string_to_number_assume_negative(samp.begin(), samp.end(), out);
        return ts::test(res && out == 5786);
    });
    
    suite.test([]() {
        std::string samp = "0";
        int out = -1;
        bool res = string_to_number(samp.begin(), samp.end(), out);
        return ts::test(res && out == 0);
    });
    suite.test([]() {
        std::string samp = "123";
        int out = 0;
        bool res = string_to_number(samp, out);
        return ts::test(res && out == 123);
    });
    // 5 tests now
    suite.test([]() {
        std::string samp = "-2147483648";
        int32_t out = 0;
        bool res = string_to_number(samp, out);
        return ts::test(res && out == -2147483648);
    });
    suite.test([]() {
        std::string samp = "-101001";
        int out = 0;
        bool res = string_to_number(samp, out, 2);
        return ts::test(res && out == -0b101001);
    });
    suite.test([]() {
        std::u32string wide = U"-9087";
        int out = 0;
        bool res = string_to_number(wide, out);
        return ts::test(res && out == -9087);
    });
    suite.test([]() {
        std::string samp = "0o675";
        int out = 0;
        bool res = string_to_number_multibase(samp.begin(), samp.end(), out);
        return ts::test(res && out == 0675);
    });
    suite.test([]() {
        std::string samp = "089";
        int out = 0;
        bool res = string_to_number_multibase(samp, out);
        return ts::test(res && out == 89);
    });
    // 10 tests now
    suite.test([]() {
        std::string samp = "-0x567.8";
        int out = 0;
        bool res = string_to_number_multibase(samp, out);
        // should round up!
        return ts::test(res && out == -0x568);
    });
    suite.test([]() {
        std::string samp = "0b11011";
        int out = 0;
        bool res = string_to_number_multibase(samp, out);
        return ts::test(res && out == 0b11011);
    });
    suite.test([]() {
        std::string samp = "10.5";
        int out = 0;
        bool res = string_to_number(samp, out);
        return ts::test(res && out == 11);
    });
    suite.test([]() {
        std::string samp = "10.4";
        int out = 0;
        bool res = string_to_number(samp, out);
        return ts::test(res && out == 10);
    });
    suite.test([]() {
        std::string samp = "7995";
        int out = 0;
        bool res = string_to_number_multibase(samp, out);
        return ts::test(res && out == 7995);
    });
    return suite.has_successes_only();
}

bool run_trim_tests() {
    ts::TestSuite suite("trim");
    suite.test([]() {
        std::string samp = " a ";
        auto beg = samp.begin();
        auto end = samp.end  ();
        trim<is_whitespace>(beg, end);
        return ts::test(end - beg == 1 && *beg == 'a');
    });
    suite.test([]() {
        const char * str = " a ";
        auto beg = str;
        auto end = str + strlen(str);
        trim<is_whitespace>(beg, end);
        return ts::test(end - beg == 1 && *beg == 'a');
    });
    suite.test([]() {
        std::u32string str = U" true";
        auto beg = str.begin();
        auto end = str.end  ();
        trim<is_whitespace_u>(beg, end);
        return ts::test(end - beg == 4 && char(*beg) == 't');
    });
    suite.test([]() {
        std::string samp = "a   ";
        auto beg = samp.begin();
        auto end = samp.end();
        trim<is_whitespace>(beg, end);
        return ts::test(end - beg == 1 && *beg == 'a');
    });
    suite.test([]() {
        std::string samp = "               ";
        auto beg = samp.begin();
        auto end = samp.end();
        trim<is_whitespace>(beg, end);
        return ts::test(end == beg);
    });
    return suite.has_successes_only();
}

bool run_wrap_tests() {
    ts::TestSuite suite("wrap_string_as_monowidth");
    static auto do_wrap_tests = [](const char * in, int max_width, std::initializer_list<const char *> list) {
        std::vector<std::string> correct { list.begin(), list.end() };
        for (const auto & str : correct) {
            if (int(str.size()) > max_width) return false;
        }

        std::vector<std::string> res;
        // render_wrapped_lines_to(inv, max_width, 100, res);
        wrap_string_as_monowidth(in, in + strlen(in), max_width,
            [&res](const char * beg, const char * end)
        {
            trim<is_whitespace>(beg, end);
            res.emplace_back(beg, end);
        }, is_whitespace);
        return std::equal(res.begin(), res.end(), correct.begin(), correct.end());
    };
    // tests "stolen" from MemR suite
    suite.test([]() {
        return ts::test(do_wrap_tests("Hello world.", 9, { "Hello", "world." }));
    });
    suite.test([]() {
        return ts::test(
        //             0123456789012345678901234
        do_wrap_tests("This is a short sentence.", 20, { "This is a short", "sentence." })
        );
    });
    suite.test([]() {
        return ts::test(
        //             0123456789012345678901234
        do_wrap_tests("This is a short sentence.", 10, { "This is a", "short", "sentence." })
        );
    });
    suite.test([]() {
        return ts::test(
        do_wrap_tests("-------------------------", 15, { "---------------", "----------" })
        );
    });
    suite.test([]() {
        return ts::test(
        do_wrap_tests("-------------------------", 10, { "----------", "----------", "-----" })
        );
    });
    suite.test([]() {
        return ts::test(
        //             0123456789012345678901234
        do_wrap_tests("0 1 2 3333 4 55 6 777 8", 8, { "0 1 2", "3333 4", "55 6 777", "8" })
        );
    });
    // test is ok with UString
    suite.test([]() {
        //                      0123456789ABCDEFG
        std::u32string samp = U"  -  --   ---  -";
        auto output =       { U"  -  --", U"   ---  ", U"-" };
        using ConstUIter = std::u32string::const_iterator;
        auto itr = output.begin();
        bool rv = true;
        wrap_string_as_monowidth(samp.begin(), samp.end(), 8,
        [&itr, &rv, &output](ConstUIter beg, ConstUIter end) {
            if (itr == output.end()) {
                rv = false;
            } else if (!std::equal(beg, end, *itr++)) {
                rv = false;
            }
            return rv ? fc_signal::k_continue : fc_signal::k_break;
        }, [](u_char c) { return c == u_char('-'); });
        return ts::test(rv);
    });
    return suite.has_successes_only();
}

} // end of <anonymous> namespace
