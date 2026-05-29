QT      += core gui widgets sql printsupport
CONFIG  += c++17
TARGET   = tv_app
SOURCES += main.cpp

# PostgreSQL
unix {
    INCLUDEPATH += /usr/include/postgresql
    LIBS        += -lpq
}

# Путь к бинарнику после сборки
DESTDIR  = .
