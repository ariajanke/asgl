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

// I have a plan for this, but it involves seperating frame border from frame.
#if 0
#pragma once

#include <asgl/FocusWidget.hpp>
#include <asgl/Frame.hpp>
#include <asgl/Text.hpp>

namespace asgl {

struct SelectionEntryReciever {
    virtual ~SelectionEntryReciever();
    virtual void activate(std::size_t menu_idx) = 0;
    virtual void deactivate(std::size_t menu_idx) = 0;
};

// Selection entries may have size constraints imposed on them...!
class SelectionEntry : public BareFrame, public FocusReceiver {
public:
    using UString = Text::UString;

    void assign_parent(SelectionEntryReciever &, std::size_t menu_idx);

    void process_event(const Event &) override;

private:
    void process_focus_event(const Event &) override;

    void notify_focus_gained() override;

    void notify_focus_lost() override;

    SelectionEntryReciever * m_parent = nullptr;
    std::size_t m_menu_idx = 0;

    ItemKey m_max_highlight, m_reg_highlight, m_no_highlight;
    bool m_mouse_is_over = false;
};

// number of items may vary

class SelectionMenu final : public FocusWidget, public SelectionEntryReciever {
public:
    using UString       = Text::UString;
    using EntryVector   = std::vector<SelectionEntry>;
    using EntryIterator = EntryVector::iterator;
    using ResponseFunc  = std::function<void(std::size_t, const UString &)>;

    EntryIterator begin() { return m_entries.begin(); }

    EntryIterator end() { return m_entries.end(); }

    void set_response_function(ResponseFunc &&);

    void set_size(float width, float height);

    static void default_response_function
        (std::size_t menu_index, const UString & entry_text);

private:
    static constexpr const auto k_uninit = std::string::npos;

    void process_event(const Event &) override;

    void set_location_(int x, int y) override;

    VectorI location() const override;

    int width() const override;

    int height() const override;

    void stylize(const StyleMap &) override;

    void iterate_children_(ChildWidgetIterator &) override;

    void iterate_children_const_(ChildWidgetIterator &) const override;
#   if 0
    void draw(sf::RenderTarget &, sf::RenderStates) const override;
#   endif
    void draw_(WidgetRenderer &) const override;

    void activate(std::size_t) override;

    void deactivate(std::size_t) override;

    void issue_auto_resize() override;

    void on_geometry_update() override;

    ResponseFunc m_resp_func = default_response_function;
    std::vector<SelectionEntry> m_entries;
#   if 0
    sf::FloatRect m_bounds;
#   endif
    sf::IntRect m_bounds;
    sf::IntRect m_selected;
#   if 0
    DrawRectangle m_selected;
#   endif
    std::size_t m_last_selected = k_uninit;
};
#if 0
namespace detail {

void add_selection_menu_default_styles(StyleMap &);

} // end of detail namespace -> into ::asgl
#endif
} // end of asgl namespace
#endif
#if 0
/****************************************************************************

    File: SelectionMenu.hpp
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

#include <ksg/FocusWidget.hpp>
#include <ksg/Text.hpp>

#include <common/DrawRectangle.hpp>

namespace ksg {

struct SelectionEntryReciever {
    virtual ~SelectionEntryReciever();
    virtual void activate(std::size_t menu_idx) = 0;
    virtual void deactivate(std::size_t menu_idx) = 0;
};

class SelectionEntry final : public FocusWidget {
public:
    static constexpr const float k_default_padding = 2.f;
    using UString = Text::UString;
#   if 0
    SelectionEntry();
#   endif
    void assign_parent(SelectionEntryReciever &, std::size_t menu_idx);

    void set_string(const UString &);

    void set_string(UString &&);

    void process_event(const sf::Event &) override;

    void set_location(float x, float y) override;

    VectorF location() const override;

    float width() const override;

    float height() const override;

    void set_size(float width, float height);

    const UString & string() const;

    void set_style(const StyleMap &) override;

    // based on content, not the wrapping
    float content_width() const;

    // based on content, not the wrapping
    float content_height() const;
private:
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void process_focus_event(const sf::Event &) override;

    void notify_focus_gained() override;

    void notify_focus_lost() override;

    void update_highlight();

    void recenter_text();

    float padding() const noexcept;

    Text m_display_text;
    float m_padding = styles::get_unset_value<float>();
    DrawRectangle m_background;

    SelectionEntryReciever * m_parent = nullptr;
    std::size_t m_menu_idx = 0;
    sf::Color m_max_highlight = styles::get_unset_value<sf::Color>();
    sf::Color m_reg_highlight = styles::get_unset_value<sf::Color>();
    sf::Color m_no_highlight  = styles::get_unset_value<sf::Color>();
    bool m_mouse_is_over = false;
};

// number of items may vary

class SelectionMenu final : public Widget, public SelectionEntryReciever {
public:
    using UString       = Text::UString;
    using EntryVector   = std::vector<SelectionEntry>;
    using EntryIterator = EntryVector::iterator;
    using ResponseFunc  = std::function<void(std::size_t, const UString &)>;

    static constexpr const char * const k_max_highlight     = "selection-menu-max-highlight";
    static constexpr const char * const k_regular_highlight = "selection-menu-reg-highlight";
    static constexpr const char * const k_no_highlight      = "selection-menu-no-highlight";

    void add_options(std::vector<UString> &&);

    EntryIterator begin() { return m_entries.begin(); }

    EntryIterator end() { return m_entries.end(); }

    template <typename Func>
    void set_response_function(Func && f) { m_resp_func = std::move(f); }

    void set_size(float width, float height);

    static void default_response_function
        (std::size_t menu_index, const UString & entry_text);

private:
    static constexpr const auto k_uninit = std::string::npos;

    void process_event(const sf::Event &) override;

    void set_location(float x, float y) override;

    VectorF location() const override;

    float width() const override;

    float height() const override;

    void set_style(const StyleMap &) override;

    void iterate_children_(ChildWidgetIterator &) override;

    void iterate_const_children_(ChildWidgetIterator &) const override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void activate(std::size_t) override;

    void deactivate(std::size_t) override;

    void issue_auto_resize() override;

    ResponseFunc m_resp_func = default_response_function;
    std::vector<SelectionEntry> m_entries;
    sf::FloatRect m_bounds;
    DrawRectangle m_selected;
    std::size_t m_last_selected = k_uninit;
};

namespace detail {

void add_selection_menu_default_styles(StyleMap &);

}

} // end of ksg namespace
#endif
