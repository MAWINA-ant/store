#-------------------------------------------------
#
# Project created by QtCreator 2018-03-26T14:50:28
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = store
TEMPLATE = app


SOURCES += main.cpp \
    application.cpp \
    catalogue.cpp   \
    catitemedit.cpp \
    dialogtpl.cpp   \
    mainwindow.cpp \
    posaction.cpp


HEADERS  += \
    application.h   \
    catalogue.h     \
    catitemedit.h   \
    dialogtpl.h     \
    mainwindow.h \
    posaction.h


FORMS += \
    buttonsframe.ui \
    catitemframe.ui
