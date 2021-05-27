#include <asgl/sfml/SfmlEngine.hpp>

#include <asgl/Frame.hpp>
#include <asgl/TextButton.hpp>

#include <common/SfmlVectorTraits.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Sleep.hpp>

#include <iostream>

#include <random>
#include <cassert>

namespace {

using CustomRectangleItem = asgl::SfmlFlatEngine::CustomRectangleItem;
using VectorD = cul::Vector2<double>;
using sf::Vertex;
using sf::Color;
using asgl::Frame;
using asgl::TextButton;

constexpr const double k_pi = cul::k_pi_for_type<double>;

struct RingMaker {
    virtual ~RingMaker() {}
    virtual void prepare_for_point_count(std::size_t incoming_points) = 0;
    virtual void place_outer_wedge(VectorD outer_a, VectorD outer_b, VectorD inner) = 0;
    virtual void place_inner_wedge(VectorD inner_a, VectorD inner_b, VectorD outer) = 0;
};

class RainbowRingMaker final : public RingMaker {
public:
    void prepare_for_point_count(std::size_t incoming_points) final {
        m_verticies.reserve(incoming_points);
    }
    void place_outer_wedge(VectorD outer_a, VectorD outer_b, VectorD inner) final {
        place_wedge(outer_a, outer_b, inner);
    }

    void place_inner_wedge(VectorD inner_a, VectorD inner_b, VectorD outer) final {
        place_wedge(inner_a, inner_b, outer);
    }

    std::vector<Vertex> give_product() {
        auto rv = std::move(m_verticies);
        m_verticies.clear();
        return rv;
    }

private:
    void place_wedge(VectorD a, VectorD b, VectorD c) {
        using cul::convert_to;
        auto color = get_and_advance_color();
        m_verticies.emplace_back(convert_to<VertVec>(a), color);
        m_verticies.emplace_back(convert_to<VertVec>(b), color);
        m_verticies.emplace_back(convert_to<VertVec>(c), color);
    }

    Color get_and_advance_color() {
        auto rv = *m_pallete_pos++;
        if (m_pallete_pos == k_pallete_end) m_pallete_pos = k_pallete_begin;
        return rv;
    }

    using VertVec = sf::Vector2f;
    static const Color * k_pallete_begin;
    static const Color * k_pallete_end  ;

    std::vector<Vertex> m_verticies;
    const Color * m_pallete_pos = k_pallete_begin;
};

class BubbleRingMaker final : public RingMaker {
public:
    void prepare_for_point_count(std::size_t incoming_points) final {
        // we're tetrasecting the wedges
        m_verticies.reserve(incoming_points*4);
    }

    void place_outer_wedge(VectorD outer_a, VectorD outer_b, VectorD inner) final {

        place_ring_wedge(outer_a, outer_b, inner, cul::normalize(inner)*m_radius);
    }

    void place_inner_wedge(VectorD inner_a, VectorD inner_b, VectorD outer) final {
        place_ring_wedge(inner_a, inner_b, outer, cul::normalize(outer)*m_radius);
        if (m_alpha_center != 0) {
            auto center_color = m_main_color;
            center_color.a = m_alpha_center;
            place_verticies(inner_a, inner_b, VectorD(),
                            outer_color(), outer_color(), center_color);
        }
    }

    std::vector<Vertex> give_product() {
        auto rv = std::move(m_verticies);
        m_verticies.clear();
        return rv;
    }

    void set_color(Color main_color) { m_main_color = main_color; }

    void set_alphas(uint8_t mid_ring, uint8_t outer_ring, uint8_t center) {
        m_alpha_mid_ring = mid_ring;
        m_alpha_outer_ring = outer_ring;
        m_alpha_center = center;
    }

    void set_radius(double rad) { m_radius = rad; }

private:
    Color outer_color() const {
        auto outer_color = m_main_color;
        outer_color.a = m_alpha_outer_ring;
        return outer_color;
    }

