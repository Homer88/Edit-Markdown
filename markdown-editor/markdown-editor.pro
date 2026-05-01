#-------------------------------------------------
#
# Project created by QtCreator
# Совместим с Qt4, Qt5 и Qt6
#
#-------------------------------------------------

QT += core gui widgets

# Для Qt6 модули разделены, но widgets включает нужное
# В Qt4 и Qt5 это тоже работает

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
else: QT += printsupport

# Поддержка проверки орфографии через Hunspell
# Библиотека должна быть установлена в системе
unix {
    # Linux/macOS
    LIBS += -lhunspell
    INCLUDEPATH += /usr/include/hunspell \
                   /usr/local/include/hunspell
    LIBS += -L/usr/lib/x86_64-linux-gnu \
            -L/usr/local/lib
}

win32 {
    # Windows - пути нужно настроить под вашу установку
    # INCLUDEPATH += C:/hunspell/include
    # LIBS += -LC:/hunspell/lib -lhunspell
    message("Для Windows настройте пути к Hunspell вручную")
}

# Исходные файлы
SOURCES += main.cpp \
           mainwindow.cpp \
           markdownparser.cpp \
           spellchecker.cpp \
           helpwindow.cpp

# Заголовочные файлы
HEADERS += mainwindow.h \
           markdownparser.h \
           spellchecker.h \
           helpwindow.h

# Ресурсы (если будут добавлены иконки)
# RESOURCES += resources.qrc

# Настройки компилятора
CONFIG += c++11

# Для Qt4 может потребоваться отключить c++11 если компилятор старый
# greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11

# Имя целевого файла
TARGET = MarkdownEditor

# Пути для установки
target.path = $$[QT_INSTALL_BINS]/MarkdownEditor
INSTALLS += target

# Дополнительные флаги для предупреждений
QMAKE_CXXFLAGS += -Wall -Wextra

# Для Windows добавить консоль при отладке
win32:CONFIG(release, debug|release): TARGET = MarkdownEditor
else:win32:CONFIG(debug, debug|release): TARGET = MarkdownEditor
else:unix: TARGET = MarkdownEditor
