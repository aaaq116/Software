QT       += core gui
QT       += serialport
QT       += axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialworker.cpp

HEADERS += \
    mainwindow.h \
    serialworker.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc

INCLUDEPATH += C:\Users\cgn12\AppData\Local\Programs\Python\Python312\include
LIBS += -LC:\Users\cgn12\AppData\Local\Programs\Python\Python312\libs -lpython312


DISTFILES += \
    get_token.py