    void place_ring_wedge(VectorD in_a, VectorD in_b, VectorD out, VectorD mid) {
        using namespace cul;
        static const auto k_no_sol = get_no_solution_sentinel<VectorD>();

        VectorD intx_a, intx_b;
        {
        auto ex_a = mid + (in_a - in_b);
        auto ex_b = mid + (in_b - in_a);
        intx_a = find_intersection(ex_a, ex_b, in_a, out);
        intx_b = find_intersection(ex_a, ex_b, in_b, out);
        assert(intx_a != k_no_sol && intx_b != k_no_sol);
        }

        auto mid_color = m_main_color;
        mid_color.a = m_alpha_mid_ring;

        place_verticies(intx_a, intx_b, out,
                        mid_color, mid_color, outer_color());
        place_verticies(intx_a, mid, in_a,
                        mid_color, mid_color, outer_color());
        place_verticies(intx_b, mid, in_b,
                        mid_color, mid_color, outer_color());
        place_verticies(in_a, mid, in_b,
                        outer_color(), mid_color, outer_color());
    }
    void place_verticies
        (VectorD a, VectorD b, VectorD c,
         Color color_a, Color color_b, Color color_c)
    {
        using VertVec = sf::Vector2f;
        using cul::convert_to;
        m_verticies.emplace_back(convert_to<VertVec>(a), color_a);
        m_verticies.emplace_back(convert_to<VertVec>(b), color_b);
        m_verticies.emplace_back(convert_to<VertVec>(c), color_c);
    }

    uint8_t m_alpha_mid_ring   = 255;
    uint8_t m_alpha_outer_ring = 100;
    uint8_t m_alpha_center     = 0;

    Color m_main_color;

    double m_radius = 0.;

    std::vector<Vertex> m_verticies;
};

std::tuple<const Color *, const Color *> get_pallete();

/* private static */ const Color * RainbowRingMaker::k_pallete_begin = std::get<0>(get_pallete());
/* private static */ const Color * RainbowRingMaker::k_pallete_end   = std::get<1>(get_pallete());

std::tuple<const Color *, const Color *> get_pallete() {
    static const std::array arr {
        sf::Color(200,  0,  0),
        sf::Color(180, 80,  0),
        sf::Color(150,150,  0),
        sf::Color( 80,180,  0),
        sf::Color(  0,200,  0),
        sf::Color(  0,180, 80),
        sf::Color(  0,150,150),
        sf::Color(  0, 80,180),
        sf::Color(  0,  0,200),
        sf::Color( 80,  0,180),
        sf::Color(150,  0,150),
        sf::Color(180,  0, 80)
    };
    return std::make_tuple(arr.data(), arr.data() + arr.size());
}

void make_ring(RingMaker & ring_maker, double radius, double ring_delta, int resolution);

template <typename T>
cul::Rectangle<T> expand(cul::Rectangle<T> rect, T amount) {
    rect.left   -= amount;
    rect.top    -= amount;
    rect.width  += amount*2;
    rect.height += amount*2;
    return rect;
}

class BubbleBackground final : public sf::Drawable {
public:

    void update(double et) {
        double boost = 1.;
        if (m_bubbles.size() < 8) {
            boost *= double(8 - m_bubbles.size());
        } else if (m_bubbles.size() > 8){
            boost /= double(m_bubbles.size() - 8);
        }
        auto probability_per_frame = et*k_probability_per_second*boost;
        if (RealDistri(0., 1.)(m_rng) < probability_per_frame) {
            spawn_bubble();
        }
        update_bubbles(et);
    }

    void set_bounds(int x, int y, int width, int height) {
        m_bounds = cul::Rectangle<double>(x, y, width, height);
    }

private:
    using RealDistri  = std::uniform_real_distribution<double>;
    using IntDistri   = std::uniform_int_distribution<int>;
    using UInt8Distri = std::uniform_int_distribution<uint8_t>;

