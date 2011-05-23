
TEMPLATE = app
TARGET = atnf
LIBS = -lgloox
CONFIG += console release
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += client.cpp \
    main.cpp
HEADERS += client.h

QT -= gui
