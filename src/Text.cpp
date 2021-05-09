#include <asgl/Text.hpp>
#include <asgl/Widget.hpp>

namespace {

using TextBase = asgl::TextBase;
using UString  = TextBase::UString;
using ItemKey  = asgl::ItemKey;

/* anonymous */ class NullTextN final : public TextBase {
public:
    static NullTextN & instance() {
        static NullTextN inst;
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

    const sf::IntRect & viewport() const override {
        static const sf::IntRect inst;
        return inst;
    }

    UString give_string_() override { return UString(); }

    int limiting_line() const override { return 0; }

private:
    using RtError = std::runtime_error;

    static RtError make_cannot_call_error(const char * caller) noexcept {
        return RtError(std::string("NullText::") + caller
                       + ": cannot call this method on the null instance.");
    }

    NullTextN() {}
};

// ----------------------------------------------------------------------------

/* anonymous */ class BasicTextN final : public TextBase {
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
        { return make_clone<BasicTextN>(*this); }

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
    sf::IntRect m_viewport;
    UString m_string;
    int m_limiting_line = 0;
};

} // end of <anonymous> namespace

namespace asgl {

void TextBase::ProxyDeleter::operator () (TextBase * ptr) const {
    if (ptr == &NullTextN::instance()) return;
    delete ptr;
}

/* static */ TextBase::ProxyPointer TextBase::Helpers::make_default_instance()
    { return NullTextN::instance().clone(); }

/* static */ TextBase::ProxyPointer TextBase::Helpers::make_basic_instance()
    { return ProxyPointer(new BasicTextN()); }

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
    auto right = port.left + port.width;
    auto bottom = port.top + port.height;
    bool valid_port = port.left >= 0 && port.left <= full_width ()
                   && port.top  >= 0 && port.top  <= full_height()
                   && right     >= 0 && right     <= full_width ()
                   && bottom    >= 0 && bottom    <= full_height();
    using InvArg = std::invalid_argument;
    if (!valid_port) {
        throw InvArg("TextN::set_viewport: invalid viewport supplied, must "
                     "fit in the (inclusive) text boundry, or be the special "
                     "'reset' rectangle.");
    }
    set_viewport_(port);
}

void TextBase::reset_viewport() {
    set_viewport(sf::IntRect(0, 0, full_width(), full_height()));
}

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
