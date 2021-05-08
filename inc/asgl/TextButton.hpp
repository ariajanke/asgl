/****************************************************************************

    File: TextButton.hpp
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
#include <ksg/Button.hpp>
#include <ksg/Text.hpp>
#endif

#include <asgl/Button.hpp>
#include <asgl/Text.hpp>

namespace asgl {

class TextButton final : public Button {
public:
    using UString = Text::UString;
    /** TextButton defaults to TextArea text's styles */

#   if 0
    static constexpr const char * const k_text_color = "text-button-text-color";
    static constexpr const char * const k_text_size  = "text-button-text-size";

    TextButton();

    void swap_string(UString & str);
#   endif
    void set_string(const UString & str);

    void set_string(UString && str);

    UString take_cleared_string();

    void stylize(const StyleMap &) override;
#   if 0
    void set_location(float x, float y) override;
#   endif
    const UString & string() const;

private:
    /** Sets the maximum size of the text button.
     *  @param w width in pixels
     *  @param h height in pixels
     */
    void set_size_back(int w, int h) override;

    void issue_auto_resize() override;
#   if 0
    void update_text_geometry(float w, float h);

    void draw(sf::RenderTarget & target, sf::RenderStates) const override;
#   endif
    void on_size_changed(int, int) override;
#   if 0
    void update_string_position();
#   endif
    void draw_(WidgetRenderer &) const override;

    void on_geometry_update() override;

    Text m_text;
};

} // end of ksg namespace
