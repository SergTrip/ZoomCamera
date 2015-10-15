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

#  Установить версию OpenCV
#OPENCV_VERSION  = V2
OPENCV_VERSION = V3

# Если компилируем с библиотекой версии 2.10
equals ( OPENCV_VERSION, "V2" ){
    # Если операционная система Windows
    win32 {
        # Путь к корню .h файлов
        OPENCV_INCLUDE_PATH         = D:/Qt/opencv2/sources/include
        OPENCV_INCLUDE_MODULE_PATH  = D:/Qt/opencv2/sources/modules

        # Путь к библиотекам
        OPENCV_LIBS_PATH            = D:/Qt/opencv2/build/x64/vc11/lib
        # Добавить путь к библиотекам
        LIBS    += -L$$OPENCV_LIBS_PATH

        CONFIG ( release, debug | release ) {

            message( " Version OpenCV - 2.10 - Release " )
            message( $$OPENCV_LIBS_PATH )
            message( $$LIBS )

            LIBS    += -lopencv_core2410         -lopencv_imgproc2410
            LIBS    += -lopencv_objdetect2410    #-lopencv_video2410
            LIBS    += -lopencv_highgui2410

        }

        CONFIG ( debug, debug | release ) {

            message( " Version OpenCV - 2.10 - Debug " )
            message( $$OPENCV_LIBS_PATH )
            message( $$LIBS )

            LIBS    += -lopencv_core2410d       -lopencv_imgproc2410d
            LIBS    += -lopencv_objdetect2410d  #-lopencv_video2410d
            LIBS    += -lopencv_highgui2410d
        }
    }

    # Если операционная система Linux
    unix {
        # Путь к корню .h файлов
        OPENCV_INCLUDE_PATH        = ~/Libs/OpenCV/opencv-3.0.0-alpha/include
        OPENCV_INCLUDE_MODULE_PATH = ~/Libs/OpenCV/opencv-3.0.0-alpha/build/modules

        # Путь к библиотекам
        OPENCV_LIBS_PATH           = ~/Libs/OpenCV/opencv-3.0.0-alpha/build/lib

        CONFIG ( release, debug | release  ) {
            LIBS    += -L$$OPENCV_LIBS_PATH
            LIBS    += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect
        }

        CONFIG ( debug, debug | release  ) {
            LIBS    += -L$$OPENCV_LIBS_PATH
            LIBS    += -lopencv_cored -lopencv_imgcodecsd -lopencv_imgprocd -lopencv_objdetectd
        }
    }
}

# Если компилируем с библиотекой версии 3.00
equals( OPENCV_VERSION, "V3" ) {
    # Если операционная система Windows
    win32 {
        # Путь к корню .h файлов
        OPENCV_INCLUDE_PATH         = D:/Qt/opencv3/OpenCV_Git/include
        OPENCV_INCLUDE_MODULE_PATH  = D:/Qt/opencv3/OpenCV_Git/modules

        # Путь к библиотекам
        OPENCV_LIBS_PATH            = D:/Qt/opencv3/OpenCV_Git_Make/lib

        CONFIG ( release, debug | release  ) {

            message( " Version OpenCV - 3.00 - Release " )
            message( $$OPENCV_LIBS_PATH )

            LIBS    += -L$$OPENCV_LIBS_PATH/Release
            LIBS    += -lopencv_core300         -lopencv_imgproc300
            LIBS    += -lopencv_objdetect300    -lopencv_videoio300
            LIBS    += -lopencv_highgui300      -lopencv_imgcodecs300
        }

        CONFIG ( debug, debug | release  ) {

            message( " Version OpenCV - 3.00 - Debug " )
            message( $$OPENCV_LIBS_PATH )

            LIBS    += -L$$OPENCV_LIBS_PATH/Debug
            LIBS    += -lopencv_core300d        -lopencv_imgproc300d
            LIBS    += -lopencv_objdetect300d   #-lopencv_videoio300d
            LIBS    += -lopencv_highgui300d     -lopencv_imgcodecs300d
        }
    }

    # Если операционная система Linux
    unix {
        # Путь к корню .h файлов
        OPENCV_INCLUDE_PATH        = ~/Libs/OpenCV/opencv-3.0.0-alpha/include
        OPENCV_INCLUDE_MODULE_PATH = ~/Libs/OpenCV/opencv-3.0.0-alpha/build/modules

        # Путь к библиотекам
        OPENCV_LIBS_PATH           = ~/Libs/OpenCV/opencv-3.0.0-alpha/build/lib

        CONFIG ( release, debug | release  ) {
            LIBS    += -L$$OPENCV_LIBS_PATH
            LIBS    += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect
        }

        CONFIG ( debug, debug | release  ) {
            LIBS    += -L$$OPENCV_LIBS_PATH
            LIBS    += -lopencv_cored -lopencv_imgcodecsd -lopencv_imgprocd -lopencv_objdetectd
        }
    }
}

message( $$OPENCV_INCLUDE_PATH )
message( $$OPENCV_INCLUDE_MODULE_PATH )

INCLUDEPATH += $$OPENCV_INCLUDE_PATH

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/core/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/legacy/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/contrib/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/imgproc/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/imgcodecs/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/photo/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/video/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/videoio/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/objdetect/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/highgui/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/features2d/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/calib3d/include
INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/ml/include

INCLUDEPATH += $$OPENCV_INCLUDE_MODULE_PATH/flann/include
