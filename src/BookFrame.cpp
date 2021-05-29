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

#include <asgl/BookFrame.hpp>

namespace asgl {

namespace detail {

/* private */ Size TakeMaxSizeFrame::TakeMaxSizeDecoration::request_size
    (int w_, int h_)
{
    Size accepted_size { w_, h_ };
    accepted_size = max_size(m_size_min, accepted_size);
    set_size(accepted_size);
    return accepted_size;
}

} // end of detail namespace -> into ::asgl

void BookFrame::set_pages(PageContainer && pages) {
    m_pages = std::move(pages);
    m_current_page = m_pages.begin();
    for (auto & page : m_pages) {
        page->assign_flags_receiver(this);
    }
    flag_needs_whole_family_geometry_update();
}

void BookFrame::set_pages(const PageContainer & cont) {
    auto temp = cont;
    set_pages(std::move(temp));
}

void BookFrame::flip_to_page(int i) {
    using namespace cul::exceptions_abbr;
    if (i < 0 || i >= int(m_pages.size())) {
        throw InvArg("BookFrame::flip_to_page: given index must be in [0 "
                     "page_count), which is " + std::to_string(page_count())
                     + ".");
    }
    m_current_page = m_pages.begin() + i;
    flag_needs_individual_geometry_update();
}

void BookFrame::process_event(const Event & event) {
    if (m_pages.empty()) return;
    (**m_current_page).process_event(event);
}

const BookFrame::Page & BookFrame::current_page() const {
    using namespace cul::exceptions_abbr;
    if (!m_pages.empty()) return (**m_current_page);
    throw RtError("BookFrame::current_page: cannot retrieve any page as "
                  "there are none.");
}

/* protected */ void BookFrame::update_size()
    { Page::take_max_size_between(m_pages.begin(), m_pages.end()); }

/* protected */ void BookFrame::set_location_(int x_, int y_) {
    m_location = Vector(x_, y_);
    if (m_pages.empty()) return;
    (**m_current_page).set_location(x_, y_);
}

/* protected */ void BookFrame::draw(WidgetRenderer & target) const {
    if (m_pages.empty()) return;
    (**m_current_page).draw(target);
}

/* protected */ Size BookFrame::size() const {
    if (m_pages.empty()) return Size();
    return m_pages.front()->size();
}

/* protected */ void BookFrame::receive_whole_family_upate_needed()
    { flag_needs_whole_family_geometry_update(); }

/* protected */ void BookFrame::receive_individual_update_needed(Widget *)
    { flag_needs_individual_geometry_update(); }

/* private */ void BookFrame::iterate_children_(const ChildWidgetIterator & itr) {
    if (m_pages.empty()) return;
    itr(**m_current_page);
}

/* private */ void BookFrame::iterate_children_const_(const ChildConstWidgetIterator & itr) const {
    if (m_pages.empty()) return;
    itr(**m_current_page);
}

} // end of asgl namespace
