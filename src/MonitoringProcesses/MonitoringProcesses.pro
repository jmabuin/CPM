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
    controller/Configuration.cpp \
    controller/ssh_handler.cpp \
    controller/ManageClusterWindow.cpp

HEADERS  += \
    controller/mainwindow.h \
    controller/Network.h \
    controller/ConfigurationWindow.h \
    controller/DataWidget.h \
    controller/qcustomplot.h \
    controller/Configuration.h \
    controller/ssh_handler.h \
    controller/ManageClusterWindow.h

FORMS    += \
    view/mainwindow.ui \
    view/ConfigurationWindow.ui \
    view/DataWidget.ui \
    view/ManageClusterWindow.ui

LIBS    += \
    -lpthread \
    -lssh

QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    resourcefile.qrc

QMAKE_CLEAN += MonitoringProcesses
