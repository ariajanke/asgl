/****************************************************************************

    File: ProgressBar.hpp
    Author: Aria Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once
#if 0
#include <common/DrawRectangle.hpp>
#endif
#include <asgl/Widget.hpp>
#include <asgl/StyleMap.hpp>

namespace asgl {

class ProgressBar final : public Widget {
public:
    enum StyleEnum {
        k_outer_style, k_fill_style, k_void_style, k_padding_style,
        k_style_count
    };
    using DefaultStyles = styles::StyleKeysEnum<StyleEnum, k_style_count>;
    inline static StyleKey to_key(StyleEnum e)
        { return DefaultStyles::to_key(e); }
#   if 0
    static constexpr const char * const k_outer_color       = "progress-bar-outer-color";
    static constexpr const char * const k_inner_front_color = "progress-bar-inner-front-color";
    static constexpr const char * const k_inner_back_color  = "progress-bar-inner-back-color";
    static constexpr const char * const k_padding           = "progress-bar-padding";
#   endif
    /** ProgressBars do not respond to any event. */
    void process_event(const Event &) override {}

    VectorI location() const override;

    /** Sets the size of the control.
     *  @note if the size is smaller than the padding (times two), then the
     *        interior "fill" will not render
     */
    void set_size(int w, int h);

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;
#   if 0
    void set_outer_color(sf::Color);

    void set_inner_front_color(sf::Color);

    void set_inner_back_color(sf::Color);
#   endif

    /** Sets outer style key.
     *
     */
    void set_outer_style(StyleKey);

    void set_fill_style(StyleKey);

    void set_void_style(StyleKey);

    void set_padding(int p);

    void set_padding(StyleKey);

    int padding() const;

    void set_fill_amount(float fill_amount);

    float fill_amount() const;

private:
    void set_location_(int x, int y) override;

    void on_geometry_update() override;

    void draw_(WidgetRenderer &) const override;

    void verify_padding_set(const char * caller) const;

#   if 0
    float active_padding() const;

    void update_positions_using_outer();

    void update_sizes_using_outer();
#   endif

    int m_padding = styles::k_uninit_size;
    float m_fill_amount = 0.f;
    sf::IntRect m_bounds;
    sf::IntRect m_inner_bounds;

    StyleKey m_outer_key = DefaultStyles::to_key(k_outer_style  );
    StyleKey m_fill_key  = DefaultStyles::to_key(k_fill_style   );
    StyleKey m_void_key  = DefaultStyles::to_key(k_void_style   );
    StyleKey m_pad_key   = DefaultStyles::to_key(k_padding_style);

    ItemKey m_fill_style;
    ItemKey m_void_style;
    ItemKey m_outer_style;
#   if 0
    DrawRectangle m_outer       = styles::make_rect_with_unset_color();
    DrawRectangle m_inner_front = styles::make_rect_with_unset_color();
    DrawRectangle m_inner_back  = styles::make_rect_with_unset_color();

    float m_fill_amount = 0.f;
    float m_padding = styles::get_unset_value<float>();
#   endif
};

} // end of ksg namespace
