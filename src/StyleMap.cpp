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

#include <stdexcept>

#include <cassert>

namespace {

using Error  = std::runtime_error;
using InvArg = std::invalid_argument;

} // end of <anonymous> namespace

namespace asgl {

/* static */ StyleMap StyleMap::construct_new_map() {
    StyleMap map;
    map.m_map_ptr = std::make_shared<MapImplType>();
    return map;
}

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

} // end of styles namespace

} // end of asgl namespace
