TEMPLATE = app
TARGET = picoview

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.c++ picoview.c++

HEADERS += picoview.h
