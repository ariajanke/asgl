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

namespace asgl {

/** @brief A TextArea is an invisible rectangle wrapped around some blob of
 *         text.
 */
class TextArea final : public Widget {
public:
    using UString = Text::UString;

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

    UString give_cleared_string();

    const UString & string() const { return m_draw_text.string(); }

    void set_max_width(int w);

    void set_max_height(int h);

    static void set_required_text_fields
        (Text &, const StyleField * font, const StyleField * style_key,
         const char * full_call);

private:
    void set_location_(int x, int y) override;

    void draw_(WidgetRenderer &) const override;

    void issue_auto_resize() override;

    void on_geometry_update() override;

    Text m_draw_text;
};

} // end of ksg namespace
