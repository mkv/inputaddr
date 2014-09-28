DESTDIR  = $$PWD/bin
ROOT = $$PWD

INCLUDEPATH += $$PWD/common
SOURCES += $$PWD/common/addressobject.cpp

INCLUDEPATH += $$PWD/3rdparty/ejdb/tcejdb

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic -std=c++0x

unix {
  LIBS += -L$$PWD/3rdparty/ejdb/tcejdb -ltcejdb
}

win32 {
  CONFIG += static
  CONFIG -= debug_and_release
  LIBS += $$PWD/3rdparty/ejdb/tcejdb/tcejdbdll.dll
}

win32:debug {
  CONFIG += console
}
