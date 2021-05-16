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

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#if 0
#include <ksg/Frame.hpp>

#include <ksg/TextButton.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/SelectionMenu.hpp>
#endif
#include <common/CurrentWorkingDirectory.hpp>

#include <cstring>
#include <cassert>

// file system stuff
#include <sys/stat.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

namespace {

class FileDisplayList;
using FileDisplayPtr = std::unique_ptr<FileDisplayList>;
using SharedStylePtr = std::shared_ptr<ksg::StyleMap>;
using UChar          = ksg::Text::UChar;
using UString        = ksg::Text::UString;

struct FileInfo {
    bool is_directory = false;
    std::string full_path;
};

template <typename WidgetType>
class ScrollWidget final : public ksg::Widget {
public:
    static constexpr const float k_default_scroll_bar_size = 20.f;
    using RenderTargetPtr = std::shared_ptr<sf::RenderTexture>;
    static_assert(std::is_base_of_v<ksg::Widget, WidgetType>, "");

    void process_event(const sf::Event &) override;

    void set_location(float x, float y) override;

    VectorF location() const override;

    float width() const override;

    float height() const override;

    void set_style(const ksg::StyleMap &) override;

    void set_window_size(float width_, float height_);

private:
    using HandledType = ksg::Widget;
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void issue_auto_resize() override;

    void iterate_children_(ksg::ChildWidgetIterator & itr) override { itr.on_child(m_widget); }
    void iterate_const_children_(ksg::ChildWidgetIterator & itr) const override { itr.on_child(m_widget); }

    DrawRectangle m_back;
    DrawRectangle m_front;
    WidgetType m_widget;
    RenderTargetPtr m_target;
};

template <typename WidgetType>
void ScrollWidget<WidgetType>::process_event(const sf::Event &) {

}

template <typename WidgetType>
void ScrollWidget<WidgetType>::set_location(float x, float y) {

}

template <typename WidgetType>
ksg::Widget::VectorF ScrollWidget<WidgetType>::location() const {

}

template <typename WidgetType>
float ScrollWidget<WidgetType>::width() const {

}

template <typename WidgetType>
float ScrollWidget<WidgetType>::height() const {

}

template <typename WidgetType>
void ScrollWidget<WidgetType>::set_style(const ksg::StyleMap &) {

}

template <typename WidgetType>
void ScrollWidget<WidgetType>::set_window_size(float width_, float height_) {

}

template <typename WidgetType>
/* private */ void ScrollWidget<WidgetType>::draw(sf::RenderTarget &, sf::RenderStates) const {

}

template <typename WidgetType>
/* private */ void ScrollWidget<WidgetType>::issue_auto_resize() {

}

using ScrollSelectionList = ScrollWidget<ksg::SelectionMenu>;

class FileDisplayList final : public ksg::Frame {
public:
    void setup();
    void setup(SharedStylePtr, const std::string & path);
    FileDisplayPtr get_new_list();
    static void assign_quit_float(bool * ptr)
        { s_quit_flag = ptr; }

private:
    FileDisplayPtr m_new_list;

    ksg::TextArea m_info_notice;
    ksg::TextButton m_exit_button;
    ksg::TextButton m_up_button;

    ksg::SelectionMenu m_file_list_display;
    std::vector<FileInfo> m_cwd_file_list;

    static bool * s_quit_flag;
};

} // end of <anonymous> namespace

int main() {
    bool done = false;
    FileDisplayList::assign_quit_float(&done);
    FileDisplayPtr file_frame = std::make_unique<FileDisplayList>();
    file_frame->setup();

    while (!done) {
        sf::RenderWindow window;
        window.setFramerateLimit(20u);
        auto vm = sf::VideoMode(unsigned(file_frame->width()), unsigned(file_frame->height()));
        assert(vm.height != 0u && vm.width != 0u);
        window.create(vm, get_current_working_directory());

        while (window.isOpen()) {
            {
            sf::Event event;
            while (window.pollEvent(event)) {
                file_frame->process_event(event);
                if (done) return 0;
                if (FileDisplayPtr gv = file_frame->get_new_list()) {
                    gv.swap(file_frame);

                    sf::Vector2f szv(file_frame->width(), file_frame->height());
                    window.setSize(sf::Vector2u(szv));
                    window.setView(sf::View(szv*0.5f, szv));
                }
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return 0;
                }
            }
            }
            window.clear();
            window.draw(*file_frame);
            window.display();
            sf::sleep(sf::microseconds(16667));
        }

    }
    return 0;
}

