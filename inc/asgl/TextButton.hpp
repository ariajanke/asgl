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

#include <asgl/Button.hpp>
#include <asgl/Text.hpp>

namespace asgl {

class TextButton final : public Button {
public:
    using UString = Text::UString;
    /** TextButton defaults to TextArea text's styles */

    void set_string(const UString & str);

    void set_string(UString && str);

    UString give_cleared_string();

    void stylize(const StyleMap &) override;

    const UString & string() const;

private:
    void issue_auto_resize() override;

    void draw_(WidgetRenderer &) const override;

    void on_geometry_update() override;

    Text m_text;
};

} // end of ksg namespace
