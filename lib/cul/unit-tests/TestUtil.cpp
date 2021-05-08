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

#include <common/Util.hpp>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <cassert>

// tests performed on the following functions:
// - quad_range
// - normalize
// - and_rectangles
// - rotate_vector
// - project_unto
// - float_equals -> for semantics testing
// - is_nan
//
// other functions are not tested, this is due to the implementation being 
// trivial
// - mag
// - sign
// - pi
// - area_of
// - dot
// - angle_between
// - major
// - rectangle_location
// - right_of
// - bottom_of
// - center_of

namespace {

using Error = std::runtime_error;

void test_quad_range();
void test_normalize();
void test_and_rectangles();
void test_rotate_vector();

}

int main() {
    try {
        // quad_range
        test_quad_range();
        // normalize
        test_normalize();
        // and_rectangles
        test_and_rectangles();
        // rotate_vector
        test_rotate_vector();
        // project_unto
        // float_equals -> for semantics testing
        // is_nan
    } catch (std::exception & exp) {
        std::cout << exp.what() << std::endl;
        return ~0;
    }
    return 0;
}

namespace {

class Obj {
public:
    Obj(char c): m_value(c) {}
    static Obj copy_without_marked(const Obj & obj) {
        Obj temp(obj.value());
        return temp;
    }
    void mark_off(const Obj & other) {
        assert(other.m_value != m_value);
        m_marked.push_back(copy_without_marked(other));
    }
    char value() const { return m_value; }
    bool compare(const std::vector<Obj> & col) const {
        auto marked_copy = m_marked;
        marked_copy.push_back(copy_without_marked(*this));
        std::sort(marked_copy.begin(), marked_copy.end());
        return std::equal(col.begin(), col.end(),
                          marked_copy.begin(), marked_copy.end());
    }
    bool operator <  (const Obj & rhs) const { return m_value <  rhs.m_value; }
    bool operator == (const Obj & rhs) const { return m_value == rhs.m_value; }
private:
    char m_value;
    std::vector<Obj> m_marked;
};

void test_quad_range() {
    quad_range<int>({ 4 }, [](int, int) 
        { throw Error("test_quad_range: " + std::to_string(__LINE__)); }
    );
    
    auto do_obj_compare = [](std::initializer_list<char> && charas) {
        std::vector<Obj> objs;
        for (auto c : charas) { objs.emplace_back(c); }
        quad_range<Obj>(objs, [&](Obj & a, Obj & b) {
            a.mark_off(b);
            b.mark_off(a);
        });
        for (const auto & obj : objs) {
            if (!obj.compare(objs)) {
                throw Error("test_quad_range: size value " + 
                            std::to_string(objs.size())      );
            }
        }
    };
    
    do_obj_compare({ 'a', 'b' });
    do_obj_compare({ 'a', 'b', 'c' });
    do_obj_compare({ 'a', 'b', 'c', 'd' });
    do_obj_compare({ 'a', 'b', 'c', 'd', 'e' });
}

void test_normalize() {
    bool flag = false;
    try {
        normalize(0);
    } catch (std::exception &) {
        flag = true;
    }
    if (!flag) {
        throw Error("test_normalize: " + std::to_string(__LINE__));
    }
    flag = false;
    try {
        normalize(sf::Vector2<double>(0.0, 0.0));
    } catch (std::exception &) {
        flag = true;
    }
    if (!flag) {
        throw Error("test_normalize: " + std::to_string(__LINE__));
    }
}

void test_and_rectangles() {
}

void test_rotate_vector() {
}

} // end of <anonymous> namespace
