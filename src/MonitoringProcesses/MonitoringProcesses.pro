#-------------------------------------------------
#
# Project created by QtCreator 2015-09-29T12:18:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MonitoringProcesses
TEMPLATE = app


SOURCES += main.cpp \
    controller/mainwindow.cpp \
    controller/Network.cpp \
    controller/ConfigurationWindow.cpp \
    controller/DataWidget.cpp \
    controller/qcustomplot.cpp \
    controller/Configuration.cpp

HEADERS  += \
    controller/mainwindow.h \
    controller/Network.h \
    controller/ConfigurationWindow.h \
    controller/DataWidget.h \
    controller/qcustomplot.h \
    controller/Configuration.h

FORMS    += \
    view/mainwindow.ui \
    view/ConfigurationWindow.ui \
    view/DataWidget.ui

LIBS    += \
    -lpthread

QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    resourcefile.qrc
