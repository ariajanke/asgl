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

#include <asgl/StyleMap.hpp>
#include <asgl/TextArea.hpp>
#if 0
#include <ksg/StyleMap.hpp>
#include <ksg/Frame.hpp>
#include <ksg/Button.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/ProgressBar.hpp>
#include <ksg/SelectionMenu.hpp>
#endif
#include <stdexcept>

#include <cassert>

namespace {

using Error  = std::runtime_error;
using InvArg = std::invalid_argument;

#if 0
template <typename T>
void add_style(ksg::StyleMap & smap, const char * key, const T & val)
    { smap[key] = ksg::StylesField(val); }
#endif

} // end of <anonymous> namespace

namespace asgl {

/* static */ StyleMap StyleMap::construct_new_map() {
    StyleMap map;
    map.m_map_ptr = std::make_shared<MapImplType>();
    return map;
}
#if 0
/* static */ StyleMap StyleMap::construct_library_default() {
    using namespace styles;
    StyleMap map = construct_new_map();
    map[k_global_padding] = StyleField(2);

    // TextArea
    TextArea::DefaultStyles::ensure(map, TextArea::k_text_color) = StyleField(sf::Color::White);
    TextArea::DefaultStyles::ensure(map, TextArea::k_text_size ) = StyleField(Text::k_default_font_size);
    return map;
}
#endif
#if 0
StyleField & StyleMap::operator [] (StyleKey key) {
    if (key == styles::k_null_key) {
        throw std::invalid_argument("asgl::StyleMap::operator []: key may not be the null key");
    }
    return (*m_map_ptr)[key];
}
#endif

StyleMap & StyleMap::add(StyleKey key, const StyleField & obj) {
    if (!obj.is_valid()) {
        throw InvArg("asgl::StyleMap::add: StyleField is not initialized.");
    }
    if (!m_map_ptr->insert(std::make_pair(key, obj)).second) {
        throw InvArg("asgl::StyleMap::add: Key is already present in the map.");
    }
    return *this;
}

StyleMap StyleMap::clone() const {
    StyleMap map;
    map.m_map_ptr = std::make_shared<MapImplType>(*m_map_ptr);
    return map;
}

bool StyleMap::has_same_map_pointer(const StyleMap & rhs) const noexcept
    { return m_map_ptr == rhs.m_map_ptr; }

/* private */ const StyleField * StyleMap::find_one(StyleKey key) const {
    auto itr = m_map_ptr->find(key);
    if (itr == m_map_ptr->end()) return nullptr;
    return &(itr->second);

}

namespace styles {

enum { k_gpad_key, k_gfont_key, k_global_key_count };
using GlobalKeyInit = StyleKeysEnum<decltype(k_global_key_count), k_global_key_count>;

const StyleKey k_null_key       = StyleKey();
const StyleKey k_global_padding = GlobalKeyInit::to_key(k_gpad_key);
const StyleKey k_global_font    = GlobalKeyInit::to_key(k_gfont_key);

ItemKeyCreator::ItemKeyCreator() {

}

ItemKey ItemKeyCreator::make_key() {
    using std::get;
    auto push_new_array = [this] () {
        m_unique_addresses.emplace_back(std::make_unique<ArrayForUniqueAddresses>());
        m_pos = m_unique_addresses.back()->begin();
    };

    if (m_unique_addresses.empty()) {
        push_new_array();
    } else if (m_pos == m_unique_addresses.back()->end()) {
        push_new_array();
    }
    return ItemKey(std::hash<const uint8_t *>()(&(*m_pos++)));
}

StyleField load_font(const std::string & filename) {
    auto sptr = std::make_shared<sf::Font>();
    if (sptr->loadFromFile(filename)) {
        return StyleField(std::shared_ptr<const sf::Font>(sptr));
    } else {
        return StyleField();
    }
}

#if 0
DrawRectangle make_rect_with_unset_color() {
    DrawRectangle rect;
    rect.set_color(get_unset_value<sf::Color>());
    return rect;
}

StyleMap construct_system_styles() {
    using sf::Color;
    StyleMap smap;

    // Global styles
    add_style(smap, k_global_padding, Frame::k_default_padding);

    // Frame's default styles
    add_style(smap, Frame::k_background_color , Color(0x51, 0x51, 0x76));
    add_style(smap, Frame::k_title_bar_color  , Color(0x08, 0x08, 0x22));
    add_style(smap, Frame::k_title_color      , Color::White           );
    add_style(smap, Frame::k_title_size       , 20.f                   );
    add_style(smap, Frame::k_widget_body_color, Color(0x18, 0x18, 0x40));
UniqueAddressesArray
    // Button's default styles
    add_style(smap, Button::k_hover_back_color , Color(0x4B, 0x46, 0x15));
    add_style(smap, Button::k_hover_front_color, Color(0x77, 0x6A, 0x45));
    add_style(smap, Button::k_regular_back_color   , Color(0x4B, 0x46, 0x15));
    add_style(smap, Button::k_regular_front_color  , Color(0x30, 0x2C, 0x05));

    // Text Button's default styles
    add_style(smap, TextButton::k_text_color, Color::White);
    add_style(smap, TextButton::k_text_size , 20.f         );

    // Text Area's default styles
    add_style(smap, TextArea::k_text_color, Color::White);
    add_style(smap, TextArea::k_text_size , 18.f        );

    // Progress Bar's default styles
    add_style(smap, ProgressBar::k_inner_back_color , Color(0x40,    0,    0));
    add_style(smap, ProgressBar::k_inner_front_color, Color(0xA0, 0xA0, 0x00));
    add_style(smap, ProgressBar::k_outer_color      , Color(0x10, 0x10, 0x10));
    add_style(smap, ProgressBar::k_padding          , 2.f                    );

    detail::add_selection_menu_default_styles(smap);
    return smap;
}

StylesField load_font(const std::string & filename) {
    auto sptr = std::make_shared<sf::Font>();
    if (sptr->loadFromFile(filename)) {
        return StylesField(std::shared_ptr<const sf::Font>(sptr));
    } else {
        return StylesField();
    }
}

/* private static */ void SetIfNumericFoundPriv::set_char_size
    (Text & text, int sz)
{ text.set_character_size(sz); }
#endif
} // end of styles namespace

} // end of asgl namespace
