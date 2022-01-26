QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camara/zwoasi.cpp \
    dlgaskinfo.cpp \
    dlgimgprocesada.cpp \
    fass_file.cpp \
    fass_preproc.cpp \
    fcut.cpp \
    imgdebug.cpp \
    main.cpp \
    fmain.cpp \
    meadecommander.cpp \
    overlay1.cpp \
    widgets/wcut.cpp \
    widgets/wimagen.cpp \
    widgets/wselector.cpp

HEADERS += \
    camara/zwoasi.h \
    dlgaskinfo.h \
    dlgimgprocesada.h \
    fass_file.h \
    fass_preproc.h \
    fcut.h \
    fmain.h \
    imgdebug.h \
    meadecommander.h \
    os_includes.h \
    overlay1.h \
    widgets/wcut.h \
    widgets/wimagen.h \
    widgets/wselector.h

FORMS += \
    dlgaskinfo.ui \
    dlgimgprocesada.ui \
    fcut.ui \
    fmain.ui \
    meadecommander.ui \
    overlay1.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += camara/include

win32 {
LIBS += -l$${PWD}/camara/lib/x64/ASICamera2
}

unix {
LIBS += $${PWD}/camara/lib_linux/libASICamera2.a
LIBS += -lusb-1.0
}

RESOURCES += \
    rc.qrc
