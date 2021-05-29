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

#include <asgl/Frame.hpp>

namespace asgl {

namespace detail {

class TakeMaxSizeFrame : public BareFrame {
public:
    template <typename Iter>
    using EnableTakeMaxIter = std::enable_if_t<
        true || std::is_base_of_v<TakeMaxSizeFrame &, decltype(**Iter())>,
    Iter>;

    template <typename Iter>
    static void take_max_size_between(EnableTakeMaxIter<Iter> beg, Iter end);

protected:
    TakeMaxSizeFrame() {}

private:
    class TakeMaxSizeDecoration final : public BlankDecorationBase {
    public:
        void set_size_minimum(Size sz) { m_size_min = sz; }

    private:
        void on_inform_is_child() override {}

        Size request_size(int w_, int h_) override;

        Size m_size_min;
    };

    FrameDecoration & decoration() final { return m_deco; }

    const FrameDecoration & decoration() const final { return m_deco; }

    TakeMaxSizeDecoration m_deco;
};

} // end of detail namespace -> into ::asgl

/** BookFrame is Widget which displays one frame at a time.
 *
 *  When page pointers are set, this widget takes the size of the maximum
 *  sized page frame. While it is not a "true Frame" it maybe used very similar
 *  to one. Widgets will need to be added by their respective parent frames.
 *
 *  This class is meant to be inherited by client code.
 */
class BookFrame : public Widget, public WidgetFlagsReceiver {
public:

    /** Pages are frames which should inherit from this class.
     *
     *  These are frames with no decoration.
     */
    class Page : public detail::TakeMaxSizeFrame {
    protected:
        Page() {}
    };
    using PageContainer = std::vector<Page *>;

    /** Sets the pages maintained by this frame.
     *  @note these pointers are not owned by this widget at least on this
     *        class level.
     */
    void set_pages(PageContainer &&);

    /** Sets the pages maintained by this frame.
     *  @note these pointers are not owned by this widget at least on this
     *        class level.
     */
    void set_pages(const PageContainer &);

    /** @returns the total number of pages */
    int page_count() const { return int(m_pages.size()); }

    /** @returns the current page number */
    int current_page_number() const { return m_current_page - m_pages.begin(); }

    /** Flips the book to the given page number.
     *  @throws if the given page number is either below zero, or greater than
     *          or equal to the page count.
     */
    void flip_to_page(int);

    void advance_page() { flip_to_page(current_page_number() + 1); }

    void regress_page() { flip_to_page(current_page_number() - 1); }

    /** All events are sent immediately to the current page.
     *
     *  If there are no pages, then this call does nothing.
     */
    void process_event(const Event &) override;

    const Page & current_page() const;

protected:
    BookFrame() {}

    void update_size() final;

    void set_location_(int x_, int y_) final;

    void draw(WidgetRenderer & target) const final;

    Size size() const final;

    Vector location() const final { return m_location; }

    void receive_whole_family_upate_needed() final;

    void receive_individual_update_needed(Widget *) final;

private:
    void iterate_children_(const ChildWidgetIterator &) final;

    void iterate_children_const_(const ChildConstWidgetIterator &) const final;

    using PageIter = PageContainer::iterator;
    PageContainer m_pages;
    PageIter      m_current_page;
    Vector        m_location;
};

// ----------------------------------------------------------------------------

namespace detail {

template <typename Iter>
/* static */ void TakeMaxSizeFrame::take_max_size_between(EnableTakeMaxIter<Iter> beg, Iter end) {
    using namespace cul::exceptions_abbr;
    Size max_size_;
    for (auto itr = beg; itr != end; ++itr) {
        TakeMaxSizeFrame & frame = **itr;
        Widget & as_widget = frame;
        frame.m_deco.set_size_minimum(Size());
        as_widget.update_size();
        max_size_ = max_size(max_size_, frame.size());
    }

    for (auto itr = beg; itr != end; ++itr) {
        TakeMaxSizeFrame & frame = **itr;
        Widget & as_widget = frame;
        frame.m_deco.set_size_minimum(max_size_);
        as_widget.update_size();
        if (frame.size() == max_size_) continue;
        throw RtError("TakeMaxSizeFrame::take_max_size_between: a frame is "
                      "changing size depending on number of calls to "
                      "'update_size'. This is a needed guarantee for the "
                      "BookFrame to work properly.");
    }
}

} // end of detail namespace -> into ::asgl

} // end of asgl namespace
