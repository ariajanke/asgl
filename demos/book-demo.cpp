#include <asgl/Frame.hpp>
#include <asgl/ImageWidget.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/TextButton.hpp>
#include <asgl/OptionsSlider.hpp>
#include <asgl/BookFrame.hpp>

#include <asgl/sfml/SfmlEngine.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Sleep.hpp>

#include <common/Util.hpp>

#include <cassert>

namespace {

using namespace asgl;

class CoverPage final : public BookFrame::Page {
public:
    void setup() {
        // maybe we can add a font style for this? c:?
        m_title.set_string(U"Super Short Fruit Book");

        m_about.set_limiting_line(300);
        m_about.set_string(U"This little book is supposed to act as a "
                           "demonstration tool for the features of this "
                           "library.");

        m_author.set_string(U"By: Aria");

        begin_adding_widgets()
            .add_horizontal_spacer().add(m_title).add_horizontal_spacer().add_line_seperator()
            .add(m_about).add_horizontal_spacer().add(m_author);
    }
private:
    TextArea m_title;

    TextArea m_about;
    TextArea m_author;
};

class BackPage final : public BookFrame::Page {
public:
    void setup() {
        m_biblography.set_string(
            U"Images of fruit were graciously provided by "
            "\"freefoodphotos.com\" each of which are released under the "
            "creative commons attribution (3.0) license.");
        m_biblography.set_limiting_line(400);
        m_biblography.set_fixed_height(250);
        begin_adding_widgets().add_horizontal_spacer().add(m_biblography).add_horizontal_spacer();
    }
private:
    TextArea m_biblography;
};

class FruitPage final : public BookFrame::Page {
public:
    struct FavoriteMarker {
        virtual ~FavoriteMarker() {}
        virtual void mark_current_page_as_favorite() = 0;
        virtual void remove_current_page_as_favorite() = 0;
        static FavoriteMarker & null_instance() {
            class NullInst final : public FavoriteMarker {
                void mark_current_page_as_favorite() final {}
                void remove_current_page_as_favorite() final {}
            };
            static NullInst inst;
            return inst;
        }
    };

    void set_page_info
        (ImageLoader & image_loader, const std::string & image_filename,
         const UString & blurb)
    {
        auto img_ptr = m_fruit_image.load_image(image_loader, image_filename);
        m_blurb.set_string(blurb);
        m_blurb.set_limiting_line(280);
        m_fruit_image.set_size((img_ptr->image_size()*2) / 3);
        m_fruit_image.set_view_rectangle(Rectangle(Vector(0, 0), img_ptr->image_size()));
    }

    void assign_favorite_marker(FavoriteMarker & marker) {
        m_favorite_marker = &marker;
    }

    enum { k_as_favorite, k_as_regular, k_as_maximum };

    void set_favorite_strings(decltype(k_as_favorite) e) {
        static constexpr const auto k_regular_ta_string
            = U"Click here to mark as your favorite.";
        static constexpr const auto k_regular_btn_string
            = U"Make Favorite";
        static constexpr const auto k_favorite_ta_string
            = U"This is your current favorite.";
        static constexpr const auto k_favorite_btn_string
            = U"Remove Favorite";

        using BlankFunctor = Button::BlankFunctor;
        auto do_string_change = [this](const UChar * ta_string, const UChar * btn_string, BlankFunctor && f) {
            if (   m_make_fav_text.string() != ta_string
                || m_make_favorite.string() != btn_string)
            {
                flag_needs_whole_family_geometry_update();
            } else { return; }
            m_make_fav_text.set_string(ta_string);
            m_make_favorite.set_string(btn_string);
            m_make_favorite.set_press_event(std::move(f));
        };
        switch (e) {
        case k_as_favorite:
            do_string_change(k_favorite_ta_string, k_favorite_btn_string, [this]() {
                m_favorite_marker->remove_current_page_as_favorite();
            });
            return;
        case k_as_regular:
            do_string_change(k_regular_ta_string, k_regular_btn_string, [this]() {
                m_favorite_marker->mark_current_page_as_favorite();
            });
            break;
        case k_as_maximum:
            m_make_fav_text.set_string(choose_larger_string(
                { k_regular_ta_string, k_favorite_ta_string },
                m_make_fav_text,
                [this](const UChar * ustr) { m_make_fav_text.set_string(ustr); }
                ));
            m_make_favorite.set_string(choose_larger_string(
                { k_regular_btn_string, k_favorite_btn_string },
                m_make_favorite,
                [this](const UChar * ustr) { m_make_favorite.set_string(ustr); }
                ));
            break;
        }
    }

private:
    template <typename Func>
    static const UChar * choose_larger_string(
        std::initializer_list<const UChar *> strings,
        Widget & widget, Func && f)
    {
        const UChar * chosen = nullptr;
        Size maxsizet;
        for (auto str : strings) {
            f(str);
            widget.update_size();
            // fuck it, prioritize only width
            if (widget.size().width > maxsizet.width) {
                chosen = str;
            }
            maxsizet = max_size(maxsizet, widget.size());
        }
        return chosen;
    }

public:

    void setup() {
        begin_adding_widgets()
            .add(m_fruit_image).add_horizontal_spacer().add(m_blurb).add_line_seperator()
            .add_horizontal_spacer().add(m_make_fav_text).add(m_make_favorite);
    }

    void stylize(const StyleMap & map) final {
        BookFrame::Page::stylize(map);
        set_favorite_strings(k_as_maximum);
    }

private:
    ImageWidget m_fruit_image;
    TextArea m_blurb;

