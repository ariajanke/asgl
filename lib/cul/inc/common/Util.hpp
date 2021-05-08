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

#include <iterator>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <iosfwd>
#include <limits>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <common/TypeList.hpp>
#include <common/ConstString.hpp>

// imperfect protection, other includes can still disurpt it
// This problem so far exist on GCC
// Will renaming fix it for all platforms? That is the question which is the
// reason why I just don't rename it and call it done.
#ifdef major
#   undef major
#endif

// <---------------------------- Public Interface ---------------------------->
// <------------------------------- iteration -------------------------------->

template <typename T, typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func);

template <typename T, typename Func, typename Cont>
void quad_range(Cont & container, Func && func);

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f);

namespace fc_signal {

enum FlowControlSignal_e { k_continue, k_break };

}

using FlowControlSignal = fc_signal::FlowControlSignal_e;

template <typename Func, typename ... Types>
typename std::enable_if<
    std::is_same_v<typename std::result_of<Func && (Types && ...)>::type, FlowControlSignal>,
    FlowControlSignal>::
type adapt_to_flow_control_signal(Func && f, Types &&... args)
    { return f(std::forward<Types>(args)...); }

template <typename Func, typename ... Types>
typename std::enable_if<
    std::is_same_v<typename std::result_of<Func && (Types && ...)>::type, void>,
    FlowControlSignal>::
type adapt_to_flow_control_signal(Func && f, Types &&... args) {
    f(std::forward<Types>(args)...);
    return fc_signal::k_continue;
}

// <------------------------------ vector math ------------------------------->

float  square_root(float );
double square_root(double);
int    square_root(int   );

float  sine(float );
double sine(double);

float  cosine(float );
double cosine(double);

float  arc_cosine(float );
double arc_cosine(double);

template <typename T>
T magnitude(const sf::Vector2<T> & v)
    { return square_root(v.x*v.x + v.y*v.y); }

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    magnitude(T t)
    { return (t < T(0)) ? -t : t; }

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> & v);

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    normalize(T t);

template <typename T>
    constexpr typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    get_pi() { return T(3.141592653589793238462643383279); }

template <typename T>
sf::Vector2<T> rectangle_location(const sf::Rect<T> & rect)
    { return sf::Vector2<T>(rect.left, rect.top); }

template <typename T>
T right_of(const sf::Rect<T> & rect) { return rect.left + rect.width; }

template <typename T>
T bottom_of(const sf::Rect<T> & rect) { return rect.top + rect.height; }

template <typename T>
sf::Vector2<T> center_of(const sf::Rect<T> &);

template <typename T>
sf::Rect<T> compute_rectangle_intersection
    (const sf::Rect<T> &, const sf::Rect<T> &);

template <typename T>
T area_of(const sf::Rect<T> & a) { return a.width*a.height; }

// required to doc, the unit circle (frame of reference) for this function...
template <typename T>
sf::Vector2<T> rotate_vector(sf::Vector2<T> r, T rot);

template <typename T>
T dot(const sf::Vector2<T> & v, const sf::Vector2<T> & u)
    { return v.x*u.x + v.y*u.y; }

template <typename T>
T angle_between(const sf::Vector2<T> & v, const sf::Vector2<T> & u);

// reads: project a onto b
template <typename T>
sf::Vector2<T> project_onto(const sf::Vector2<T> & a, const sf::Vector2<T> & b);

template <typename T>
sf::Vector2<T> major(const sf::Vector2<T> & v);

template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    is_nan(T a)
{ return a != a; }


template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, bool> is_real(T x) {
    using Lims = std::numeric_limits<T>;
    return x == x && Lims::infinity() != x && -Lims::infinity() != x;
}

// first  low  bounds where f(first ) = false
// second high bounds where f(second) = true
template <typename Func>
std::pair<double, double> find_smallest_diff(Func && f, double hint = 0.5, double error = 0.0005);

template <typename Func>
double find_highest_false(Func && f, double hint = 0.5, double error = 0.0005) {
    return find_smallest_diff(std::move(f), hint, error).first;
}

template <typename Func>
double find_lowest_true(Func && f, double hint = 0.5, double error = 0.0005) {
    return find_smallest_diff(std::move(f), hint, error).second;
}

// <-------------------------------- debugging ------------------------------->

void message_assert(const char * msg, bool cond);

// ----------------------------------------------------------------------------
// ----------------------- Implementation Details -----------------------------

