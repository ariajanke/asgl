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
    using UString      = Text::UString;
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

    void set_options(const std::vector<UString> &);

    void set_options(std::vector<UString> &&);

    void select_option(std::size_t index);

    std::size_t selected_option_index() const;

    const UString & selected_option() const;

    std::size_t options_count() const;

    void set_option_change_event(BlankFunctor && func);

    void swap(OptionsSlider &);

private:
    void draw_(WidgetRenderer &) const override;

    void issue_auto_resize() override;

    void iterate_children_(ChildWidgetIterator &) override;

    void iterate_children_const_(ChildWidgetIterator &) const override;

    void on_geometry_update() override;

    void set_location_(int x, int y) override;

    void set_arrow_events();

    void update_selections();

    ArrowButton m_left_arrow;
    ArrowButton m_right_arrow;

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

    BlankFunctor m_press_func = [](){};
    bool m_wrap_enabled = false;
};

} // end of ksg namespace
