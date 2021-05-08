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

#include <common/Grid.hpp>
#include <common/SubGrid.hpp>
#include <common/TestSuite.hpp>

#include <common/TypeList.hpp>

#include <iostream>
#include <algorithm>

#include <cassert>

namespace {

using ts::TestSuite;
using VectorI = sf::Vector2i;

void test_grid();
void test_make_sub_grid();
void test_sub_grid_iterator();

} // end of <anonymous> namespace

int main() {
    test_grid();
    test_make_sub_grid();
    test_sub_grid_iterator();
    return 0;
}

namespace {

void test_grid() {
    // I need "call everything" tests for any template class!

    TestSuite tsuite;
    tsuite.start_series("Grid tests");
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(1, 1);
        g.set_size(2, 3, 10);
        return ts::test(g(1, 1) == 10);
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(2, 3, 10);
        int count = 0;
        for (auto i : g) {
            (void)i;
            ++count;
        }
        return ts::test(int(g.size()) == count);
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(8, 8);
        Grid<int>::ConstReferenceType e = g(3, 4);
        auto gv = g.position_of(e);
        std::cout << g.width() << "x" << g.height() << std::endl;
        std::cout << gv.x << ", " << gv.y << std::endl;
        return ts::test(g.position_of(e) == sf::Vector2i(3, 4));
    });
    tsuite.test([]() {
        Grid<int> g;
        g.set_size(4, 3);
        assert(!g.is_empty());
        auto e = g.begin() + 5;
        *e = 20;
        std::cerr << ((e < g.begin()) ? "t" : "f") << " " << ((e > g.end()) ? "t" : "f") << std::endl;
        auto r = g.position_of(e);
        std::cout << r.x << ", " << r.y << std::endl;
        return ts::test(g(r) == 20);
    });
    // nothing to test outside of, it should compile
    {
    Grid<int> g;
    static constexpr const int k_res_size = 3;
    g.set_size(3, 3, k_res_size);
    g.set_size(4, 5, []() { return 2; }());

    g.set_width(3, k_res_size);
    g.set_width(4, []() { return 2; }());

    g.set_height(3, k_res_size);
    g.set_height(4, []() { return 2; }());

    }

    tsuite.test([]() {
        Grid<int> p;
        auto subg = make_sub_grid(p);
        const auto & cp = p;
        auto csg  = make_sub_grid(cp);

        return ts::test(&p == &subg.parent() && &p == &csg.parent());
    });
    tsuite.test([]() {
        Grid<int> p {{ 1, 1, 1, }, { 1, 1, 1, }, { 1, 1, 1 }};
        int one_count = 0;
        for (int i : p) {
            if (i == 1) ++one_count;
        }
        return ts::test(p.width() == 3 && p.height() == 3 && one_count == 9);
    });
    tsuite.test([]() {
        try {
            Grid<int> { { 1, 1, 1 }, { 1, 1, 1, 1 } };
        } catch (std::invalid_argument &) {
            return ts::test(true);
        } catch (...) {
            return ts::test(false);
        }
        return ts::test(false);
    });
}

