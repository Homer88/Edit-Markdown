QT += core testlib
CONFIG += console c++17
TARGET = roundtrip_tests

SOURCES += roundtrip_tests.cpp ../markdownparser.cpp
HEADERS += ../markdownparser.h

INCLUDEPATH += ..
