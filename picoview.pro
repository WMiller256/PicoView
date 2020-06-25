TEMPLATE = app
TARGET = picoview

QT = core gui widgets
CONFIG += debug
LIBS += -lstdc++fs

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.c++ picoview.c++
HEADERS += picoview.h

