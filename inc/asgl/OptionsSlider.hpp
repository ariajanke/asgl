/****************************************************************************

    File: OptionsSlider.hpp
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
#include <asgl/ArrowButton.hpp>
#include <asgl/Text.hpp>

namespace asgl {

/** A compound control comprised of two buttons and text. Each button scrolls
 *  between a user defined set of options.
 *
 *  @note For now, assumes horizontal oreintation...
 *
 *  @note inherits many of its styles from Button
 */
class OptionsSlider final : public Widget {
public:
    using UString = Text::UString;
    using BlankFunctor = ArrowButton::BlankFunctor;

    OptionsSlider();
    OptionsSlider(const OptionsSlider &);
    ~OptionsSlider() override;

    OptionsSlider & operator = (const OptionsSlider &);
    OptionsSlider & operator = (OptionsSlider &&);

    void process_event(const Event &) override;

    VectorI location() const override;

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;
#   if 0
    /** @brief Sets the size of the widget by setting the size of it's interior.
     *
     *  This function will limit the size of the text to the given size. Unless
     *  OptionsSlider::k_size_unset is passed. In which case, the control will
     *  choose its own size in that dimension's regard.
     *
     *  @note The size of the arrows is determined by the height. They are made
     *        into squares.
     *  @param w width of the interior of the widget, where the text lives.
     *  @param h height of the widget, this is also the height and width of
     *         each arrow.
     */
    void set_interior_size(int w, int h);
#   endif
#   if 0
    void swap_options(std::vector<UString> &);
#   endif
    void set_options(const std::vector<UString> &);

    void set_options(std::vector<UString> &&);

    void select_option(std::size_t index);

    std::size_t selected_option_index() const;

    const UString & selected_option() const;

    std::size_t options_count() const;
#   if 0
        { return m_options.size(); }
#   endif
    void set_option_change_event(BlankFunctor && func);
#   if 0
    void set_wrap_enabled(bool);
#   endif
    void swap(OptionsSlider &);

private:
#   if 0
    void draw(sf::RenderTarget & target, sf::RenderStates) const override;
#   endif

    void draw_(WidgetRenderer &) const override;

    void issue_auto_resize() override;

    void iterate_children_(ChildWidgetIterator &) override;

    void iterate_children_const_(ChildWidgetIterator &) const override;

    void on_geometry_update() override;

    void set_location_(int x, int y) override;

    void set_arrow_events();
#   if 0
    void recenter_text();

    bool is_horizontal() const
        { return width() >= height(); }

    bool is_vertical() const
        { return !is_horizontal(); }

    float padding() const noexcept;
#   endif
    ArrowButton m_left_arrow;
    ArrowButton m_right_arrow;
#   if 0
    DrawRectangle m_back  = styles::make_rect_with_unset_color();
    DrawRectangle m_front = styles::make_rect_with_unset_color();
#   endif

    int m_padding = styles::k_uninit_size;
    ItemKey m_back;
    ItemKey m_front;

    StyleKey m_back_style = Button::to_key(Button::k_regular_back_style);
    StyleKey m_front_style = Button::to_key(Button::k_regular_front_style);
    StyleKey m_padding_style = Button::to_key(Button::k_button_padding);

    // boundry around the text (includes the padding)
    sf::IntRect m_inner_bounds;

    Text m_text;
    std::vector<UString> m_options;
    std::size_t m_selected_index = 0;
#   if 0
    float m_padding = styles::get_unset_value<float>();
    sf::Vector2f m_size;
#   endif
    BlankFunctor m_press_func = [](){};
    bool m_wrap_enabled = false;
};

} // end of ksg namespace
