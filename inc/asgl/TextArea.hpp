/****************************************************************************

    File: TextArea.hpp
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

#include <asgl/Text.hpp>
#include <asgl/Widget.hpp>
#include <asgl/StyleMap.hpp>
#if 0
#include <asgl/Frame.hpp>
#endif
namespace asgl {
#if 0
void set_if_present(Text &, const StyleMap &, const char * font_field,
                    const char * char_size_field, const char * text_color);
#endif
/** @brief A TextArea is an invisible rectangle wrapped around some blob of
 *         text.
 */
class TextArea final : public Widget {
public:
    using UString = Text::UString;
    enum StyleKeysEnum { k_text_color, k_text_size, k_styles_count };
    using DefaultStyles = styles::StyleKeysEnum<StyleKeysEnum, k_styles_count>;
    inline static StyleKey to_key(StyleKeysEnum e)
        { return DefaultStyles::to_key(e); }

    // <-------------------------- Basic Widget ------------------------------>

    /** Does nothing, as a plain text area does not interact with events. */
    void process_event(const Event &) override {}

    /** @copydoc Widget::location() */
    VectorI location() const override;

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;

    // <----------------------------- TextWidget ----------------------------->

    void set_string(const UString & str);

    void set_string(UString && str);

    UString take_cleared_string();

    const UString & string() const { return m_draw_text.string(); }
#   if 0
    void set_color(sf::Color);

    void set_color(StyleKey);

    void set_character_size(int size_);

    void set_character_size(StyleKey);

    void set_width(int w);

    void set_height(int h);
#   endif
    void set_max_width(int w);

    void set_max_height(int h);
#   if 0
    void set_size(int w, int h);

    void assign_font(const sf::Font & font);

    const sf::Font & assigned_font() const
        { return m_draw_text.assigned_font(); }

    bool has_font_assigned() const
        { return m_draw_text.has_font_assigned(); }

    int character_size() const
        { return m_draw_text.character_size(); }
#   endif
    static void set_required_text_fields
        (Text &, const StyleField * font, const StyleField * color,
         const StyleField * character_size, const char * full_call);

private:
    void set_location_(int x, int y) override;

    void draw_(WidgetRenderer &) const override;

    void issue_auto_resize() override;

    void on_geometry_update() override;

    StyleKey m_color_key = DefaultStyles::to_key(k_text_color);
    StyleKey m_size_key  = DefaultStyles::to_key(k_text_size );
    Text m_draw_text;
};

} // end of ksg namespace
