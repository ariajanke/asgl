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
#include <asgl/Text.hpp>

namespace asgl {

/** @brief The EditableText Widget allows a user to enter their own text.
 *
 *  It is encouraged that the client programmer constrain the dimensions of
 *  this widget, though it is not strictly required. @n
 *  For styles: for any style not set for constants defined by this class will
 *              fall back to those defined in the Frame class. @n
 *
 *  @note "in-order subsequence" is a term I use to describe a sequence of
 *        characters which are found in order but not necessarily contagious.
 *        For example " c-a | te-gory" contains an "in-order subsequence" of
 *        "category".
 */
class EditableText final : public FocusWidget {
public:
    enum StyleEnum {
        k_text_background_style, k_widget_border_style,
        k_widget_border_on_hover,
        k_fill_text_style, k_empty_text_style, k_cursor_style,
        k_style_count
    };
    inline static StyleKey to_key(StyleEnum e)
        { return styles::StyleKeysEnum<StyleEnum, k_style_count>::to_key(e); }

    using StringCheckFunc  = std::function<bool(const UString &, UString &)>;

    /** Width maybe set, though height may not be, as that is controlled by the
     *  font and character size.
     *  @note if left unitialized this control will take the width of the empty
     *        string
     *  @note this does not take into account for padding, the actual width of
     *        the control will very likely be larger (by 2*padding)
     */
    void set_text_width(int);

    /** Resets the control's width to fit the "empty" string.
     *  @note this is how instances are by default
     */
    void set_text_width_to_match_empty_text();

    /** Sets the function called whenever text is changed.
     *  @param func this function takes the following form:
     *         bool (UString & new_string, UString & display_string)
     *         - "new_string" parameter is the new string produced by the
     *           text entered event
     *         - "display_string" parameter is the current display string
     *           which maybe modified to this function's liking, the display
     *           string *must* be an in-order subsequence of the entered string
     *         - the return value is false to "reject" the new string, and
     *           true to "accept" the new string
     */
    void set_check_string_event(StringCheckFunc && func);

    /** When clicked with the mouse or "pressed" in any fashion, this control
     *  begins to request focus.
     */
    void process_event(const Event &) final;

    /** @copydoc asgl::Widget::location() */
    Vector location() const final { return m_location; }

    Size size() const final;

    /** Sets the style for the text cursor, "empty string" text, "display
     *  string" text, the background, and the border.
     */
    void stylize(const StyleMap &) final;
#   if 0
    /** @copydoc asgl::Widget::on_geometry_update() */
    void update_geometry() override;
#   endif
    /** If the display string is empty, then the "empty string" is shown. If
     *  this control has focus, then the text cursor is shown. Note that the
     *  display string may not match the actual "entered" string.
     */
    void draw(WidgetRenderer &) const final;

    /** Sets the text with a string that is displayed when the display string
     *  is empty (even if the actual string is non-empty).
     */
    void set_empty_string(const UString &);

    /** Calls the string check function, and then changes the entered string.
     *  @throws if the check fails, a std::invalid_argument is thrown
     */
    void set_entered_string(const UString &);

    /** @returns the actual text entered into the control, not to be confused
     *           with the "display string", though they sometimes have the
     *           same value
     */
    const UString & entered_string() const;

    /** The default behavior of the string change event. It accepts any string
     *  which does not contain many of the ascii control characters.
     *  @param new_string copied to display_string, regardless of contents
     *  @param display_string sets to new_string's value if no control
     *                        characters are found
     *  @returns true if the new_string contains no control characters
     */
    static bool default_check_string_event
        (const UString & new_string, UString & display_string);

    /** Checks if the display_string contains an in-order subsequence that is
     *  entered_string.
     *  @note this function should not ever throw
     *  @returns true if entered_string an in-order subsequence in
     *           display_string, false otherwise
     */
    static bool is_display_string_ok
        (const UString & display_string, const UString & entered_string);

    /** Finds the position in the display string that matches a position in the
     *  entered string.
     *  @throws if the entered_string is not contained as an in-order
     *           subsequence
     *  @throws if pos is greater than the size of the entered_string
     *  @param display_string target string of the search operation
     *  @param entered_string must be an in-order subsequence in display_string
     *  @param pos position in the entered_string
     *  @returns a constant iterator at the corresponding position in the
     *           display_string
     */
    static UStringConstIter find_display_position
        (const UString & display_string, const UString & entered_string,
         std::size_t pos);

private:
    void set_location_(int x, int y) final;

    void update_size() final;

    /** Responds to arrow keys, text entered, backspace, delete, home and end
     *  key presses.
     *
     *  Calls the check string event function everytime text is entered.
     */
    void process_focus_event(const Event &) final;

    void notify_focus_gained() final;

    void notify_focus_lost() final;

    int cursor_width() const { return m_padding; }

    int text_width() const;

    int text_height() const;

    void check_invarients() const;

    void handle_focused_key_typed(const KeyTyped &);

    void handle_focused_key_press(const KeyPress &);

    /** @returns true if character was deleted successfully */
    bool delete_character_at(std::size_t);

    Rectangle bounds() const;

    void update_internals_locations();

    int m_padding      = 0;
    int m_chosen_width = styles::k_uninit_size;
    int m_used_width   = 0;

    ItemKey m_border_appearance, m_border_hover_appearance;
    ItemKey m_area_appearance, m_cursor_appearance;
    Text m_display_left, m_display_right;
    Text m_empty_text;

    UString m_entered_string, m_display_string;

    Vector m_location;
    Rectangle m_cursor;
    /** indicates the position where a character is inserted in the entered string. */
    std::size_t m_edit_position = 0;

    StringCheckFunc m_string_check_func = default_check_string_event;
};

} // end of asgl namespace
