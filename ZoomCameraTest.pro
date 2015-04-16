#-------------------------------------------------
#
# Project created by QtCreator 2015-04-13T04:04:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = ZoomCameraTest
TEMPLATE    = app


SOURCES +=  main.cpp                    \
            zoomcameratestwidget.cpp    \
            DisplayThread.cpp

HEADERS  += zoomcameratestwidget.h      \
            DisplayThread.h

FORMS    += zoomcameratestwidget.ui


win32 {

    INCLUDEPATH    +=   "C:/Program Files (x86)/Pleora Technologies Inc/eBUS SDK/Includes"

    # Путь к библиотекам eBus
    EBUS_LIBS_PATH  =   -"LC:/Program Files (x86)/Pleora Technologies Inc/eBUS SDK/Libraries/"
    # Добавить к пути
    LIBS    +=  $$EBUS_LIBS_PATH


 #   EBUS_LIBS_POSTFIX = "_Dbg"

    CONFIG ( release, debug | release  ) {

        message( WIN32 - Release    )
        EBUS_LIBS_POSTFIX = ""
    }

    CONFIG ( debug, debug | release  ) {

        message( WIN32 - Debug      )
        EBUS_LIBS_POSTFIX = "_Dbg"
    }

    # Добавить библиотеки
    LIBS    +=  -lPvGUI64_VC11$$EBUS_LIBS_POSTFIX   -lPvBase64$$EBUS_LIBS_POSTFIX           \
                -lPvBuffer64$$EBUS_LIBS_POSTFIX     -lPvDevice64$$EBUS_LIBS_POSTFIX         \
                -lPvStream64$$EBUS_LIBS_POSTFIX     -lPtUtilsLib64$$EBUS_LIBS_POSTFIX       \
                -lPvGenICam64$$EBUS_LIBS_POSTFIX    -lPvPersistence64$$EBUS_LIBS_POSTFIX    \
                -lPvAppUtils64$$EBUS_LIBS_POSTFIX

    # Вывести пути линковки
    message( $$LIBS )
}
