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

    void stylize(const StyleMap &) final;

    const UString & string() const;

    void draw(WidgetRenderer &) const final;

private:
#   if 0
    void issue_auto_resize() override;

    void update_geometry() override;
#   endif

    void set_location_(int x, int y) final;

    void update_size() final;

    Text m_text;
};

} // end of asgl namespace
