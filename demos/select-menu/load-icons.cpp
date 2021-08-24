/****************************************************************************

    Copyright 2021 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#include "load-icons.hpp"

#include <asgl/sfml/SfmlEngine.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include <cassert>

namespace {

using asgl::Vector;
using asgl::Size;
using asgl::Rectangle;
using namespace cul::exceptions_abbr;

Grid<sf::Color> load_image(const std::string & fn);
Grid<sf::Color> make_shadow_image(const Grid<bool> & shadow_mask, int icon_size, int expansion_size);
std::vector<Rectangle> make_frames_for_size(int width_in_frames, int height_in_frames, int frame_size);

} // end of <anonymous> namespace

LoadIconsRt load_icons
    (const std::string & filename, asgl::SfmlFlatEngine & engine,
     int icon_size, int expansion_size, int opacity_limit)
{
    auto image_grid = load_image(filename);

    Grid<bool> mask;
    // guess reserve
    mask.set_size(image_grid.width(), image_grid.height(), false);
    for (Vector r; r != image_grid.end_position(); r = image_grid.next(r)) {
        if (image_grid(r).a < opacity_limit) continue;
        mask(r) = true;
    }

    // only do full icons
    // I want to do the smallest step at a time

    Grid<sf::Color> icon_shadows = make_shadow_image(mask, icon_size, expansion_size);
    int width_in_frames  = image_grid.width () / icon_size;
    int height_in_frames = image_grid.height() / icon_size;
    std::vector<Rectangle> icon_frames
        = make_frames_for_size(width_in_frames, height_in_frames, icon_size);
    std::vector<Rectangle> icon_shadow_frames = make_frames_for_size
        (width_in_frames, height_in_frames, icon_size + expansion_size*2);

    LoadIconsRt rv;
    rv.icon_frames = std::move(icon_frames);
    rv.icon_shadows_frames = std::move(icon_shadow_frames);
    rv.mask_ptr = std::make_shared<const Grid<bool>>( std::move(mask) );
    rv.icons = engine.make_image_from(image_grid);
    rv.icon_shadows = engine.make_image_from(icon_shadows);
    return rv;
}

namespace {

const sf::Color k_transparency(0x0);

Grid<bool> make_icon_shadows(const Grid<bool> & shadow_mask, int icon_size, int expansion_size);
Grid<bool> grow_shadow_mask(const Grid<bool> & shadow_mask, int expansion_size);
void mark_pixel(sf::Color & pixel, Vector r);

Grid<sf::Color> load_image(const std::string & fn) {
    sf::Image img;
    if (!img.loadFromFile(fn)) {
        throw InvArg("Cannot load file \"" + fn + "\".");
    }
    Grid<sf::Color> imgout;
    imgout.set_size(int(img.getSize().x), int(img.getSize().y));
    for (unsigned y = 0; y != img.getSize().y; ++y) {
    for (unsigned x = 0; x != img.getSize().x; ++x) {
        imgout(int(x), int(y)) = img.getPixel(x, y);
    }}
    return imgout;
}

Grid<sf::Color> make_shadow_image(const Grid<bool> & shadow_mask, int icon_size, int expansion_size) {
    auto icon_shadows = make_icon_shadows(shadow_mask, icon_size, expansion_size);
    icon_shadows = grow_shadow_mask(icon_shadows, expansion_size);

    {
    sf::Image img;
    img.create(unsigned(icon_shadows.width()), unsigned(icon_shadows.height()));
    for (Vector r; r != icon_shadows.end_position(); r = icon_shadows.next(r)) {
        img.setPixel(unsigned(r.x), unsigned(r.y), icon_shadows(r) ? sf::Color::Black : sf::Color::White);
    }
    img.saveToFile("/media/ramdisk/firstmask.png");
    }
    Grid<sf::Color> image;
    image.set_size(icon_shadows.width(), icon_shadows.height(), k_transparency);
    for (Vector r; r != image.end_position(); r = image.next(r)) {
        if (!icon_shadows(r)) continue;
        mark_pixel(image(r), r);
    }
    return image;
}

std::vector<Rectangle> make_frames_for_size(int width_in_frames, int height_in_frames, int frame_size) {
    std::vector<Rectangle> rv;
    rv.reserve(width_in_frames*height_in_frames);
    for (Vector icon_r; icon_r.y != height_in_frames; ++icon_r.y) {
    for (icon_r.x = 0  ; icon_r.x != width_in_frames ; ++icon_r.x) {
        rv.emplace_back(icon_r*frame_size, Size(1, 1)*frame_size);
    }}
    return rv;
}

// ----------------------------------------------------------------------------

const auto k_shadow_pallete = {
    sf::Color(0xD0D0D0FF),
    sf::Color(0xFFFFFFFF),
    sf::Color(0xD0D0D0FF),

    k_transparency,
    k_transparency,

    sf::Color(0x202020FF),
    sf::Color(0x000000FF),
    sf::Color(0x202020FF)
};

template <typename T>
bool any_are_edge(const Grid<T> & grid, Vector r, const T & inside_value);

void clean_up_edge_pixels(std::vector<Vector> & edge_pixels);

Grid<bool> expand_mask
    (const Grid<bool> & mask, std::vector<Vector> edge_pixels_copy,
     std::size_t expansion_size, std::size_t reserve_offset);

Grid<bool> make_icon_shadows(const Grid<bool> & shadow_mask, int icon_size, int expansion_size) {
    Grid<bool> icon_shadows;
    const int shadow_size = icon_size + expansion_size*2;
    icon_shadows.set_size((shadow_mask.width () / icon_size)*shadow_size,
                          (shadow_mask.height() / icon_size)*shadow_size,
                          false);
    for (Vector icon_r; icon_r.y != shadow_mask.height() / icon_size; ++icon_r.y) {
    for (icon_r.x = 0  ; icon_r.x != shadow_mask.width () / icon_size; ++icon_r.x) {
        auto icon_tile   = make_sub_grid(shadow_mask , icon_r*icon_size, icon_size, icon_size);
        auto shadow_tile = make_sub_grid(icon_shadows, icon_r*shadow_size, shadow_size, shadow_size);
        // note the following:
        assert(icon_tile.width() + expansion_size*2 == shadow_tile.width());
        Vector shadow_offset = Vector(1, 1)*expansion_size;
        for (Vector r; r != icon_tile.end_position(); r = icon_tile.next(r)) {
            if (!icon_tile(r)) continue;
            shadow_tile(r + shadow_offset) = true;
        }
    }}
    return icon_shadows;
}

Grid<bool> grow_shadow_mask(const Grid<bool> & shadow_mask, int expansion_size) {
    std::vector<Vector> edge_pixels;
    // guess reserve
    edge_pixels.reserve( shadow_mask.size() / 4 );
    for (Vector r; r != shadow_mask.end_position(); r = shadow_mask.next(r)) {
        if (!shadow_mask(r)) continue;
        if (any_are_edge(shadow_mask, r, true)) {
            edge_pixels.push_back(r);
        }
    }

    clean_up_edge_pixels(edge_pixels);
    return expand_mask(shadow_mask, edge_pixels, expansion_size,
                       std::max(shadow_mask.width(), shadow_mask.height())*4);
}

void mark_pixel(sf::Color & pixel, Vector r) {
    int idx = (r.x + r.y % int(k_shadow_pallete.size())) % int(k_shadow_pallete.size());
    pixel = *(k_shadow_pallete.begin() + idx);
}

// ----------------------------------------------------------------------------

const auto k_neighbors =
    { Vector(1, 0), Vector(-1, 0), Vector(0, 1), Vector(0, -1) };

template <typename Func>
inline void do_n_times(int n, Func && f)
    { for (int i = 0; i != n; ++i) f(); }

inline auto make_strict_weak_vector_orderer(const std::vector<Vector> & col) {
    int max_x = std::max_element(col.begin(), col.end(),
        [](const Vector & a, const Vector & b) { return a.x < b.x; })->x + 1;
    if (max_x*max_x <= max_x) {
        throw OorError("make_strict_weak_vector_orderer: maximum value x "
                       "squared overflows the data type.");
    }
    return [max_x](const Vector & a, const Vector & b) {
        int a_v = a.x + a.y*max_x;
        int b_v = b.x + b.y*max_x;
        return a_v < b_v;
    };
}

template <typename T>
bool any_are_edge(const Grid<T> & grid, Vector r, const T & inside_value) {
    assert(grid.has_position(r));
    assert(grid(r) == inside_value);
    for (auto n : k_neighbors) {
        if (!grid.has_position(n + r)) {
            // image edge pixels are considered on the edge
            return true;
        } else if (grid(n + r) != inside_value) {
            return true;
        }
    }
    return false;
}

void clean_up_edge_pixels(std::vector<Vector> & edge_pixels) {
    std::sort(edge_pixels.begin(), edge_pixels.end(), make_strict_weak_vector_orderer(edge_pixels));
    static const Vector k_remove_cand(-1, -1);
    static auto is_remove_cand = [](Vector r) { return r == k_remove_cand; };
    if (std::any_of(edge_pixels.begin(), edge_pixels.end(), is_remove_cand)) {
        throw InvArg("No edge pixel maybe the remove candidate (should not be possible, the remove candidate is not a valid location on *any* grid).");
    }
    if (edge_pixels.size() > 1) {
        auto itr = edge_pixels.begin();
        for (auto jtr = itr + 1; jtr != edge_pixels.end(); ++jtr, ++itr) {
            if (*itr != *jtr) continue;
            *itr = k_remove_cand;
        }
    }
    auto rem_itr = std::remove_if(edge_pixels.begin(), edge_pixels.end(), is_remove_cand);
    edge_pixels.erase(rem_itr, edge_pixels.end());
}

Grid<bool> expand_mask
    (const Grid<bool> & mask, std::vector<Vector> edge_pixels_copy,
     std::size_t expansion_size, std::size_t reserve_offset)
{
    auto temp_mask = mask;
    do_n_times(expansion_size, [&]() {
        std::vector<Vector> new_edge_pixels;
        new_edge_pixels.reserve(edge_pixels_copy.size() + reserve_offset);
        for (auto r : edge_pixels_copy) {
            assert(temp_mask(r));
            for (auto n : k_neighbors) {
                if (!temp_mask.has_position(n + r)) continue;
                if (!temp_mask(n + r)) {
                    temp_mask(n + r) = true;
                    new_edge_pixels.push_back(n + r);
                }
            }
        }
        clean_up_edge_pixels(new_edge_pixels);
        new_edge_pixels.swap(edge_pixels_copy);
    });
    return temp_mask;
}

} // end of <anonymous> namespace
