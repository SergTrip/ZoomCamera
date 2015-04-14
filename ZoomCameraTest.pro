#-------------------------------------------------
#
# Project created by QtCreator 2015-04-13T04:04:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZoomCameraTest
TEMPLATE = app


SOURCES +=  main.cpp                    \
            zoomcameratestwidget.cpp    \
            DisplayThread.cpp

HEADERS  += zoomcameratestwidget.h      \
            DisplayThread.h

FORMS    += zoomcameratestwidget.ui


win32 {

    INCLUDEPATH += "C:/Program Files (x86)/Pleora Technologies Inc/eBUS SDK/Includes"

#    OPENCV_LIBS_PATH            = D:/Qt/opencv3/OpenCV_Git_Make/lib

#    CONFIG ( release, debug|release ) {

#        LIBS    += -L$$OPENCV_LIBS_PATH/Release
#        LIBS    += -lopencv_core300 -lopencv_imgcodecs300 -lopencv_imgproc300 -lopencv_highgui300
#    }

#    CONFIG ( debug, debug|release ) {

#        LIBS    += -L$$OPENCV_LIBS_PATH/Debug
#        LIBS    += -lopencv_core300d -lopencv_imgcodecs300d -lopencv_imgproc300d -lopencv_highgui300d
#    }
}