    static constexpr const double k_probability_per_second = 1.;

    void draw(sf::RenderTarget & target, sf::RenderStates states) const final {
        for (const auto & bubble : m_bubbles) {
            auto states_copy = states;
            states_copy.transform.translate(cul::convert_to<sf::Vector2f>(bubble.center));
            target.draw(bubble.verticies.data(), bubble.verticies.size(),
                        sf::PrimitiveType::Triangles, states_copy);
        }
    }

    void spawn_bubble() {
        using namespace cul;
        m_bubbles.emplace_back();
        SpawnParameters params;
        auto & bubble = m_bubbles.back();
        bubble.velocity = VectorD(0, -1)*double(IntDistri(params.min_speed, params.max_speed)(m_rng));
        bubble.radius   = IntDistri(params.min_radius, params.max_radius)(m_rng);
        bubble.center   = VectorD(IntDistri(m_bounds.left, right_of(m_bounds))(m_rng),
                                  bottom_of(m_bounds) + bubble.radius);
        BubbleRingMaker brm;
        uint8_t mid_alpha = 0;
        if (RealDistri(params.min_radius, params.max_speed)(m_rng) > bubble.radius) {
            mid_alpha = UInt8Distri(10, 50)(m_rng);
        }

        brm.set_alphas(UInt8Distri(180, 255)(m_rng), UInt8Distri(100, 180)(m_rng),
                       mid_alpha);
        brm.set_color(random_color(m_rng));
        brm.set_radius(bubble.radius);
        make_ring(brm, bubble.radius,
                  RealDistri(params.min_delta, params.max_delta)(m_rng),
                  std::max(12, int(std::ceil(bubble.radius / 5.))) );
        bubble.verticies = brm.give_product();
    }

    static Color random_color(std::default_random_engine & rng) {
        Color rv;
        std::array color_array = { &rv.r, &rv.g, &rv.b };
        std::shuffle(color_array.begin(), color_array.end(), rng);
        *color_array[0] = UInt8Distri(100, 255)(rng);
        if (UInt8Distri(0, 1)(rng) == 0)
            *color_array[1] = UInt8Distri(100, 255)(rng);
        else
            *color_array[1] = UInt8Distri(  0, 100)(rng);
        if (UInt8Distri(0, 3)(rng) == 0)
            *color_array[2] = UInt8Distri(100, 255)(rng);
        else
            *color_array[2] = UInt8Distri(  0, 100)(rng);
        return rv;
    }

    void update_bubbles(double et) {
        for (auto & bubble : m_bubbles) {
            bubble.center += bubble.velocity*et;
        }
        auto should_be_removed = [this](const Bubble & bubble) {
            auto ex_bounds = expand(m_bounds, bubble.radius);
            return !cul::is_contained_in(bubble.center, ex_bounds);
        };
        auto rm_beg = std::remove_if(
            m_bubbles.begin(), m_bubbles.end(), should_be_removed);
        m_bubbles.erase(rm_beg, m_bubbles.end());
    }

    struct Bubble {
        VectorD center;
        double  radius = 0.;
        VectorD velocity;
        std::vector<sf::Vertex> verticies;
    };

    struct SpawnParameters {
        double max_radius = 100.;
        double min_radius = 20.;
        double max_delta  = 0.4;
        double min_delta  = 0.1;
        double min_speed  = 10.;
        double max_speed  = 180.;
    };

    std::default_random_engine m_rng = std::default_random_engine { std::random_device()() };
    cul::Rectangle<double> m_bounds;
    std::vector<Bubble> m_bubbles;
};

class TopFrame final : public Frame {
public:
    void assign_exit_bool(bool & exit_bool) { m_exit_ptr = &exit_bool; }

    void setup();

private:
    bool * m_exit_ptr = nullptr;
    TextButton m_exit;
};

} // end of <anonymous> namespace

