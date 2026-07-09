QT += core gui widgets svg
TARGET = LGUFBrightness
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    DisplayWidget.cpp \
    DisplayHandler.cpp

HEADERS += \
    MainWindow.h \
    DisplayWidget.h \
    DisplayHandler.h

FORMS += \
    MainWindow.ui \
    DisplayWidget.ui

RESOURCES += resources.qrc

# OS-specific libusb-1.0 configuration

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}

win32 {
    isEmpty(LIBUSB_DIR): LIBUSB_DIR = $$PWD/libusb

    INCLUDEPATH += $${LIBUSB_DIR}/include/libusb-1.0

    contains(QT_ARCH, x86_64) {
        LIBS += $${LIBUSB_DIR}/MinGW64/static/libusb-1.0.a
    } else {
        LIBS += $${LIBUSB_DIR}/MinGW32/static/libusb-1.0.a
    }

    LIBS += -lsetupapi -lwinusb
}