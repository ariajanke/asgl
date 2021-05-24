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

#include <asgl/Text.hpp>
#include <asgl/Widget.hpp>
#include <asgl/StyleMap.hpp>

namespace asgl {

/** @brief A TextArea is an invisible rectangle wrapped around some blob of
 *         text.
 */
class TextArea final : public Widget {
public:
    // <-------------------------- Basic Widget ------------------------------>

    /** Does nothing, as a plain text area does not interact with events. */
    void process_event(const Event &) final {}

    /** @copydoc Widget::location() */
    Vector location() const final;

    Size size() const final;

    void stylize(const StyleMap &) final;

    // <----------------------------- TextWidget ----------------------------->

    void set_string(const UString & str);

    void set_string(UString && str);

    UString give_cleared_string();

    const UString & string() const { return m_draw_text.string(); }

    /** @copydoc asgl::Text::set_limiting_line(int) */
    void set_limiting_line(int x_limit);

    void set_fixed_height(int height);

    /** @copydoc asgl::Text::set_viewport(const Rectangle&) */
    void set_viewport(const Rectangle &);

    /** @copydoc asgl::Text::reset_viewport() */
    void reset_viewport();

    /** @copydoc asgl::Text::viewport() */
    const Rectangle & viewport() const;

    static void set_required_text_fields
        (Text &, const StyleField * font, const StyleField * style_key,
         const char * full_call);

    void draw(WidgetRenderer &) const final;

    void update_size() final;

private:
    void set_location_(int x, int y) final;

    // not perfect, and possibly only useful for American English
    void check_and_adjust_for_text_too_big();

    Text m_draw_text;
    int m_height_fix = 0, m_controls_y = 0;
};

} // end of asgl namespace
