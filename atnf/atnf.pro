
TEMPLATE = app
TARGET = atnf
LIBS = -lgloox
CONFIG += console
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += client.cpp \
    main.cpp
HEADERS += client.h

QT -= gui
