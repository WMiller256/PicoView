TEMPLATE = app
TARGET = ~/bin/picoview

QT = core gui widgets multimediawidgets multimedia
CONFIG += debug
LIBS += -lstdc++fs

SOURCES += main.c++ picoview.c++
HEADERS += picoview.h

RESOURCES += picoview.qrc