int main() {
    sf::RenderWindow win;
    win.create(sf::VideoMode(640, 480), " ");
    win.setFramerateLimit(20);
#   if 0
    auto view = win.getView();
    view.setCenter(0.f, 0.f);
    win.setView(view);
#   endif
#   if 0
    RainbowRingMaker rrm;
    make_ring(rrm, 190., 0.1, 19);
    //auto product = rrm.take_product();

    BubbleRingMaker brm;
    auto bubble_radius = 190.;
    brm.set_radius(bubble_radius);
    brm.set_alphas(200, 50, 0);
    brm.set_color(Color(80, 120, 220));
    make_ring(brm, bubble_radius, 0.3, 19);
    auto product = brm.give_product();
#   endif
    BubbleRingMaker brm;
    brm.set_radius(30.);
    make_ring(brm, 30., 0.1, 6);

    BubbleBackground bubble_background;
    bubble_background.set_bounds(0, 0, win.getSize().x, win.getSize().y);

    while (win.isOpen()) {
        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                win.close();
        }
        win.clear();
        bubble_background.update(1. / 60.);
        win.draw(bubble_background);
        win.display();
        sf::sleep(sf::microseconds(16667));
    }
}

namespace {

class OuterWedgeMaker {
public:
    OuterWedgeMaker(RingMaker & target): m_target(target) {}

    void push_point(VectorD v) {
        *m_pos++ = v;
        if (m_pos != m_triangle.end()) return;
        const auto & pts = m_triangle;
        m_target.place_outer_wedge(pts[2], pts[1], pts[0]);
        m_pos = m_triangle.begin();
    }

private:
    std::array<VectorD, 3> m_triangle;
    decltype (m_triangle)::iterator m_pos = m_triangle.begin();
    RingMaker & m_target;
};

void make_ring(RingMaker & ring_maker, double radius, double ring_delta, int resolution) {
    assert(resolution >= 0);
    if (radius < 0.25 || resolution < 6) return;

    using cul::normalize;
    using cul::rotate_vector;
    using VecD = VectorD;

    double step = k_pi*2. / double(resolution);

    ring_maker.prepare_for_point_count(resolution*2*3);

    OuterWedgeMaker outer_wedge_maker(ring_maker);
    VecD last_outer_wedge_point;
    for (int step_val = 0; step_val != resolution; ++step_val) {
        VecD inner_wedge_outer_pt;
        VecD wedge_a, wedge_b;

        {
        double t = step*double(step_val);
        VecD ray = VecD(std::cos(t), std::sin(t));
        VecD ring_mid = ray*radius;
        inner_wedge_outer_pt    = ring_mid*(1. + ring_delta);
        VecD step_into_ring     = ring_mid*(1. - ring_delta);
        auto inner_wedge_i_mag  = std::tan(step / 2.)*radius*(1. - ring_delta);
        wedge_a = step_into_ring + rotate_vector(ray,  float(k_pi / 2.f))*inner_wedge_i_mag;
        wedge_b = step_into_ring + rotate_vector(ray, -float(k_pi / 2.f))*inner_wedge_i_mag;
        }
        if (step_val == 0) {
            // wedge point chosen must "point" in the opposite direction of
            // rotation, I'm guessing here which
            last_outer_wedge_point = inner_wedge_outer_pt;
        } else {
            outer_wedge_maker.push_point(inner_wedge_outer_pt);
        }
        // the order at which points are pushed are *very* important
        // to make sure that points are pushed in order of parameters sent
        // to the maker
        outer_wedge_maker.push_point(wedge_a);
        outer_wedge_maker.push_point(inner_wedge_outer_pt);

        ring_maker.place_inner_wedge(wedge_a, wedge_b, inner_wedge_outer_pt);
    }
    outer_wedge_maker.push_point(last_outer_wedge_point);

}

} // end of <anonymous> namespace
