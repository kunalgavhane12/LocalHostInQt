QT = core network gui

CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNING QT_DISABLE_DEPRECATED_BEFORE=0x060000
SOURCES += \
        clientconnection.cpp \
        main.cpp \
        webserver.cpp

HEADERS += \
    clientconnection.h \
    webserver.h
