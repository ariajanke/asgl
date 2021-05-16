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

#include <asgl/Widget.hpp>
#include <asgl/StyleMap.hpp>

namespace asgl {

class ProgressBar final : public Widget {
public:
    enum StyleEnum {
        k_outer_style, k_fill_style, k_void_style, k_padding_style,
        k_style_count
    };
    using DefaultStyles = styles::StyleKeysEnum<StyleEnum, k_style_count>;
    inline static StyleKey to_key(StyleEnum e)
        { return DefaultStyles::to_key(e); }

    /** ProgressBars do not respond to any event. */
    void process_event(const Event &) override {}

    VectorI location() const override;

    /** Sets the size of the control.
     *  @note if the size is smaller than the padding (times two), then the
     *        interior "fill" will not render
     */
    void set_size(int w, int h);

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;

    /** Sets outer style key.
     *
     */
    void set_outer_style(StyleKey);

    void set_fill_style(StyleKey);

    void set_void_style(StyleKey);

    void set_padding(int p);

    void set_padding(StyleKey);

    int padding() const;

    void set_fill_amount(float fill_amount);

    float fill_amount() const;

    void draw(WidgetRenderer &) const override;

private:
    void set_location_(int x, int y) override;

    void update_geometry() override;

    void verify_padding_set(const char * caller) const;

    int m_padding = styles::k_uninit_size;
    float m_fill_amount = 0.f;
    sf::IntRect m_bounds;
    sf::IntRect m_inner_bounds;

    StyleKey m_outer_key = DefaultStyles::to_key(k_outer_style  );
    StyleKey m_fill_key  = DefaultStyles::to_key(k_fill_style   );
    StyleKey m_void_key  = DefaultStyles::to_key(k_void_style   );
    StyleKey m_pad_key   = DefaultStyles::to_key(k_padding_style);

    ItemKey m_fill_style;
    ItemKey m_void_style;
    ItemKey m_outer_style;
};

} // end of asgl namespace