namespace {

std::vector<FileInfo> get_cwd_file_list();
UString to_ustring(const std::string &);

/* static */ bool * FileDisplayList::s_quit_flag = nullptr;

void FileDisplayList::setup() {
    auto styles_ = std::make_shared<ksg::StyleMap>();
    (*styles_) = ksg::styles::construct_system_styles();
    (*styles_)[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
    (*styles_)[ksg::TextArea::k_text_size] = ksg::StylesField(18.f);

    setup(styles_, get_current_working_directory());
}

void FileDisplayList::setup(SharedStylePtr styles_, const std::string & path) {
    assert(styles_);
    set_current_working_directory(path);
    m_cwd_file_list = get_cwd_file_list();
    std::sort(m_cwd_file_list.begin(), m_cwd_file_list.end(),
        [](const FileInfo & rhs, const FileInfo & lhs)
    {
        if (rhs.is_directory != lhs.is_directory) {
            return (rhs.is_directory);
        }
        return rhs.full_path < lhs.full_path;
    });

    std::vector<ksg::Text::UString> files_strings;
    for (const auto & info : m_cwd_file_list) {
        auto pos = info.full_path.find_last_of('/');
        if (pos == std::string::npos) {
            throw std::runtime_error("not a path?");
        }
        files_strings.emplace_back();
        for (auto itr = info.full_path.begin() + pos + 1; itr != info.full_path.end(); ++itr)
            files_strings.back() += UChar(*itr);
        if (info.is_directory) {
            files_strings.back() += U" (directory)";
        }
    }

    m_up_button  .set_size(150.f, 45.f);
    m_exit_button.set_size( 50.f, 45.f);
    m_info_notice.set_size(250.f, 45.f);

    m_file_list_display.set_size(0.f, 45.f);

    m_info_notice.set_text(U"File list " + to_ustring(std::to_string(m_cwd_file_list.size())));
    m_up_button.set_string(U"Up one level...");
    m_exit_button.set_string(U"Exit");
    m_file_list_display.add_options(std::move(files_strings));

    m_up_button.set_press_event([this, styles_]() {
        m_new_list = std::make_unique<FileDisplayList>();
        m_new_list->setup(styles_, get_current_working_directory() + "/..");
    });

    m_exit_button.set_press_event([]() { *s_quit_flag = true; });
    m_file_list_display.set_response_function(
        [this, styles_](std::size_t index, const UString &)
    {
        const auto & info = m_cwd_file_list.at(index);
        if (!info.is_directory) return;
        m_new_list = std::make_unique<FileDisplayList>();
        m_new_list->setup(styles_, info.full_path);
    });

    auto widadder = begin_adding_widgets(*styles_);
    widadder.add(m_info_notice).add_horizontal_spacer();
    if (get_current_working_directory() != "/") { widadder.add(m_up_button); }
    widadder.add(m_exit_button).add_line_seperator();
    widadder.add(m_file_list_display);
}

FileDisplayPtr FileDisplayList::get_new_list() {
    return std::move(m_new_list);
}

// ----------------------------------------------------------------------------

void closedir_norv(DIR *);
using DirectoryPointer = std::unique_ptr<DIR, decltype(&closedir_norv)>;
DirectoryPointer open_directory(const std::string &);

std::vector<FileInfo> get_cwd_file_list() {
    std::vector<FileInfo> rv;
    auto out_itr = std::back_inserter(rv);
    auto path = get_current_working_directory();
    auto dir = open_directory(path);
    static auto eq = [](const char * a, const char * b) { return std::equal(a, a + strlen(a), b); };
    for (auto * res = readdir(dir.get()); res; res = readdir(dir.get())) {
        if (eq(".", res->d_name) || eq("..", res->d_name)) continue;

        auto fullpath = path + "/" + res->d_name;
        struct stat file_info;
        // ignore file if we can't access
        if (stat(fullpath.c_str(), &file_info) == -1) continue;

        *out_itr++ = FileInfo { static_cast<bool>(S_ISDIR(file_info.st_mode)), fullpath };
    }
    return rv;
}

UString to_ustring(const std::string & str) {
    UString rv;
    rv.reserve(str.size());
    for (char c : str) rv += UChar(c);
    return rv;
}

// ----------------------------------------------------------------------------

DirectoryPointer open_directory(const std::string & path) {
    return DirectoryPointer(opendir(path.c_str()), closedir_norv);
}

void closedir_norv(DIR * ptr) { (void)closedir(ptr); }

} // end of <anonymous> namespace
