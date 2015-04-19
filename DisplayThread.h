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

#include <opencv/cv.h>
#include <opencv2/opencv.hpp>

#include <PvDisplayThread.h>
#include <PvDisplayWnd.h>
#include <PvImage.h>
#include <PvPixelType.h>

class DisplayThread : public    QWidget,
                      public    PvDisplayThread
{
    Q_OBJECT
public:

    DisplayThread   (  /*PvDisplayWnd *aDisplayWnd*/ QWidget * parent = 0 );
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
    QImage*  m_poQtImage;

    // Хранилище с динамически изменяемым размером
    cv::Mat m_oCVMat;

    // Буфер для хранения изображения
    PvBuffer*   m_poImageBuffer;

 //   PvDisplayWnd*   mDisplayWnd;
};

#endif DISPLAYTHREAD_H
