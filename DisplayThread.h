#ifndef DISPLAYTHREAD_H
#define DISPLAYTHREAD_H

// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QMessageBox>
#include <QColor>

#include <PvDisplayThread.h>
#include <PvDisplayWnd.h>
#include <PvImage.h>
#include <PvPixelType.h>
#include <PvBuffer.h>
#include <PvBufferConverter.h>

//#include <opencv/cv.h>
#include "/home/sergey/libs/opencv-3.0.0/include/opencv/cv.h"
//#include <opencv2/opencv.hpp>
#include "/home/sergey/libs/opencv-3.0.0/include/opencv2/opencv.hpp"

//#include <opencv2/core/core.hpp>
#include "/home/sergey/libs/opencv-3.0.0/modules/core/include/opencv2/core.hpp"
//#include <opencv2/objdetect/objdetect.hpp>
#include "/home/sergey/libs/opencv-3.0.0/modules/objdetect/include/opencv2/objdetect.hpp"

//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/videoio/videoio.hpp>

#define FACE_CASCADE_NAME   "D:\\Dropbox\\Germanij\\Projects\\Faces\\Data\\haarcascades\\haarcascade_frontalface_alt.xml"
//#define FACE_CASCADE_NAME   "../../Data/lbpcascades/lbpcascade_frontalface.xml"
#define EYE_CASCADE_NAME    "../../Data/haarcascades/haarcascade_eye_tree_eyeglasses.xml"

#define FILE_NAME           "../../Data/MyFace.3gp"
#define PHOTO_NAME          "../../Data/Face.jpg"

class DisplayThread : public    QWidget,
                      public    PvDisplayThread
{
    Q_OBJECT
public:

    DisplayThread   ( QWidget * parent = 0 );
    ~DisplayThread  ();

protected:

    void OnBufferRetrieved  ( PvBuffer *aBuffer );
    void OnBufferDisplay    ( PvBuffer *aBuffer );
    void OnBufferDone       ( PvBuffer *aBuffer );

    // Реализация унаследованного класса перерисовки
    void paintEvent         ( QPaintEvent* event );

    void showEvent          ( QShowEvent*  event );

private:
    // Экземпляр изображения Qt
    QImage*     m_poQtImage;

    // Буфер для хранения изображения
    PvBuffer*   m_poImageBuffer;

    // *********** Переменные для обнаружения лиц ****************
private:
    // Непосредственно опеределяет расположение лица
    void detectAndDraw();

private:
    // Охватывающий прямоугольник
    QRect                   m_oQtFaceLocationRect;

    // Хранилище с динамически изменяемым размером
    cv::Mat*                m_poCVMat;

    cv::CascadeClassifier   m_pCVFaceCascade;
    cv::CascadeClassifier   m_pCVEyeCascade;
};

#endif // DISPLAYTHREAD_H
