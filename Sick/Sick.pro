#-------------------------------------------------
#
# Project created by QtCreator 2014-02-01T12:12:20
#
#-------------------------------------------------

QT       -= gui

TARGET = Sick
TEMPLATE = lib

DEFINES += SICK_LIBRARY

SOURCES += sick.cpp \
    SickSocket.cpp \
    SickComponent.cpp \
    SickLDMRSSensor.cpp \
    AbstractSickSensor.cpp \
    SickLMS.cpp \
    SickLMSSensor.cpp

HEADERS +=\
    SickComponent.h \
    SickLDMRSData.h \
    SickSocket.h \
    SickLDMRSSensor.h \
    AbstractSickSensor.h \
    SickLMSSensor.h \
    SickLMSData.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    CMakeLists.txt
