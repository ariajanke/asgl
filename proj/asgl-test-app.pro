QT      -= core gui
TEMPLATE = app
# CONFIG  += staticlib
CONFIG  -= c++11

QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS   += -std=c++17
LIBS           += -ltinyxml2 -lsfml-graphics -lsfml-window -lsfml-system -lz \
                  -L/usr/lib/x86_64-linux-gnu

linux {
    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX
    contains(QT_ARCH, i386) {
        LIBS += -L../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        LIBS += -L../../bin/linux/g++-x86_64 \
                -L/usr/lib/x86_64-linux-gnu
    }
}

debug {
    TARGET  = ksg-d
    LIBS += "-L$$PWD/../lib/cul"
    LIBS   += -lcommon
}

SOURCES += ../demos/demo.cpp


SOURCES += \
    ../src/ArrowButton.cpp      \
    ../src/Button.cpp           \
    ../src/Draggable.cpp        \
    ../src/DrawCharacter.cpp    \
    ../src/DrawTriangle.cpp     \
    ../src/Frame.cpp            \
    ../src/FrameBorder.cpp      \
    ../src/ImageWidget.cpp      \
    ../src/OptionsSlider.cpp    \
    ../src/ProgressBar.cpp      \
    ../src/SfmlFlatRenderer.cpp \
    ../src/StyleMap.cpp         \
    ../src/TextArea.cpp         \
    ../src/TextButton.cpp       \
    ../src/Text.cpp             \
    ../src/Widget.cpp           \
    ../src/EditableText.cpp     \
    ../src/FocusWidget.cpp      \
    ../src/Event.cpp

HEADERS += \
    ../inc/asgl/SfmlFlatRenderer.hpp \
    \ # private headers
    # <none>
    \ # public headers
    ../inc/asgl/DrawCharacter.hpp  \
    ../inc/asgl/ArrowButton.hpp    \
    ../inc/asgl/Button.hpp         \
    ../inc/asgl/Draggable.hpp      \
    ../inc/asgl/DrawTriangle.hpp   \
    ../inc/asgl/Frame.hpp          \
    ../inc/asgl/FrameBorder.hpp    \
    ../inc/asgl/ImageWidget.hpp    \
    ../inc/asgl/OptionsSlider.hpp  \
    ../inc/asgl/SelectionList.hpp  \
    ../inc/asgl/ProgressBar.hpp    \
    ../inc/asgl/StyleMap.hpp       \
    ../inc/asgl/TextArea.hpp       \
    ../inc/asgl/TextButton.hpp     \
    ../inc/asgl/Text.hpp           \
    ../inc/asgl/Widget.hpp         \
    ../inc/asgl/Visitor.hpp        \
    ../inc/asgl/ForwardWidgets.hpp \
    ../inc/asgl/EditableText.hpp   \
    ../inc/asgl/FocusWidget.hpp    \
    ../inc/asgl/Event.hpp

INCLUDEPATH += \
    ../inc           \
    ../lib/cul/inc
