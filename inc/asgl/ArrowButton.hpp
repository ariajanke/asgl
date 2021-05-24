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

namespace asgl {

/** An Arrow Button is a clickable button with an arrow facing in one of four
 *  possible directions.
 *  @note While it is up to the renderer on what to do with triangle shape,
 *        this button will maintain its geometry regardless.
 */
class ArrowButton final : public Button {
public:
    enum StylesEnum { k_triangle_style, k_style_count };
    inline static StyleKey to_key(StylesEnum e)
        { return styles::StyleKeysEnum<StylesEnum, k_style_count>::to_key(e); }

    enum class Direction {
        k_up, k_down, k_right, k_left,
        k_none
    };

    ArrowButton();

    /** Changes the direction of the button, a lone point of the triangle will
     *  appear on the "nside" of the button where "n" is the direction.
     *  @param dir_ new direction of the button
     */
    void set_direction(Direction dir_);

    /** @returns currect direction of the arrow. */
    Direction direction() const { return m_dir; }

    /** Arrow button ignores events if its direction is "k_none". Otherwise it
     *  behaves like a regular button.
     */
    void process_event(const Event &) override;

    /** The arrow itself takes an ItemKey style, which is then sent to the
     *  widget renderer/engine.
     */
    void stylize(const StyleMap &) override;

    void set_size(int w, int h)
        { set_button_frame_size(w, h); }

    void draw(WidgetRenderer &) const override;

private:
    void update_points();

    void set_location_(int x, int y) override {
        Button::set_location_(x, y);
        update_points();
    }

    void update_size() override {}

    ItemKey m_triangle_style;
    Triangle m_tri;
    Direction m_dir;
};

} // end of asgl namespace
