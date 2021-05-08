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

#include <iosfwd>

namespace ts {

class TestAssertion;
class TestSuite;

TestAssertion test(bool);

using TestFunc = TestAssertion(*)(bool);

// semantic object
class TestAssertion {
    friend TestAssertion test(bool);
    friend class TestSuite;
    bool value;
};

class TestSuite {
public:
    TestSuite();
    explicit TestSuite(const char * series_name);
    
    TestSuite(const TestSuite &) = delete;
    TestSuite(TestSuite &&) = delete;
    
    ~TestSuite();
    
    TestSuite & operator = (const TestSuite &) = delete;
    TestSuite & operator = (TestSuite &&) = delete;
    
    void start_series(const char *);
    void test(TestAssertion (*test_func)());
    void test(TestAssertion (*test_func)(TestFunc));
    void finish_up();
    bool has_successes_only() const;
private:
    template <typename Func>
    void do_test_back(Func &&);
    int m_test_count;
    int m_test_successes;
    std::ostream * m_out;
};

} // end of ts namespace
