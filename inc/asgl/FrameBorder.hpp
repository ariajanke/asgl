/****************************************************************************

    File: FrameBorder.hpp
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
#if 0
#include <ksg/Widget.hpp>
#include <ksg/Text.hpp>
#endif
#include <asgl/Draggable.hpp>
#include <asgl/Widget.hpp>
#include <asgl/Text.hpp>

#include <functional>

namespace asgl {

// ----------------------------------------------------------------------------
// ---                  Frame Helpers (FrameBorder Below)                   ---

class Frame;

namespace detail {

/** Sentinal Widget type used to force the parent frame to move a new line
 *  (prempts the "overflow" that too many widgets produce are put onto a line).
 *
 *  Features:
 *  - maintian a uniform interface for widgets and frame construction
 *  - forces the frame to move all following widgets to be added on the next
 *    line (see Frame declaration).
 */
class LineSeperator final : public Widget {
public:
    ~LineSeperator() override;

    void process_event(const Event &) override {}
#   if 0
    void set_location(float, float) override {}
#   endif
    VectorI location() const override { return VectorI(); }

    int width() const override { return 0; }

    int height() const override { return 0; }
#   if 0
    void set_style(const StyleMap &) override {}
#   endif
    void stylize(const StyleMap &) override {}
private:
    void set_location_(int, int) override {}
    void on_geometry_update() override {}
    void draw_(WidgetRenderer &) const override {}
#   if 0
    void draw(sf::RenderTarget &, sf::RenderStates) const override {}
#   endif
};

class HorizontalSpacer final : public Widget {
public:
    HorizontalSpacer(): m_width(0) {}

    void process_event(const Event &) override { }
#   if 0
    void set_location(float x_, float y_) override;
#   endif
    VectorI location() const override;

    int width() const override;

    int height() const override { return 0.f; }

    void set_width(int w);
#   if 0
    void set_style(const StyleMap &) override {}
#   endif
    void stylize(const StyleMap &) override {}
private:
#   if 0
    void draw(sf::RenderTarget &, sf::RenderStates) const override {}
#   endif
    void set_location_(int x, int y) override;

    void on_geometry_update() override {}
    void draw_(WidgetRenderer &) const override {}

    VectorI m_location;
    int m_width;
};

} // end of detail namespace

// ----------------------------------------------------------------------------

/** A helper class for Frame. The object manages the border graphics, events,
 *  and provides information for widget placement.
 *  Naturally the client only needs to concern themselves mostly with the
 *  title, styling, and click event function.
 */
class FrameBorder final : public Draggable
#   if 0
        , public sf::Drawable
#   endif
{
public:
#   if 0
    using VectorF = Widget::VectorF;
#   endif
    using VectorI = Widget::VectorI;
    using UString = Text::UString;
    enum ClickResponse {
        k_skip_other_events,
        k_continue_other_events
    };
    struct EventResponseSignal {
        bool skip_other_events      = false;
        bool should_update_geometry = false;
    };
#   if 0
    static constexpr const float k_default_padding = 2.f;
#   endif

    VectorI widget_start() const noexcept;

    VectorI location() const noexcept;

    int width() const noexcept;

    int height() const noexcept;

    EventResponseSignal process_event(const Event &);

    void set_location(int x, int y);

    // function renamed
#   if 0
    void set_style(const StyleMap &);
#   endif
    // takes styles from frame
    void stylize(const StyleMap &);

    void set_size(int w, int h);

    /** Sets the title of the border.
     *  @note Title bar becomes invisible if empty string is given. If a blank
     *        bar is desired for some reason, giving " " as the title will have
     *        the desired effect.
     *  @note An empty string will also disable dragging.
     */
    void set_title(const UString &);

    /** @brief Sets the font size for the border title.
     *  @param font_size font size in points
     */
    void set_title_size(int font_size);

    /** Sets the function/functor to call in the event that the mouse is
     *  clicked inside the frame.
     *  @param f function that takes no arguments and returns a ClickResponse
     *  If function f returns k_skip_other_events, then no other events which
     *  may occur with this frame will fire. If function f returns
     *  k_continue_other_events, then event processing will continue normally
     *  after f returns.
     *  @see Frame::ClickResponse
     */
    template <typename Func>
    void set_register_click_event(Func && f);

    /** Resets the register click event function back to its default value. */
    void reset_register_click_event();

    void update_geometry();

    /** @return the pixels needed to be set aside for rendering the title's
     *          width
     *  @note   accommodations for the title's height can be made from the
     *          difference between widget_start and location vertical distance.
     */
    int title_width_accommodation() const noexcept;

    /** @return the pixel width available for widgets */
    int width_available_for_widgets() const noexcept;

    void set_border_size(float pixels);

    void draw(WidgetRenderer &) const;

private:
    void update_drag_position(int drect_x, int drect_y) override;
#   if 0
    void draw(sf::RenderTarget &, sf::RenderStates) const override;
#   endif
    int title_height() const noexcept;

    void check_should_update_drag(const Event &);

    static ClickResponse do_default_click_event();

    int outer_padding() const noexcept;
#   if 0
    float m_outer_padding = styles::get_unset_value<float>();
#   endif
    int m_outer_padding = styles::k_uninit_size;
    bool m_recently_dragged = false;
#   if 0
    DrawRectangle m_back        = styles::make_rect_with_unset_color();
    DrawRectangle m_title_bar   = styles::make_rect_with_unset_color();
    DrawRectangle m_widget_body = styles::make_rect_with_unset_color();
#   endif

    sf::IntRect m_back, m_title_bar, m_widget_body;
    ItemKey m_title_bar_style;

    Text m_title;

    std::function<ClickResponse()> m_click_in_frame = do_default_click_event;
};

// ----------------------------------------------------------------------------

template <typename Func>
void FrameBorder::set_register_click_event(Func && f) {
    m_click_in_frame = std::move(f);
}

} // end of ksg namespace