void test_make_sub_grid() {
    Grid<int> p;
    p.set_size(3, 3);
    {
    // Grid -> SubGrid
    auto a = make_sub_grid(p);
    auto b = make_sub_grid(p, VectorI(1, 2));
    auto c = make_sub_grid(p, 2);
    auto d = make_sub_grid(p, 2, 2);
    auto e = make_sub_grid(p, VectorI(1, 1), 1);
    auto f = make_sub_grid(p, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<SubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(f)>, "");
    }
    {
    // const Grid & -> ConstSubGrid
    const auto & cref_p = p;
    auto a = make_sub_grid(cref_p);
    auto b = make_sub_grid(cref_p, VectorI(1, 2));
    auto c = make_sub_grid(cref_p, 2);
    auto d = make_sub_grid(cref_p, 2, 2);
    auto e = make_sub_grid(cref_p, VectorI(1, 1), 1);
    auto f = make_sub_grid(cref_p, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    {
    // Grid & -> ConstSubGrid
    auto a = make_const_sub_grid(p);
    auto b = make_const_sub_grid(p, VectorI(1, 2));
    auto c = make_const_sub_grid(p, 2);
    auto d = make_const_sub_grid(p, 2, 2);
    auto e = make_const_sub_grid(p, VectorI(1, 1), 1);
    auto f = make_const_sub_grid(p, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    {
    const auto & cref_p = p;
    auto a = make_const_sub_grid(cref_p);
    auto b = make_const_sub_grid(cref_p, VectorI(1, 2));
    auto c = make_const_sub_grid(cref_p, 2);
    auto d = make_const_sub_grid(cref_p, 2, 2);
    auto e = make_const_sub_grid(cref_p, VectorI(1, 1), 1);
    auto f = make_const_sub_grid(cref_p, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    // ------------------------------ SubGrid ------------------------------
    // make_sub_grid      : SubGrid      -> SubGrid
    //                      ConstSubGrid -> ConstSubGrid
    // make_const_sub_grid: SubGrid      -> ConstSubGrid
    //                      ConstSubGrid -> ConstSubGrid
    {
    auto subg = make_sub_grid(p);
    auto a = make_sub_grid(subg);
    auto b = make_sub_grid(subg, VectorI(1, 2));
    auto c = make_sub_grid(subg, 2);
    auto d = make_sub_grid(subg, 2, 2);
    auto e = make_sub_grid(subg, VectorI(1, 1), 1);
    auto f = make_sub_grid(subg, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<SubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<SubGrid<int>, decltype(f)>, "");
    }
    {
    auto subg = make_const_sub_grid(p);
    auto a = make_sub_grid(subg);
    auto b = make_sub_grid(subg, VectorI(1, 2));
    auto c = make_sub_grid(subg, 2);
    auto d = make_sub_grid(subg, 2, 2);
    auto e = make_sub_grid(subg, VectorI(1, 1), 1);
    auto f = make_sub_grid(subg, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    {
    auto subg = make_sub_grid(p);
    auto a = make_const_sub_grid(subg);
    auto b = make_const_sub_grid(subg, VectorI(1, 2));
    auto c = make_const_sub_grid(subg, 2);
    auto d = make_const_sub_grid(subg, 2, 2);
    auto e = make_const_sub_grid(subg, VectorI(1, 1), 1);
    auto f = make_const_sub_grid(subg, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    {
    auto subg = make_const_sub_grid(p);
    auto a = make_const_sub_grid(subg);
    auto b = make_const_sub_grid(subg, VectorI(1, 2));
    auto c = make_const_sub_grid(subg, 2);
    auto d = make_const_sub_grid(subg, 2, 2);
    auto e = make_const_sub_grid(subg, VectorI(1, 1), 1);
    auto f = make_const_sub_grid(subg, VectorI(1, 1), 1, 1);
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(a)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(b)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(c)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(d)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(e)>, "");
    static_assert(std::is_same_v<ConstSubGrid<int>, decltype(f)>, "");
    }
    // need to test const ref to sub grid
}

void test_sub_grid_iterator() {
    static_assert(
        std::is_same_v<std::iterator_traits<SubGridIteratorImpl<false, int>>::iterator_category,
                       std::bidirectional_iterator_tag>, "");
    static_assert(
        std::is_same_v<std::iterator_traits<SubGrid<int>::Iterator>::iterator_category,
                       std::bidirectional_iterator_tag>, "");
    ts::TestSuite suite;
    suite.start_series("sub grid iterator");
    suite.test([]() {
        Grid<int> p;
        p.set_size(3, 3, 1);
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        return ts::test(!subg.is_empty() && SubGrid<int>().is_empty());
    });
    suite.test([]() {
        Grid<int> p { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        return ts::test(subg.begin() != subg.end());
    });
    suite.test([]() {
        return ts::test(SubGrid<int>().begin() == SubGrid<int>().end());
    });
    // test 03
    suite.test([]() {
        Grid<int> p { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        (*subg.begin()) = 2;
        return ts::test(p(1, 1) == 2);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(3, 3, 1);
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        auto itr = subg.begin();
        ++itr;
        *itr = 2;
        return ts::test(p(2, 1) == 2);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(3, 3, 1);
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        auto itr = subg.begin();
        ++itr;
        --itr;
        itr++;
        itr--;
        *itr = 2;
        return ts::test(p(1, 1) == 2);
    });
    // test 06
    suite.test([]() {
        Grid<int> p;
        p.set_size(3, 3, 1);
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        auto itr = subg.begin();
        ++itr;
        itr++;
        *itr = 2;
        return ts::test(p(1, 2) == 2);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(4, 4, 3);
        auto subg = make_sub_grid(p, VectorI(1, 1), 2, 2);
        auto itr = subg.begin();
        for (int i = 0; i != 2*2; ++i) {
            ++itr;
        }
        return ts::test(itr == subg.end());
    });
    // let's try bigger advancements
    suite.test([]() {
        Grid<int> p;
        p.set_size(6, 6, 2);
        // + + * *
        // * * * *
        // * * x - modified on (2, 2) -> (3, 3)
        auto subg = make_sub_grid(p, VectorI(1, 1), 4, 4);
        auto itr = subg.begin();
        ++itr;
        ++itr;
        itr.move_position(8);
        *itr = 5;
        return ts::test(p(3, 3) == 5);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(6, 6, 2);
        // * * * *
        // * * * *
        // * * x - modified on (2, 2) -> (3, 3)
        auto subg = make_sub_grid(p, VectorI(1, 1), 4, 4);
        auto itr = subg.begin();
        itr.move_position(10);
        *itr = 5;
        return ts::test(p(3, 3) == 5);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(6, 6, 2);
        // * * * *
        // * * * o
        // * * x - then back
        //
        // (3, 1) -> (4, 2)
        auto subg = make_sub_grid(p, VectorI(1, 1), 4, 4);
        auto itr = subg.begin();
        itr.move_position(10);
        --itr; --itr;
        --itr;
        *itr = 5;
        return ts::test(p(4, 2) == 5);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(6, 6, 2);
        // * * o *
        // * * * *
        // * * x - then back
        //
        // (2, 0) -> (3, 1)
        auto subg = make_sub_grid(p, VectorI(1, 1), 4, 4);
        auto itr = subg.begin();
        itr.move_position(10);
        itr.move_position(-8);
        *itr = 5;
        return ts::test(p(3, 1) == 5);
    });
    suite.test([]() {
        Grid<int> p;
        p.set_size(3, 3, 2);
        auto subg = make_sub_grid(p, VectorI(1, 1));
        auto itr = subg.begin();
        auto b   = itr++;
        return ts::test(b == subg.begin() && itr != b);
    });
    suite.test([]() {
        Grid<int> p {
            { 0, 2, 2, 2 },
            { 0, 1, 2, 3 },
            { 0, 2, 2, 3 },
            { 0, 3, 4, 2 },
            { 2, 2, 0, 0 }
        };
        auto subg = make_sub_grid(p, VectorI(1, 1), 3, 3);
        auto count = std::count(subg.begin(), subg.end(), 2);
        return ts::test(count == 4);
    });
}

} // end of <anonymous> namespace
