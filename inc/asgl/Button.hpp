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

#include <asgl/FocusWidget.hpp>

#include <functional>

namespace asgl {

/** A button is any widget which has a click event.
 *
 *  It may also be highlighted, which is nothing more than a visual tell that
 *  the user may trigger the click event by clicking or by pressing the Return
 *  key.
 *
 *  This class uses a non-virtual interface for changes applied to it whether
 *  its size, highlight, deselect ("anti-highlight").
 */
class Button : public FocusWidget {
public:
    enum ButtonStyleEnum {
        k_regular_back_style, k_regular_front_style,
        k_hover_back_style  , k_hover_front_style  ,
        k_button_padding    ,
        k_style_count
    };

    inline static StyleKey to_key(ButtonStyleEnum e)
        { return styles::StyleKeysEnum<ButtonStyleEnum, k_style_count>::to_key(e); }

    /** Void returning parameterless functors are used to respond to events. */
    using BlankFunctor = std::function<void()>;

    /** @returns pixel location of the button. */
    VectorI location() const final;

    /** Allows the setting of the width and height of Button.
     *
     *  (what about TextButton??)
     *  @param w width  in pixels
     *  @param h height in pixels
     */
    void set_size(int w, int h);

    /** @return This returns width of the button in pixels. */
    int width() const final;

    /** @return This returns height of the button in pixels. */
    int height() const final;

    /** Listens for events that would press the button, and changes its
     *  appearance.
     *
     *  Buttons responds to movement of the cursor, and will change its style
     *  as the mouse hovers and leaves. Any mouse button release will "press"
     *  the button.
     *
     *  @see process_focus_event(const Event &)
     */
    void process_event(const Event &) override;

    /** Sets the press event which is called whenever the button is pressed.
     *
     *  That is when the user clicks/presses the Return key when the button is
     *  selected.
     *  @param func the callback function to call when the button is pressed
     */
    void set_press_event(BlankFunctor && func);

    /** Explicity fires the press event. (rather than having the user click it
     *  or press enter when active.)
     */
    void press();

    /** Sets button's foreground and background appearance.
     *
     *  Sets the following styles:
     *  - hover background color
     *  - hover foreground color
     *  - regular background color
     *  - regular foreground color
     */
    void stylize(const StyleMap &) override;

    /** Padding, which is applied both horizontally and vertically. Maybe
     *  useful with geometry updates.
     *  @note added to public interface, some composite widgets may need to
     *        know this widget's padding for consistency
     *  @return padding amount in pixels
     */
    int padding() const noexcept { return std::max(0, m_padding); }

    void process_focus_event(const Event &) final;

    void on_geometry_update() override;

    void draw(WidgetRenderer &) const override;

protected:
    /** Creates a zero-sized, white colored button. Pending setting of styles.
     */
    Button();

    /** Sets the size of the button's frame.
     *  @note Make sure to adjust for padding if necessary so that the button
     *        frame will not be too small.
     *  @param width  in pixels including padding
     *  @param height in pixels including padding
     */
    void set_button_frame_size(int width, int height);

    /** Change button aesthetics to denote a deselected button. */
    void deselect();

    /** Change button aesthetics to denote a selected button. */
    void highlight();

    /** When called, changes button's appearance. */
    void notify_focus_gained() final;

    /** When called, changes button's appearance. */
    void notify_focus_lost() final;

    /** When called, changes button's location. */
    void set_location_(int x, int y) final;

private:
    template <typename T>
    struct KeyPairImpl {
        KeyPairImpl() {}
        KeyPairImpl(T b_, T f_): back(b_), front(f_) {}
        T back ;
        T front;
    };
    using StyleKeyPair = KeyPairImpl<StyleKey>;
    using ItemKeyPair  = KeyPairImpl<ItemKey >;

    StyleKeyPair m_regular_keys = StyleKeyPair(to_key(k_regular_back_style), to_key(k_regular_front_style));
    StyleKeyPair m_hover_keys   = StyleKeyPair(to_key(k_hover_back_style  ), to_key(k_hover_front_style  ));
    ItemKeyPair m_regular_items;
    ItemKeyPair m_hover_items;

    ItemKeyPair m_active_items;

    sf::IntRect m_back, m_front;

    int m_padding = styles::k_uninit_size;
    bool m_is_highlighted = false;
    BlankFunctor m_press_functor = [](){};
};

} // end of asgl namespace
