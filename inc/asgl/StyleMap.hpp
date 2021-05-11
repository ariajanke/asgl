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

#include <common/MultiType.hpp>

#include <map>
#include <memory>
#include <vector>

namespace asgl {

namespace detail {

class IdObject {
public:
    std::size_t hash() const { return m_value; }

    bool operator < (const IdObject & rhs) const noexcept
        { return m_value < rhs.m_value; }

protected:
    IdObject() {}
    IdObject(const IdObject &) = default;
    explicit IdObject(std::size_t value): m_value(value) {}
    IdObject & operator = (const IdObject &) = default;

    bool is_same_as(const IdObject & rhs) const noexcept
        { return m_value == rhs.m_value; }

private:
    std::size_t m_value = 0;
};

struct IdObjectHasher {
    std::size_t operator() (const IdObject & obj) const noexcept
        { return obj.hash(); }
};

template <typename T, std::size_t kt_enum_size>
class KeysEnumBase {
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>,
        "Type T must be a integral and optionally an enumeration type.");
    using IntegerType = std::conditional_t<std::is_enum_v<T>, std::underlying_type_t<T>, T>;
    using OoRange = std::out_of_range;

protected:
    static std::size_t to_unique_hash(const T & val) {
        auto val_int = static_cast<IntegerType>(val);
        if (val_int >= 0 && val_int < kt_enum_size) {
            // we want this array for its unique addresses in the program's
            // static storage
            static const std::array<uint8_t, kt_enum_size> k_arr = {};
            //return static_cast<const void *>(k_arr.data() + val_int);
            return std::hash<const uint8_t *>()(k_arr.data() + val_int);
        }
        throw OoRange("to_key: integer value of the given argument is out of "
                      "range. Must be between [0 "
                      + std::to_string(kt_enum_size) + ").");
    }
};

} // end of detail namespace

namespace styles {

struct AutomaticSize {};

template <typename T, std::size_t kt_enum_size>
class StyleKeysEnum;

template <typename T, std::size_t kt_enum_size>
class ItemKeysEnum;

class ItemKeyCreator;

} // end of styles namespace

/** A StyleKey is a map key, which is then linked to an StyleField. Which may
 *  contain information on either how this UI element should either appear or
 *  how large it should be.
 *
 *  If a style key links to an "item key", this item key then maybe used to link
 *  to a color, (an image...
 */
class StyleKey final : public detail::IdObject {
public:
    StyleKey() {}
    StyleKey(const StyleKey &) = default;
    StyleKey & operator = (const StyleKey &) = default;

    bool operator == (const StyleKey & rhs) const noexcept
        { return is_same_as(rhs); }
    bool operator != (const StyleKey & rhs) const noexcept
        { return !is_same_as(rhs); }

private:
    template <typename T, std::size_t kt_enum_size>
    friend class styles::StyleKeysEnum;

    explicit StyleKey(std::size_t val): IdObject(val) {}
};

/** An item key, represents a key which is used to figure out how a specific UI
 *  element should be rendered by the renderer.
 */
class ItemKey final : public detail::IdObject {
public:
    ItemKey() {}
    ItemKey(const ItemKey &) = default;
    ItemKey & operator = (const ItemKey &) = default;

    bool operator == (const ItemKey & rhs) const noexcept
        { return is_same_as(rhs); }
    bool operator != (const ItemKey & rhs) const noexcept
        { return !is_same_as(rhs); }

    static std::shared_ptr<ItemKey> make_unique_key() {
        auto item_ptr = std::make_shared<ItemKey>();
        *item_ptr = ItemKey( std::hash<const ItemKey *>()(item_ptr.get()) );
        return item_ptr;
    }

private:
    template <typename T, std::size_t kt_enum_size>
    friend class styles::ItemKeysEnum;

    friend class styles::ItemKeyCreator;

    explicit ItemKey(std::size_t val): IdObject(val) {}
};

class Font;

// ItemKey and StyleKey should not be considered to be the same type
using StyleField = MultiType<
    int,
    std::weak_ptr<const Font>,
    styles::AutomaticSize
    // not ready for this
    , ItemKey
>;

/** Styles map offers a way to "style" UI elements with certain values.
 *
 *  Widgets are "stylized" by this map. That is this map links UI elements to
 *  information about how each element should appear for the renderer and how
 *  large each widget is.
 *
 *  @note
 *  All copies of a style map will share the same underlying map. If a copy is
 *  desired, the clone method will provide.
 */
class StyleMap {
public:
    static StyleMap construct_new_map();

    StyleMap() {}

    StyleMap & add(StyleKey, const StyleField &);

    /** Creates a copy of this map, which maybe modified without modifying the
     *  original.
     *  @note this class uses a shared_ptr for the underlying map
     */
    StyleMap clone() const;

    /** Finds and returns a field for the first key for which a field is found.
     *
     *
     *  @param first
     *  @param args  zero or more keys
     *  @returns
     *
     */
    template <typename ... Types>
    const StyleField * find(StyleKey first, Types &&... args) const {
        if (first == StyleKey()) return nullptr;
        if (auto * res = find_one(first)) return res;
        return find(std::forward<Types>(args)...);
    }

    bool has_same_map_pointer(const StyleMap &) const noexcept;

private:
    const StyleField * find_one(StyleKey) const;

    template <typename ... Types>
    const StyleField * find(Types &&...) const { return nullptr; }

    using MapImplType = std::map<StyleKey, StyleField>;
    using MapImplPtr  = std::shared_ptr<MapImplType>;
    MapImplPtr m_map_ptr;
};

namespace styles {

template <typename T, std::size_t kt_enum_size>
class StyleKeysEnum final : public detail::KeysEnumBase<T, kt_enum_size> {
    using Base = detail::KeysEnumBase<T, kt_enum_size>;
public:
    static StyleKey to_key(const T & val)
        { return StyleKey( Base::to_unique_hash(val) ); }
};

template <typename T, std::size_t kt_enum_size>
class ItemKeysEnum final : public detail::KeysEnumBase<T, kt_enum_size> {
    using Base = detail::KeysEnumBase<T, kt_enum_size>;
public:
    static ItemKey to_key(const T & val)
        { return ItemKey( Base::to_unique_hash(val) ); }
};

class ItemKeyCreator {
public:
    ItemKeyCreator();

    ItemKey make_key();

private:
    using ArrayForUniqueAddresses = std::array<uint8_t, 1024>;
    using UniqueAddressesVector = std::vector<std::unique_ptr<ArrayForUniqueAddresses>>;
    using Position = ArrayForUniqueAddresses::const_iterator;
    UniqueAddressesVector m_unique_addresses;
    Position m_pos;
};

extern const StyleKey k_null_key      ;
extern const StyleKey k_global_padding;
extern const StyleKey k_global_font   ;

constexpr const int k_uninit_size = -1;

} // end of styles namespace

} // end of namespace asgl
