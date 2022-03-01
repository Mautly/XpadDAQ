#-------------------------------------------------
#
# Project created by QtCreator 2013-05-27T15:10:43
#
#-------------------------------------------------

QT       += network gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XpadDAQ
TEMPLATE = app


SOURCES += main.cpp \
    daqclient.cpp \
    qcustomplot.cpp \
    daqviewer.cpp \
    receiveimagesthread.cpp

HEADERS  += \
    daqclient.h \
    qcustomplot.h \
    daqviewer.h \
    receiveimagesthread.h

FORMS    += \
    daqclient.ui \
    daqviewer.ui

LIBS += -pthread

macx:  ICON = daq_logo.icns
win32: RC_FILE = daq_logo.rc

macx: QMAKE_MAC_SDK = macosx10.12
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