    TextArea m_make_fav_text;
    TextButton m_make_favorite;

    FavoriteMarker * m_favorite_marker = &FavoriteMarker::null_instance();
};

class FruitBook final : public BookFrame {
public:
    void setup(ImageLoader & loader) {
        m_cover_page.setup();
        m_back_page.setup();
        auto itr = k_fruit_data.begin();
        for (auto & page : m_fruit_pages) {
            using std::get;
            page.set_page_info(loader, get<0>(*itr), get<1>(*itr));
            page.setup();
            page.assign_favorite_marker(m_favorite_marker);
            ++itr;
        }

        PageContainer pages;
        pages.push_back(&m_cover_page);
        for (auto & page : m_fruit_pages) pages.push_back(&page);
        pages.push_back(&m_back_page);

        set_pages(std::move(pages));
    }

    void stylize(const StyleMap & smap) override {
        m_cover_page.stylize(smap);
        m_back_page.stylize(smap);
        for (auto & page : m_fruit_pages) {
            page.stylize(smap);
        }
    }

    void process_event(const Event & event) override {
        BookFrame::process_event(event);
        // part of a "hack" to ensure that the fruit pages have the correct
        // strings for the "favorite fruit" feature
        //
        // any first event should trigger a change of the strings to their
        // correct values after set up
        if (m_favorite_marker.needs_update()) {
            for (auto & page : m_fruit_pages) {
                if (   &page == &current_page()
                    && m_favorite_marker.current_is_new_favorite())
                {
                    page.set_favorite_strings(FruitPage::k_as_favorite);
                } else {
                    page.set_favorite_strings(FruitPage::k_as_regular);
                }
            }
            m_favorite_marker.unset_update_need();
        }
    }

private:
    static constexpr auto k_fruit_data = {
        std::make_tuple("images/apple.jpg",
            U"These are apples, there are many kinds. Some are good for "
            "baking."),
        std::make_tuple("images/bananas.jpg",
            U"These are bananas."),
        std::make_tuple("images/orange.jpg",
            U"This is an orange.")
    };

    class BookFavoriteMarker final : public FruitPage::FavoriteMarker {
    public:
        void mark_current_page_as_favorite() final
            { m_current_is_favorite = m_needs_update = true; }
        void remove_current_page_as_favorite() final {
            m_current_is_favorite = false;
            m_needs_update = true;
        }
        bool current_is_new_favorite() const { return m_current_is_favorite; }
        bool needs_update() const { return m_needs_update; }
        void unset_update_need() { m_needs_update = false; }

    private:
        // a "trick"/"hack" to do that first update where we set the
        // favorite fruit widget strings to their correct default value
        bool m_needs_update = true;
        bool m_current_is_favorite = false;
    };

    std::array<FruitPage, k_fruit_data.size()> m_fruit_pages;
    CoverPage m_cover_page;
    BackPage m_back_page;
    BookFavoriteMarker m_favorite_marker;
};

class TopLevelFrame final : public Frame {
public:
    void setup(ImageLoader & loader) {
        m_book.setup(loader);
        {
        std::vector<UString> opts;
        opts.reserve(m_book.page_count());
        for (int i = 1; i != m_book.page_count() + 1; ++i) {
            auto str = "- Page " + std::to_string(i) + " -";
            UString ustr;
            ustr.reserve(str.size());
            for (auto c : str) ustr += UChar(c);
            opts.emplace_back(std::move(ustr));
        }
        m_page_turner.set_options(std::move(opts));
        }
        m_page_turner.set_option_change_event([this]() {
            m_book.flip_to_page(m_page_turner.selected_option_index());
        });

        m_quit_app.set_string(U"Exit App");
        m_quit_app.set_press_event([this]() { m_request_quit = true; });

        m_force_geo_update.set_string(U"Force Geometry Update");
        m_force_geo_update.set_press_event([this]() {
            flag_needs_whole_family_geometry_update();
        });

        begin_adding_widgets()
            .add(m_book).add_line_seperator()
            .add_horizontal_spacer().add(m_page_turner).add_horizontal_spacer().add_line_seperator()
            .add(m_force_geo_update).add(m_quit_app);
    }

    bool is_requesting_exit() const
        { return m_request_quit; }

private:
    FruitBook m_book;
    OptionsSlider m_page_turner;
    TextButton m_force_geo_update;
    TextButton m_quit_app;

    bool m_request_quit = false;
};

} // end of <anonymous> namespace

int main() {

    SfmlFlatEngine engine;
    TopLevelFrame test_frame;
    sf::RenderWindow win;

    engine.assign_target_and_states(win, sf::RenderStates::Default);
    engine.load_global_font("font.ttf");
    test_frame.setup(engine);
    engine.stylize(test_frame);
    test_frame.check_for_geometry_updates();

    win.create(sf::VideoMode
        (std::max(200, test_frame.width ())
        ,std::max(200, test_frame.height()))
        /* title */ ,"");
    win.setFramerateLimit(20);
    while (win.isOpen()) {
        if (test_frame.is_requesting_exit()) return 0;
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            test_frame.process_event(SfmlFlatEngine::convert(event));
            switch (event.type) {
            case sf::Event::Closed:
                win.close();
                break;
            default: break;
            }
        }
        }

        sf::sleep(sf::microseconds(16667));
        test_frame.check_for_geometry_updates();
        win.clear(sf::Color(40, 180, 40));
        test_frame.draw(engine);

        win.display();
    }
}
