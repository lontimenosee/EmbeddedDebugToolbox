QT += core gui widgets serialport

CONFIG += c++17
TEMPLATE = app
TARGET = EmbeddedDebugToolbox

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialWidget/serialwidget.cpp

HEADERS += \
    mainwindow.h \
    serialWidget/serialwidget.h

RESOURCES += \
    resources/resources.qrc
