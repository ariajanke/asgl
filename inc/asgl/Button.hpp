/****************************************************************************

    File: Button.hpp
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

#include <functional>

#include <asgl/FocusWidget.hpp>

namespace asgl {

/** A button is any widget which has a click event. It may also be highlighted,
 *  which is nothing more than a visual tell that the user may trigger the
 *  click event by clicking or by pressing the Return key.
 *
 *  === NON VIRTUAL INTERFACE ===
 *
 *  This class uses a non-virtual interface for changes applied to it whether
 *  its size, highlight, deselect ("anti-highlight").
 */
class Button : public FocusWidget {
public:
    using BlankFunctor = std::function<void()>;

    enum ButtonStyleEnum {
        k_regular_back_style, k_regular_front_style,
        k_hover_back_style  , k_hover_front_style  ,
        k_button_padding    ,
        k_style_count
    };
    using ButtonStyles = styles::StyleKeysEnum<ButtonStyleEnum, k_style_count>;
    inline static StyleKey to_key(ButtonStyleEnum e)
        { return ButtonStyles::to_key(e); }

private:
    void set_location_(int x, int y) final;

public:
    VectorI location() const final;

    /** Allows the setting of the width and height of Button
     *  @note the virtual on_size_changed method is available for any
     *        resize events if inheriting classes wishes to resize their
     *        internals
     *  @param w width  in pixels
     *  @param h height in pixels
     */
    void set_size(int w, int h);

    //! @return This returns width of the button in pixels.
    int width() const final;

    //! @return This returns height of the button in pixels.
    int height() const final;

    void process_event(const Event & evnt) override;

    /** Sets the press event which is called whenever the button is pressed.
     *  That is when the user clicks/presses the Return key when the button is
     *  selected.
     *  @param func the callback function to call when the button is pressed
     */
    void set_press_event(BlankFunctor && func);

    /** Explicity fires the press event. (rather than having the user click it
     *  or press enter when active.)
     */
    void press();

    /** @brief Sets button's styles.
     *
     *  Sets the following styles:
     *  - hover background color
     *  - hover foreground color
     *  - regular background color
     *  - regular foreground color
     *  @note when overriding, please don't forget to make this call
     */
    void stylize(const StyleMap &) override;

    /** Padding, which is applied both horizontally and vertically. Maybe
     *  useful with geometry updates.
     *  @note added to public interface, some composite widgets may need to
     *        know this widget's padding for consistency
     *  @return padding amount in pixels
     */
    int padding() const noexcept { return m_padding; }

protected:
    /** Creates a zero-sized, white colored button. Pending setting of styles.
     */
    Button();

    void draw_(WidgetRenderer &) const override;

    void on_geometry_update() override;

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

private:
    void process_focus_event(const Event &) override;

    void notify_focus_gained() override;

    void notify_focus_lost() override;

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

} // end of ksg namespace
