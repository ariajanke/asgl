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
    using BlankFunctor = ArrowButton::BlankFunctor;

    enum { k_back_style, k_front_style, k_style_count };
    inline static StyleKey to_key(decltype(k_style_count) e)
        { return styles::StyleKeysEnum<decltype (k_style_count), k_style_count>::to_key(e); }

    OptionsSlider();
    OptionsSlider(const OptionsSlider &);
    ~OptionsSlider() override;

    OptionsSlider & operator = (const OptionsSlider &);
    OptionsSlider & operator = (OptionsSlider &&);

    void process_event(const Event &) override;

    Vector location() const override;

    Size size() const override;

    void stylize(const StyleMap &) override;

    void set_options(const std::vector<UString> &);

    void set_options(std::vector<UString> &&);

    void select_option(std::size_t index);

    std::size_t selected_option_index() const;

    const UString & selected_option() const;

    std::size_t options_count() const;

    void set_option_change_event(BlankFunctor && func);

    void swap(OptionsSlider &);

    void set_wrap_enabled(bool);

    void draw(WidgetRenderer &) const override;

private:
    void iterate_children_(const ChildWidgetIterator &) override;

    void iterate_children_const_(const ChildConstWidgetIterator &) const override;

    void update_size() override;

    void set_location_(int x, int y) override;

    void set_arrow_events();

    void update_selections();

    int padding() const;

    ArrowButton m_left_arrow;
    ArrowButton m_right_arrow;

    int m_padding = styles::k_uninit_size;
    ItemKey m_back;
    ItemKey m_front;

    StyleKey m_back_style    = to_key(k_back_style);
    StyleKey m_front_style   = to_key(k_front_style);
    StyleKey m_padding_style = Button::to_key(Button::k_button_padding);

    // boundry around the text (includes the padding)
    Rectangle m_inner_bounds;

    Text m_text;
    std::vector<UString> m_options;
    std::size_t m_selected_index = 0;

    BlankFunctor m_press_func = [](){};
    bool m_wrap_enabled = false;
};

} // end of asgl namespace
