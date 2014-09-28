include(../common.pri)

TEMPLATE = app

QT += xml
QT -= gui
CONFIG += qt

HEADERS += fiasconverter.h

SOURCES += main.cpp \
           fiasconverter.cpp
