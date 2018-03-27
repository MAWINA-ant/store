#-------------------------------------------------
#
# Project created by QtCreator 2018-03-26T14:50:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = store
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    application.cpp \
    catitemedit.cpp \
    dialogtpl.cpp

HEADERS  += mainwindow.h \
    application.h \
    catitemedit.h \
    dialogtpl.h

FORMS += \
    buttonsframe.ui \
    catitemframe.ui
