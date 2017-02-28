CONFIG += qt
QT += core gui widgets qml quick quickwidgets sql


SOURCES += $$files(src/*.cpp, true)
HEADERS += $$files(src/*.h, true)
INCLUDEPATH += src/

FORMS    += $$files(src/*.ui, true)
RESOURCES += ressources.qrc

win32{
        INCLUDEPATH += "C:/msys64/mingw64/lib/libzip/include"
        LIBS	+= -llibzip
}

unix{
        LIBS    += /usr/lib64/libzip.so
}


