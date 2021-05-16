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

#include <asgl/Text.hpp>
#include <asgl/Widget.hpp>

namespace {

using TextBase = asgl::TextBase;
using UString  = TextBase::UString;
using ItemKey  = asgl::ItemKey;

constexpr const int k_int_max = std::numeric_limits<int>::max();

/* anonymous */ class NullText final : public TextBase {
public:
    static NullText & instance() {
        static NullText inst;
        return inst;
    }

    const UString & string() const override {
        static const UString inst;
        return inst;
    }

    void set_location(int, int) override
        { throw make_cannot_call_error("set_location"); }

    VectorI location() const override { return VectorI(); }
    int width() const override { return 0; }
    int height() const override { return 0; }
    int full_width() const override { return 0; }
    int full_height() const override { return 0; }

    void set_limiting_line(int) override
        { throw make_cannot_call_error("set_limiting_line"); }

    void stylize(ItemKey) override { throw make_cannot_call_error("stylize"); }

    TextSize measure_text(UStringConstIter, UStringConstIter) const override
        { return TextSize(); }

    ProxyPointer clone() const override
        { return ProxyPointer(&instance()); }

    void set_viewport_(const sf::IntRect & rect) override {
        if (rect == sf::IntRect()) return;
        throw make_cannot_call_error("set_viewport_");
    }

    void swap_string(UString &) override
        { throw make_cannot_call_error("swap_string"); }

    const sf::IntRect & viewport() const override
        { return TextBase::k_default_viewport; }

    UString give_string_() override { return UString(); }

    int limiting_line() const override
        { return TextBase::k_default_limiting_line; }

private:
    using RtError = std::runtime_error;

    static RtError make_cannot_call_error(const char * caller) noexcept {
        return RtError(std::string("NullText::") + caller
                       + ": cannot call this method on the null instance.");
    }

    NullText() {}
};

// ----------------------------------------------------------------------------

/* anonymous */ class BasicText final : public TextBase {
public:
    const UString & string() const override
        { return m_string; }

    void set_location(int x, int y) override
        { m_location = VectorI(x, y); }

    VectorI location() const override { return m_location; }

    int width() const override { return 0; }

    int height() const override { return 0; }

    int full_width() const override { return 0; }

    int full_height() const override { return 0; }

    void set_limiting_line(int i) override
        { m_limiting_line = i; }

    void stylize(ItemKey) override {}

    TextSize measure_text(UStringConstIter, UStringConstIter) const override
        { return TextSize(); }

    ProxyPointer clone() const override
        { return make_clone<BasicText>(*this); }

    void set_viewport_(const sf::IntRect & port) override
        { m_viewport = port; }

    void swap_string(UString & str) override
        { m_string.swap(str); }

    const sf::IntRect & viewport() const override
        { return m_viewport; }

    UString give_string_() override { return std::move(m_string); }

    int limiting_line() const override { return m_limiting_line; }

private:
    VectorI m_location;
    sf::IntRect m_viewport = TextBase::k_default_viewport;
    UString m_string;
    int m_limiting_line = TextBase::k_default_limiting_line;
};

} // end of <anonymous> namespace

namespace asgl {

void TextBase::ProxyDeleter::operator () (TextBase * ptr) const {
    if (ptr == &NullText::instance()) return;
    delete ptr;
}

/* static */ TextBase::ProxyPointer TextBase::Helpers::make_default_instance()
    { return NullText::instance().clone(); }

/* static */ TextBase::ProxyPointer TextBase::Helpers::make_basic_instance()
    { return ProxyPointer(new BasicText()); }

void TextBase::set_string(const UString & str) {
    UString temp;
    swap_string(temp);
    temp = str;
    swap_string(temp);
}

void TextBase::set_string(UString && str) {
    swap_string(str);
}

UString TextBase::give_cleared_string() {
    UString temp = give_string();
    temp.clear();
    return temp;
}

UString TextBase::give_string()
    { return give_string_(); }

void TextBase::set_viewport(const sf::IntRect & port) {
    bool valid_port =    port.left >= 0 && port.left <= full_width ()
                      && port.top  >= 0 && port.top  <= full_height();
    if (port.width != k_default_viewport.width) {
        auto right = port.left + port.width;
        valid_port = valid_port && right >= 0;
    }
    if (port.height != k_default_viewport.height) {
        auto bottom = port.top + port.height;
        valid_port = valid_port && bottom >= 0;
    }

    using InvArg = std::invalid_argument;
    if (!valid_port) {
        throw InvArg("TextN::set_viewport: invalid viewport supplied, must "
                     "fit in the (inclusive) text boundry, or be the special "
                     "'reset' rectangle.");
    }
    set_viewport_(port);
}

void TextBase::reset_viewport() {
    set_viewport(k_default_viewport);
}

/* static */ const sf::IntRect TextBase::k_default_viewport =
    sf::IntRect(0, 0, k_int_max, k_int_max);

/* static */ const int TextBase::k_default_limiting_line = k_int_max;

// ----------------------------------------------------------------------------

Text::Text(const Text & rhs):
    m_proxy( rhs.m_proxy->clone() )
{}

Text & Text::operator = (const Text & rhs) {
    if (this != &rhs) {
        m_proxy = rhs.m_proxy->clone();
    }
    return *this;
}

void Text::set_string(const UString & str) {
    check_to_transform_to_basic();
    m_proxy->set_string(str);
}

void Text::set_string(UString && str) {
    check_to_transform_to_basic();
    m_proxy->set_string(std::move(str));
}

Text::UString Text::give_cleared_string()
    { return m_proxy->give_cleared_string(); }

Text::UString Text::give_string()
    { return m_proxy->give_cleared_string(); }

const UString & Text::string() const
    { return m_proxy->string(); }

void Text::set_location(const VectorI & r)
    { set_location(r.x, r.y); }

void Text::set_location(int x, int y) {
    check_to_transform_to_basic();
    m_proxy->set_location(x, y);
}

Text::VectorI Text::location() const
    { return m_proxy->location(); }

int Text::width() const
    { return m_proxy->width(); }

int Text::height() const
    { return m_proxy->height(); }

int Text::full_width() const
    { return m_proxy->full_width(); }

int Text::full_height() const
    { return m_proxy->full_height(); }

void Text::set_limiting_line(int x_limit) {
    check_to_transform_to_basic();
    m_proxy->set_limiting_line(x_limit);
}

void Text::stylize(ItemKey item) {
    // ???
    // I think I'll allow throwing here
    // as this call should be handled by stylize
    //
    // I may consider simply ignoring it
    m_proxy->stylize(item);
}

Text::TextSize Text::measure_text
    (UStringConstIter beg, UStringConstIter end) const
{ return m_proxy->measure_text(beg, end); }

void Text::set_viewport(const sf::IntRect & viewport) {
    check_to_transform_to_basic();
    m_proxy->set_viewport(viewport);
}

void Text::reset_viewport() {
    // reset does not need a basic transformation check, "stateless" text can
    // handle it
    m_proxy->reset_viewport();
}

const sf::IntRect & Text::viewport() const
    { return m_proxy->viewport(); }

void Text::set_font(const Font & font) {
    m_proxy = font.fit_pointer_to_adaptor(std::move(m_proxy));
}

void Text::draw_to(WidgetRenderer & target) const
    { target.render_text(*m_proxy); }

/* private */ void Text::check_to_transform_to_basic() {
    if (m_proxy != Helpers::make_default_instance()) return;
    m_proxy = Helpers::make_basic_instance();
}

// ----------------------------------------------------------------------------

} // end of asgl namespace
