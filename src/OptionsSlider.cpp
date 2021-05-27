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

#include <asgl/OptionsSlider.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/Frame.hpp>

#include <cassert>

namespace asgl {

OptionsSlider::OptionsSlider() {
    using Dir = ArrowButton::Direction;
    m_left_arrow .set_direction(Dir::k_left );
    m_right_arrow.set_direction(Dir::k_right);
    set_arrow_events();
}

OptionsSlider::OptionsSlider(const OptionsSlider &)
    { set_arrow_events(); }

OptionsSlider::~OptionsSlider() {}

OptionsSlider & OptionsSlider::operator = (const OptionsSlider & rhs) {
    if (this != &rhs) {
        OptionsSlider temp(rhs);
        swap(temp);
    }
    return *this;
}

OptionsSlider & OptionsSlider::operator = (OptionsSlider && rhs) {
    if (this != &rhs) swap(rhs);
    return *this;
}

void OptionsSlider::process_event(const Event & evnt) {
    m_left_arrow .process_event(evnt);
    m_right_arrow.process_event(evnt);
}

Vector OptionsSlider::location() const
    { return m_left_arrow.location(); }

Size OptionsSlider::size() const {
    return Size(m_inner_bounds.width + m_inner_bounds.height*2
               ,m_inner_bounds.height);
}

void OptionsSlider::stylize(const StyleMap & stylemap) {
    m_left_arrow .stylize(stylemap);
    m_right_arrow.stylize(stylemap);
    TextArea::set_required_text_fields(
        m_text, stylemap.find(styles::k_global_font),
        stylemap.find(asgl::to_key(frame_styles::k_widget_text_style)),
        "OptionsSlider::stylize");

    using std::make_tuple;
    Helpers::handle_required_fields("OptionsSlider::stylize", {
        make_tuple(&m_front, "front style",
                   stylemap.find(m_front_style, to_key(k_front_style))),
        make_tuple(&m_back, "back style",
                   stylemap.find(m_back_style, to_key(k_back_style))),
    });
    m_padding = Helpers::verify_padding(
        stylemap.find(styles::k_global_padding), "OptionsSlider::stylize");
}

void OptionsSlider::set_options(const std::vector<UString> & options) {
    m_options = options;
    flag_needs_whole_family_geometry_update();
}

void OptionsSlider::set_options(std::vector<UString> && options) {
    m_options = std::move(options);
    flag_needs_whole_family_geometry_update();
}

void OptionsSlider::select_option(std::size_t index) {
    if (index == m_selected_index) return;    
    m_selected_index = index;
    update_selections();
    flag_needs_individual_geometry_update();
}

std::size_t OptionsSlider::selected_option_index() const
    { return m_selected_index; }

const UString & OptionsSlider::selected_option() const
    { return m_text.string(); }

std::size_t OptionsSlider::options_count() const
    { return m_options.size(); }

void OptionsSlider::set_option_change_event(BlankFunctor && func) {
    m_press_func = std::move(func);
}

void OptionsSlider::swap(OptionsSlider & rhs) {
    using std::swap;
    swap(m_left_arrow , rhs.m_left_arrow );
    swap(m_right_arrow, rhs.m_right_arrow);

    swap(m_padding, rhs.m_padding);
    swap(m_back   , rhs.m_back   );
    swap(m_front  , rhs.m_front  );

    swap(m_back_style   , rhs.m_back_style   );
    swap(m_front_style  , rhs.m_front_style  );
    swap(m_padding_style, rhs.m_padding_style);

    swap(m_inner_bounds, rhs.m_inner_bounds);

    swap(m_text          , rhs.m_text          );
    swap(m_options       , rhs.m_options       );
    swap(m_selected_index, rhs.m_selected_index);
    swap(m_press_func    , rhs.m_press_func    );
    swap(m_wrap_enabled  , rhs.m_wrap_enabled  );
}

void OptionsSlider::set_wrap_enabled(bool b) {
    if (m_wrap_enabled == b) return;
    m_wrap_enabled = b;
    update_selections();
}

void OptionsSlider::draw(WidgetRenderer & target) const {
    draw_to(target, m_inner_bounds, m_back);

    auto front = m_inner_bounds;
    front.top    += padding();
    front.height -= padding()*2;
    draw_to(target, front, m_front);

    m_left_arrow .draw(target);
    m_right_arrow.draw(target);
    m_text.draw_to(target);
}

/* private */ void OptionsSlider::iterate_children_
    (const ChildWidgetIterator & itr)
{
    itr(m_left_arrow );
    itr(m_right_arrow);
}

/* private */ void OptionsSlider::iterate_children_const_
    (const ChildConstWidgetIterator & itr) const
{
    itr(m_left_arrow );
    itr(m_right_arrow);
}

/* private */ void OptionsSlider::update_size() {
    int width_ = 0, height_ = 0;
    for (const auto & str : m_options) {
        auto gv = m_text.measure_text(str.begin(), str.end());
        width_  = std::max(width_ , gv.width );
        height_ = std::max(height_, gv.height);
    }

    set_size_of(m_inner_bounds, width_ + padding()*2, height_ + padding()*2);
    assert(m_inner_bounds.width  >= 0);
    assert(m_inner_bounds.height >= 0);
}

/* private */ void OptionsSlider::set_location_(int x, int y) {
    m_left_arrow.set_location(x, y);

    // update other internals
    m_left_arrow .set_size(m_inner_bounds.height, m_inner_bounds.height);
    m_right_arrow.set_size(m_inner_bounds.height, m_inner_bounds.height);

    // inner part
    int left_arrow_right = m_left_arrow.location().x + m_left_arrow.width();
    set_top_left_of(m_inner_bounds, left_arrow_right, m_left_arrow.location().y);

    // center text
    int center_offset = std::max(0, m_inner_bounds.width - m_text.width()) / 2;
    m_text.set_location(left_arrow_right + center_offset,
                        m_left_arrow.location().y + padding());

    // reposition right arrow
    m_right_arrow.set_location(
        left_arrow_right + m_inner_bounds.width,
        m_left_arrow.location().y);

    update_selections();
}

/* private */ void OptionsSlider::set_arrow_events() {
    m_left_arrow.set_press_event([this]() {
        if (m_options.empty() || m_selected_index == 0) return;
        --m_selected_index;
        update_selections();
        m_press_func();

    });
    m_right_arrow.set_press_event([this]() {
        auto last_idx = m_options.size() - 1;
        if (m_options.empty() || m_selected_index == last_idx) return;
        ++m_selected_index;
        update_selections();
        m_press_func();
    });
}

/* private */ void OptionsSlider::update_selections() {
    using Dir = ArrowButton::Direction;
    auto idx = m_selected_index;
    m_text.set_string( m_options.at(idx) );
    if (!m_wrap_enabled) {
        const auto last_idx = m_options.size() - 1;
        m_left_arrow .set_direction(idx == 0        ? Dir::k_none : Dir::k_left );
        m_right_arrow.set_direction(idx == last_idx ? Dir::k_none : Dir::k_right);
    }
}

/* private */ int OptionsSlider::padding() const
    { return std::max(0, m_padding); }

} // end of asgl namespace
