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
 */
class ArrowButton final : public Button {
public:
    enum StylesEnum {
        k_triangle_style, k_style_count
    };
    using DefaultStyles = styles::StyleKeysEnum<StylesEnum, k_style_count>;
    inline static StyleKey to_key(StylesEnum e)
        { return DefaultStyles::to_key(e); }

    enum class Direction {
        k_up, k_down, k_right, k_left,
        k_none
    };

    ArrowButton();

    void set_direction(Direction dir_);

    Direction direction() const { return m_dir; }

    void process_event(const Event & evnt) override;

    void stylize(const StyleMap &) override;

    void set_arrow_style(ItemKey);

    void on_geometry_update() override;

private:
    void draw_(WidgetRenderer &) const override;

    void update_points();

    ItemKey m_triangle_style;
    TriangleTuple m_tri;
    Direction m_dir;
};

} // end of ksg namespace
