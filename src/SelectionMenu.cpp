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

#if 0
#include <ksg/SelectionMenu.hpp>
#include <ksg/TextArea.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#if 0
#include <iostream>
#endif
using VectorF = ksg::SelectionMenu::VectorF;
using UString = ksg::Text::UString;

namespace {

const auto k_default_max_highlight = sf::Color(40, 40, 140);
const auto k_regular_highlight = sf::Color(0, 10, 80);
const auto k_no_highlight = (sf::Color(0, 0, 50));

} // end of <anonymous> namespace

namespace ksg {

SelectionEntryReciever::~SelectionEntryReciever() {}

// ----------------------------------------------------------------------------
#if 0
SelectionEntry::SelectionEntry():
    m_max_highlight(k_default_max_highlight),
    m_reg_highlight(k_regular_highlight),
    m_no_highlight(k_no_highlight)
{}
#endif
void SelectionEntry::assign_parent(SelectionEntryReciever & parent, std::size_t menu_idx) {
    m_parent   = &parent;
    m_menu_idx = menu_idx;
}

void SelectionEntry::set_string(const UString & ustr) {
    m_display_text.set_string(ustr);
    recenter_text();
}

void SelectionEntry::set_string(UString && ustr) {
    m_display_text.set_string(std::move(ustr));
    recenter_text();
}

void SelectionEntry::set_location(float x, float y) {
    m_background.set_position(x + padding(), y + padding());
    recenter_text();
}

VectorF SelectionEntry::location() const {
    return m_background.position() - padding()*VectorF(1.f, 1.f);
}

float SelectionEntry::width() const {
    return m_background.width() + padding()*2.f;
}

float SelectionEntry::height() const {
    return m_background.height() + padding()*2.f;
}

void SelectionEntry::set_size(float width, float height) {
    if (width == 0.f || height == 0.f) return;
    m_background.set_size(width - padding()*2.f, height - padding()*2.f);
    m_display_text.set_limiting_dimensions(width, height);
    recenter_text();
}

const UString & SelectionEntry::string() const
    { return m_display_text.string(); }

void SelectionEntry::set_style(const StyleMap & styles) {

    using SelMenu = SelectionMenu;
    m_display_text.assign_font(styles, styles::k_global_font);
    if (auto * color = styles::find<sf::Color>(styles, TextArea::k_text_color)) {
        m_display_text.set_color(*color);
    } else {
        m_display_text.set_color(sf::Color::White);
    }
    styles::set_if_numeric_found(styles, TextArea::k_text_size, m_display_text);
    styles::set_if_found(styles, SelMenu::k_max_highlight, m_max_highlight);
    styles::set_if_found(styles, SelMenu::k_regular_highlight, m_reg_highlight);
    styles::set_if_found(styles, SelMenu::k_no_highlight, m_no_highlight);
    m_padding = 2.f;
#   if 0
    if (auto * color = styles::find<sf::Color>(styles, TextArea::k_text_color)) {
        m_display_text.set_color(*color);
    }
    if (auto * size = styles::find<float>(styles, TextArea::k_text_size)) {
        m_display_text.set_character_size(int(*size));
    }
    if (auto * color = styles::find<sf::Color>(styles, SelMenu::k_max_highlight)) {
        m_max_highlight = *color;
    }
    if (auto * color = styles::find<sf::Color>(styles, SelMenu::k_regular_highlight)) {
        m_reg_highlight = *color;
    }
    if (auto * color = styles::find<sf::Color>(styles, SelMenu::k_no_highlight)) {
        m_no_highlight = *color;
    }
#   endif
    m_background.set_color(m_no_highlight);
}

float SelectionEntry::content_width() const
    { return m_display_text.width() + padding()*2.f; }

float SelectionEntry::content_height() const
    { return m_display_text.height() + padding()*2.f; }

/* private */ void SelectionEntry::process_event(const sf::Event & event) {
    if (!m_parent) {
        throw std::runtime_error("SelectionEntry::process_event: cannot process events without a parent menu");
    }
    sf::FloatRect rect(m_background.position(), VectorF(m_background.width(), m_background.height()));
    switch (event.type) {
    case sf::Event::MouseMoved: {
        bool mouse_is_over = rect.contains(float(event.mouseMove.x), float(event.mouseMove.y));
        if (mouse_is_over != m_mouse_is_over) {
            m_mouse_is_over = mouse_is_over;
            update_highlight();            
        }
        }
        break;
    case sf::Event::MouseButtonReleased:
        if (rect.contains(float(event.mouseButton.x), float(event.mouseButton.y))) {
            m_parent->activate(m_menu_idx);
            request_focus();
        }    
    default: break;
    }
}

/* private */ void SelectionEntry::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    target.draw(m_background  , states);
    target.draw(m_display_text, states);
}

/* private */ void SelectionEntry::process_focus_event(const sf::Event & event) {
    if (!m_parent) {
        throw std::runtime_error("SelectionEntry::process_focus_event: cannot process events without a parent menu");
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Return) {
            m_parent->activate(m_menu_idx);
        }
    }
}

