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

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <memory>
#include <string>

namespace asgl {

class Text;
class WidgetRenderer;
class ItemKey;

// ----------------------------------------------------------------------------
//             this is *sort of* beyond what the client should see

class TextBase {
public:
    using UString          = std::u32string;
    using UChar            = UString::value_type;
    using UStringConstIter = UString::const_iterator;
    using VectorI          = sf::Vector2i;

    struct ProxyDeleter {
        void operator () (TextBase *) const;
    };
    using ProxyPointer = std::unique_ptr<TextBase, ProxyDeleter>;

    class Helpers {
        friend class TextBase;
        friend class Text;

        static ProxyPointer make_default_instance();
        static ProxyPointer make_basic_instance();
    };

    struct TextSize { int width = 0, height = 0; };

    virtual ~TextBase() {}

    void set_string(const UString & str);

    void set_string(UString && str);

    /** @returns a cleared string which in turn maybe reused to minimize
     *           reallocation.
     *  "give" puts Text in the nomitive, Text gives string
     *  "take" puts Text in the accusitive, Text has string taken
     */
    UString give_cleared_string();

    UString give_string();

    /** @returns a string representation of the text */
    virtual const UString & string() const = 0;

    /** Sets the pixel location of the text. */
    virtual void set_location(int x, int y) = 0;

    /** @returns the pixel location of the text */
    virtual VectorI location() const = 0;

    /** @returns the appearent width of the text (in pixels) */
    virtual int width() const = 0;

    /** @returns the appearent height of the text (in pixels) */
    virtual int height() const = 0;

    /** @returns the full width of the text, without viewport imposed
     *           limitations.
     */
    virtual int full_width() const = 0;

    /** @returns the full height of the text, without viewport imposed
     *           limitations.
     */
    virtual int full_height() const = 0;

    /** Sets a hard limit of how far text may go, for now this is the maximum
     *  width.
     *  @note presently intented as "limited width" though I'm not sure if I'd
     *        like to repurpose this later to something other than height.
     *  @param x_limit line position, no character will appear after or at this
     *                 line
     */
    virtual void set_limiting_line(int x_limit) = 0;

    /** Sets the style of the text, this is meant to include the size, color,
     *  and anything else.
     */
    virtual void stylize(ItemKey) = 0;

    virtual TextSize measure_text(UStringConstIter beg, UStringConstIter end) const = 0;

    virtual ProxyPointer clone() const = 0;

    virtual int limiting_line() const = 0;

    void set_viewport(const sf::IntRect &);

    void reset_viewport();

    virtual const sf::IntRect & viewport() const = 0;

    static const sf::IntRect k_default_viewport;
    static const int k_default_limiting_line;

protected:
    template <typename T>
    static std::unique_ptr<T, ProxyDeleter> make_clone(const T & obj) {
        static_assert (std::is_base_of_v<TextBase, T>, "Type T must be derived from Text.");
        return std::unique_ptr<T, ProxyDeleter>(new T (obj));
    }

    virtual void set_viewport_(const sf::IntRect &) = 0;

    virtual void swap_string(UString &) = 0;

    virtual UString give_string_() = 0;
};

// ----------------------------------------------------------------------------

class Font {
public:
    using TextSize         = TextBase::TextSize;
    using UString          = TextBase::UString;
    using UStringConstIter = TextBase::UStringConstIter;
    using TextPointer      = TextBase::ProxyPointer;

    virtual TextPointer fit_pointer_to_adaptor(TextPointer &&) const = 0;
    virtual TextSize measure_text(ItemKey, UStringConstIter beg, UStringConstIter end) const = 0;

protected:
    template <typename T>
    static T & check_and_transform_text(TextPointer &);
};

// ----------------------------------------------------------------------------

// this should be the client's interface
class Text final {
public:
    using UString          = TextBase::UString;
    using UStringConstIter = TextBase::UStringConstIter;
    using VectorI          = TextBase::VectorI;
    using ProxyPointer     = TextBase::ProxyPointer;
    using TextSize         = TextBase::TextSize;

    Text() {}
    Text(const Text &);
    Text(Text &&) = default;
    ~Text() = default;

    Text & operator = (const Text &);
    Text & operator = (Text &&) = default;

    void set_string(const UString & str);

    void set_string(UString && str);

    /** @returns a cleared string which in turn maybe reused to minimize
     *           reallocation.
     */
    UString give_cleared_string();

    UString give_string();

    const UString & string() const;

    void set_location(const VectorI &);

    void set_location(int x, int y);

    VectorI location() const;

    /** @returns the appearent width of the text (in pixels) */
    int width() const;

    /** @returns the appearent height of the text (in pixels) */
    int height() const;

    /** @returns the full width of the text, without viewport imposed
     *           limitations.
     */
    int full_width() const;

    /** @returns the full height of the text, without viewport imposed
     *           limitations.
     */
    int full_height() const;

    /** Sets a hard limit of how far text may go, for now this is the maximum
     *  width.
     *  @note presently intented as "limited width" though I'm not sure if I'd
     *        like to repurpose this later to something other than height.
     *  @param x_limit line position, no character will appear after or at this
     *                 line
     */
    void set_limiting_line(int x_limit);

    /** Sets the style of the text, this is meant to include the size, color,
     *  and anything else.
     */
    void stylize(ItemKey);

    TextSize measure_text(UStringConstIter beg, UStringConstIter end) const;

    void set_viewport(const sf::IntRect &);

    void reset_viewport();

    const sf::IntRect & viewport() const;

    void set_font(const Font &);

    void draw_to(WidgetRenderer &) const;

private:
    using Helpers = TextBase::Helpers;

    void check_to_transform_to_basic();

    ProxyPointer m_proxy = Helpers::make_default_instance();
};

// -------------------------- implementation detail ---------------------------

template <typename T>
/* static protected */ T & Font::check_and_transform_text(TextPointer & ptr) {
    static_assert(std::is_base_of_v<TextBase, T>, "T must be derived from Text.");
    using DerivedPointer = std::unique_ptr<T, TextBase::ProxyDeleter>;
    if (auto * rv = dynamic_cast<T *>(ptr.get())) {
        return *rv;
    }
    auto new_ptr = DerivedPointer( new T() );
    T & as_rv = *new_ptr;
    TextBase & as_base = *new_ptr;

    auto loc = ptr->location();
    as_base.set_string       ( ptr->give_string()   );
    as_base.set_viewport     ( ptr->viewport()      );
    as_base.set_limiting_line( ptr->limiting_line() );
    as_base.set_location     ( loc.x, loc.y );

    ptr = std::move(new_ptr);
    return as_rv;
}

} // end of asgl namespace
