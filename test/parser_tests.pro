QT += core gui testlib
CONFIG += console c++17
TARGET = parser_tests

SOURCES += parser_tests.cpp ../markdownparser.cpp
HEADERS += ../markdownparser.h

INCLUDEPATH += ..