/* private */ void SelectionEntry::notify_focus_gained()
    { update_highlight(); }

/* private */ void SelectionEntry::notify_focus_lost() {
    update_highlight();
    if (m_parent)
        m_parent->deactivate(m_menu_idx);
}

/* private */ void SelectionEntry::update_highlight() {
    if (has_focus() && m_mouse_is_over) {
        m_background.set_color(m_max_highlight);
    } else if (has_focus() || m_mouse_is_over) {
        m_background.set_color(m_reg_highlight);
    } else {
        m_background.set_color(m_no_highlight);
    }
}

/* private */ void SelectionEntry::recenter_text() {
    m_display_text.set_location(
        m_background.x() + (m_background.width () - m_display_text.width ())*0.5f,
        m_background.y() + (m_background.height() - m_display_text.height())*0.5f);
}

/* private */ float SelectionEntry::padding() const noexcept
    { return std::max(0.f, m_padding); }

// ----------------------------------------------------------------------------

/* static */ constexpr const char * const SelectionMenu::k_max_highlight    ;
/* static */ constexpr const char * const SelectionMenu::k_regular_highlight;
/* static */ constexpr const char * const SelectionMenu::k_no_highlight     ;

/* private static */ constexpr const std::size_t SelectionMenu::k_uninit;

void SelectionMenu::add_options(std::vector<UString> && options) {
    if (!m_entries.empty()) {
        if (m_entries.size() != options.size()) {
            throw std::invalid_argument(
                "SelectionMenu::add_options: due to limitations of this "
                "library, this selection menu cannot change the number of "
                "entries it has");
        }
    }
    m_entries.reserve(options.size());
    for (auto opt : options) {
        m_entries.emplace_back();
        SelectionEntry & entry = m_entries.back();
        entry.assign_parent(*this, m_entries.size() - 1);
        entry.set_string(std::move(opt));
        entry.set_size(width(), height() / float(options.size()));
    }
    options.clear();
}

void SelectionMenu::set_size(float width_, float height_) {
    m_bounds.width = width_;
    m_bounds.height = height_;
    for (auto & entry : m_entries) {
        entry.set_size(width_, height_ / float(m_entries.size()));
    }
}

/* static */ void SelectionMenu::default_response_function
    (std::size_t, const UString &) {}

/* private */ void SelectionMenu::process_event(const sf::Event & event) {
    for (auto & entry : m_entries) {
        entry.process_event(event);
    }
}

/* private */ void SelectionMenu::set_location(float x, float y) {
    m_bounds.left = x;
    m_bounds.top  = y;
    for (auto & entry : m_entries) {
        entry.set_location(x, y);
        y += m_bounds.height / float(m_entries.size());
    }
}

/* private */ VectorF SelectionMenu::location() const {
    return VectorF(m_bounds.left, m_bounds.top);
}

/* private */ float SelectionMenu::width() const {
    return m_bounds.width;
}

/* private */ float SelectionMenu::height() const {
    return m_bounds.height;
}

/* private */ void SelectionMenu::set_style(const StyleMap & smap) {
    for (auto & wid : m_entries) {
        wid.set_style(smap);
    }
}

/* private */ void SelectionMenu::iterate_children_(ChildWidgetIterator & itr) {
    for (auto & wid : m_entries) itr.on_child(wid);
}

/* private */ void SelectionMenu::iterate_const_children_(ChildWidgetIterator & itr) const {
    for (const auto & wid : m_entries) itr.on_child(wid);
}

/* private */ void SelectionMenu::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    target.draw(m_selected);
    for (const auto & wid : m_entries) {
        target.draw(wid, states);
    }
}

/* private */ void SelectionMenu::activate(std::size_t index) {
    m_resp_func(index, m_entries[index].string());
    auto entry_height = m_bounds.height / float(m_entries.size());
    m_selected.set_position(m_bounds.left, m_bounds.top + entry_height*float(index));
    m_selected.set_size(m_bounds.width, entry_height);
    m_last_selected = index;
}

/* private */ void SelectionMenu::deactivate(std::size_t index) {
    if (m_last_selected == index) {
        m_selected = DrawRectangle();
    }
}

/* private */ void SelectionMenu::issue_auto_resize() {
    float entry_width  = 0.f;
    float entry_height = 0.f;
    for (const auto & wid : m_entries) {
        entry_width  = std::max(entry_width , wid.content_width ());
        entry_height = std::max(entry_height, wid.content_height());
    }
    for (auto & wid : m_entries) {
        wid.set_size(entry_width, entry_height);
    }
    m_bounds.width  = entry_width;
    m_bounds.height = entry_height*float(m_entries.size());
}

namespace detail {

void add_selection_menu_default_styles(StyleMap & smap) {
    smap[SelectionMenu::k_no_highlight] = StylesField(k_no_highlight);
    smap[SelectionMenu::k_regular_highlight] = StylesField(k_regular_highlight);
    smap[SelectionMenu::k_max_highlight] = StylesField(k_default_max_highlight);
}

} // end of detail namespace

} // end of ksg namespace
#endif
