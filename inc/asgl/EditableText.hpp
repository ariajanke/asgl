/****************************************************************************

    File: EditableText.hpp
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

// off until I get that first demo program running!
// !!! KEEP THIS !!!
#if 0
#pragma once

#include <ksg/Widget.hpp>
#include <ksg/Text.hpp>
#include <ksg/FocusWidget.hpp>
#include <ksg/DrawTriangle.hpp>

#include <common/DrawRectangle.hpp>

namespace ksg {

namespace detail {

class Ellipsis final : public sf::Drawable {
public:
    using VectorF = sf::Vector2f;
    void set_size(float w, float h);
    void set_location(float x, float y);
    float width() const { return m_back.width(); }
private:

    static constexpr const int   k_dot_shape_count = 6;
    static constexpr const float k_inner_hsize = 3.f / 4.f;
    static constexpr const float k_inner_step = (1.f / 8.f)*k_inner_hsize;
    static constexpr const float k_top_line = 1.f / 3.f;
    static constexpr const float k_bottom_line = 2.f / 3.f;
    static constexpr const float k_hmargin = (1.f - k_inner_hsize) / 2.f;

    static VectorF point_a_for(int);
    static VectorF point_b_for(int);
    static VectorF point_c_for(int);

    void update_dots();

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    DrawRectangle m_back;
    FixedLengthArray<DrawTriangle, k_dot_shape_count> m_dots;
};

} // end of detail namespace

/** @brief The EditableText Widget allows a user to enter their own text.
 *
 *  It is encouraged that the client programmer constrain the dimensions of
 *  this widget, though it is not strictly required. \n
 *  If text overflows and ellipsis is rendered at the end without consideration
 *  for width/height constraints (an extreme case). \n
 *  For styles: for any style not set for constants defined by this class will
 *              fall back to those defined in the Frame class.
 */
class EditableText final : public FocusWidget {
public:
    using Ellipsis       = detail::Ellipsis;
    using UString        = Text::UString;
    using CharFilterFunc = std::function<bool(const UString &)>;
    using BlankFunc      = std::function<void()>;

    // always uses frame's border color, text font, padding
    static constexpr const char * const k_background_color    = "editable-text-background";
    static constexpr const char * const k_ellipsis_back_color = "editable-text-ellipsis-background";

    EditableText();

    /** Behavior differs depending on whether or not this widget has focus or
     *  not. Some behaviors take place regardless of focus. It will not change
     *  the text in response to an event.
     *  When the widget has focus: \n
     *  It will move the cursor in manner of accordance with any editable text
     *  box in response to the arrow keys.
     */
    void process_event(const sf::Event &) override;

    void set_location(float x, float y) override;

    VectorF location() const override;

    float width() const override;

    float height() const override;

    void set_style(const StyleMap &) override;

    void set_width(float);

    [[deprecated]] void set_text(const UString &);

    void set_string(const UString &);

    void set_cursor_position(int);

    int character_count() const;

    [[deprecated]] const UString & text() const;

    const UString & string() const;

    void set_character_size(int);

    void set_character_filter(CharFilterFunc &&);

    void set_text_change_event(BlankFunc &&);

private:
    float max_text_width() const;

    void process_focus_event(const sf::Event &) override;

    void notify_focus_gained() override;

    void notify_focus_lost() override;

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    bool need_ellipsis() const noexcept {
        if (!m_text.has_font_assigned()) return false;
        return m_text.measure_text(m_text.string()).width > max_text_width();
    }

    void update_geometry();

    void update_cursor();

    float padding() const noexcept;

    float inner_padding() const noexcept;

    Text m_text;
    DrawRectangle m_outer = styles::make_rect_with_unset_color();
    DrawRectangle m_inner = styles::make_rect_with_unset_color();
    DrawRectangle m_cursor;

    sf::Color m_focus_color = styles::get_unset_value<sf::Color>();
    sf::Color m_reg_color   = styles::get_unset_value<sf::Color>();

    float m_padding       = styles::get_unset_value<float>();
    float m_inner_padding = styles::get_unset_value<float>();

    Ellipsis m_ellipsis;

    CharFilterFunc m_filter_func = [](const UString &) { return true; };
    BlankFunc m_change_text_func = [](){};
};

} // end of ksg namespace
#endif