template <typename T, typename Func, typename Iter>
void quad_range(Iter beg, Iter end, Func && func) {
    for (Iter itr = beg; itr != end; ++itr) {
    for (Iter jtr = beg; jtr != itr; ++jtr) {
        auto & i_obj = *itr;
        auto & j_obj = *jtr;
        if (adapt_to_flow_control_signal(std::move(func), i_obj, j_obj) == fc_signal::k_break)
            break;
    }}
}

template <typename T, typename Func, typename Cont>
void quad_range(Cont & container, Func && func) {
    quad_range<T>(std::begin(container), std::end(container), std::move(func));
}

template <typename T, typename Func>
void quad_range(std::initializer_list<T> && ilist, Func && f) {
    quad_range<T>(ilist.begin(), ilist.end(), std::move(f));
}

// ----------------------------------------------------------------------------

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> & v) {
    if (v == sf::Vector2<T>())
        throw std::invalid_argument("Attempting to normalize the zero vector");
    return v*(T(1) / magnitude(v));
}

template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    normalize(T t)
{
    if (t == T(0))
        throw std::invalid_argument("Attempting to normalize the zero vector.");
    return (t < T(0)) ? -T(1) : T(1);
}

// ----------------------------------------------------------------------------

template <typename T>
sf::Vector2<T> center_of(const sf::Rect<T> & rect) {
    // omg, what's wrong with me?!
    return sf::Vector2<T>(rect.left + rect.width  / T(2),
                          rect.top  + rect.height / T(2));
}

template <typename T>
sf::Rect<T> compute_rectangle_intersection
    (const sf::Rect<T> & a, const sf::Rect<T> & b)
{
    using TVec = sf::Vector2<T>;
    auto high_a = TVec(right_of(a), bottom_of(a));
    auto high_b = TVec(right_of(b), bottom_of(b));

    auto low_rv  = TVec(std::max(a.left, b.left), std::max(a.top, b.top));
    auto high_rv = TVec(std::min(high_a.x, high_b.x), std::min(high_a.y, high_b.y));
    if (low_rv.x >= high_rv.x || low_rv.y >= high_rv.y) {
        return sf::Rect<T>();
    } else {
        return sf::Rect<T>(low_rv.x, low_rv.y,
                           high_rv.x - low_rv.x, high_rv.y - low_rv.y);
    }
}

// required to doc, the unit circle (frame of reference) for this function...
template <typename T>
sf::Vector2<T> rotate_vector(sf::Vector2<T> r, T rot) {
    // [r.x] * [ cos(rot) sin(rot)]
    // [r.y]   [-sin(rot) cos(rot)]
    return sf::Vector2<T>
        ( r.x*cosine(rot) - r.y*sine  (rot),
          r.x*sine  (rot) + r.y*cosine(rot));
}

template <typename T>
T angle_between(const sf::Vector2<T> & v, const sf::Vector2<T> & u) {
    static const constexpr T k_error = 0.00005;
    if (   (v.x*v.x + v.y*v.y) < k_error*k_error
        || (u.x*u.x + u.y*u.y) < k_error*k_error)
    {
        throw std::invalid_argument("angle_between: both vectors must be non-zero vectors.");
    }
    T frac = dot(v, u) / (magnitude(u)*magnitude(v));
    if      (frac > T( 1)) { frac = T( 1); }
    else if (frac < T(-1)) { frac = T(-1); }
    return arc_cosine(frac);
}

template <typename T>
sf::Vector2<T> project_onto(const sf::Vector2<T> & a, const sf::Vector2<T> & b) {
    static const constexpr T k_error = 0.00005;
    if ((a.x*a.x + a.y*a.y) < k_error*k_error)
        throw std::invalid_argument("project_onto: cannot project onto the zero vector.");
    return (dot(b, a)/(b.x*b.x + b.y*b.y))*b;
}

template <typename T>
sf::Vector2<T> major(const sf::Vector2<T> & v) {
    if (magnitude(v.x) < magnitude(v.y))
        return sf::Vector2<T>(T(0), v.y);
    else
        return sf::Vector2<T>(v.x, T(0));
}

template <typename Func>
std::pair<double, double> find_smallest_diff(Func && f, double hint, double error) {
    if (f(0)) {
        throw std::invalid_argument("find_smallest_diff: f(0) is true");
    }
    if (!f(1)) {
        throw std::invalid_argument("find_smallest_diff: f(1) is false");
    }

    bool   fg   = f(hint);
    double low  = fg ? 0    : hint;
    double high = fg ? hint : 1   ;

    while ((high - low) > error) {
        double t = low + (high - low)*0.5;
        if (f(t)) {
            high = t;
        } else {
            low = t;
        }
    }
    return std::make_pair(low, high);
}
