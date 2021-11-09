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

/** @file asgl/SampleStyleValues.hpp
 *
 *  @see StyleMap
 *  Defines some sample style values for various builtin UI engines.
 */

namespace asgl {

namespace sample_style_values {

enum SampleStyleColor {
    k_primary_light,
    k_primary_mid,
    k_primary_dark,

    k_secondary_light,
    k_secondary_mid,
    k_secondary_dark,

    k_mono_light,
    k_mono_dark,

    k_color_count
};

enum SampleStyleValue {
    k_title_text,
    k_widget_text,

    k_editable_text_fill,
    k_editable_text_empty,

    k_bordered_regular_widget,
    k_bordered_hover_widget,
    k_bordered_focus_widget,
    k_bordered_hover_and_focus_widget,

    k_special_draw_item,

    k_other_style_count
};
#if 0
constexpr auto k_primary_light = SampleStyleColor::k_primary_light;
constexpr auto k_primary_mid   = SampleStyleColor::k_primary_mid;
constexpr auto k_primary_dark  = SampleStyleColor::k_primary_dark;

constexpr auto k_secondary_light = SampleStyleColor::k_secondary_light;
constexpr auto k_secondary_mid   = SampleStyleColor::k_secondary_mid;
constexpr auto k_secondary_dark  = SampleStyleColor::k_secondary_dark;

constexpr auto k_mono_light = SampleStyleColor::k_mono_light;
constexpr auto k_mono_dark  = SampleStyleColor::k_mono_dark;

constexpr int k_color_count = static_cast<int>(SampleStyleColor::k_color_count);

constexpr auto k_title_text  = SampleStyleValue::k_title_text;
constexpr auto k_widget_text = SampleStyleValue::k_widget_text;

constexpr auto k_editable_text_fill  = SampleStyleValue::k_editable_text_fill;
constexpr auto k_editable_text_empty = SampleStyleValue::k_editable_text_empty;

constexpr auto k_bordered_regular_widget         = SampleStyleValue::k_bordered_regular_widget;
constexpr auto k_bordered_hover_widget           = SampleStyleValue::k_bordered_hover_widget;
constexpr auto k_bordered_focus_widget           = SampleStyleValue::k_bordered_focus_widget;
constexpr auto k_bordered_hover_and_focus_widget = SampleStyleValue::k_bordered_hover_and_focus_widget;

constexpr auto k_special_draw_item = SampleStyleValue::k_special_draw_item;

constexpr int k_other_style_count = static_cast<int>(SampleStyleValue::k_other_style_count);
#endif
}  // end of sample_style_values namespace -> into ::asgl

using SampleStyleColor = sample_style_values::SampleStyleColor;
using SampleStyleValue = sample_style_values::SampleStyleValue;

} // end of asgl namespace
