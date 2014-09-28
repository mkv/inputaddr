include(../common.pri)

TEMPLATE = app

CONFIG -= debug_and_release console
unix:CONFIG += debug

HEADERS += progressdialog.h \
           indexingthread.h \
           inputaddrform.h \
           addrobjmodel.h \
           inputmodel.h

SOURCES += main.cpp \
           progressdialog.cpp \
           indexingthread.cpp \
           inputaddrform.cpp \
           addrobjmodel.cpp \
           inputmodel.cpp

FORMS   = inputaddrform.ui \
          progressdialog.ui

RC_FILE   += inputaddr.rc
RESOURCES += inputaddr.qrc

TRANSLATIONS += app_ru.ts

CODECFORTR = UTF-8
