// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "DisplayThread.h"

///
/// \brief Constructor
///

DisplayThread::DisplayThread(QWidget *parent) :
    QWidget         ( parent    ),
    m_poImageBuffer ( 0         ),
    m_poQtImage     ( 0         ),
    m_poCVMat       ( 0         )
{

    //*************** Подготовка OpenCV ************************

    qDebug() << "Face cascade path: "<< FACE_CASCADE_NAME;
    // Load the cascades
    if( !m_pCVFaceCascade.load( FACE_CASCADE_NAME ) )
    {
        qDebug() << ("--(!)Error loading face cascade\n");
        return;
    }

    if( !m_pCVEyeCascade.load(  EYE_CASCADE_NAME  ) )
    {
        qDebug() << ("--(!)Error loading eyes cascade\n");
        return;
    }
}


///
/// \brief Destructor
///

DisplayThread::~DisplayThread()
{
    // Если буффер еще не выделен
       if( m_poImageBuffer != NULL )
       {
           // Освобдить память
           delete m_poImageBuffer;
           m_poImageBuffer = NULL;
       }
}


///
/// \brief Callback from PvDisplayThread
///

void DisplayThread::OnBufferDisplay( PvBuffer *aBuffer )
{
    // Преобразовуем полученный буфера в изображение
    PvImage* pvImage = aBuffer->GetImage();

    PvResult res;

    // Если буффер еще не выделен
    if( m_poImageBuffer == 0 )
    {
        qDebug() <<  "m_poImageBuffer == 0";

        // Создать новый буффер
        m_poImageBuffer = new PvBuffer( aBuffer->GetPayloadType() );
        qDebug() << "Payload Type:" << aBuffer->GetPayloadType();

        // Если изубражение уже существует
        if ( m_poQtImage != 0 )
        {
            // Очистить изображение
            delete m_poQtImage;
            // Сообщить об этом
            qDebug() <<  " delete m_poQtImage ";
        }

        // Создать мзображение Qt c таким же размером
        m_poQtImage = new QImage( pvImage->GetWidth(), pvImage->GetHeight(), QImage::Format_RGB32 );

        // Вывидим размеры нового изображения
        qDebug() <<  " new QImage" << pvImage->GetWidth() << " x " <<  pvImage->GetHeight();

        // Привязываем данные фрейма к озображнию
        res = (m_poImageBuffer->GetImage())->Attach(    m_poQtImage ->bits()        ,
                                                        pvImage     ->GetWidth()    ,
                                                        pvImage     ->GetHeight()   ,
                                                        PV_PIXEL_QT_RGB32              );
        // Проверяем результат операции
        switch ( res.GetCode() )
        {
            case PvResult::Code::INVALID_PARAMETER:
                qDebug() <<  "Attach - Invalid parametr";
                break;

            case PvResult::Code::OK:
                qDebug() << "Attach - OK";
                break;
        }

        // Если  OpenCV буфер уже был создан
        if ( m_poCVMat != 0 )
        {
            // Очистить его
            delete  m_poCVMat;
        }
         // Подключаем данные изображения к OpenCV
        m_poCVMat = new cv::Mat(    pvImage     ->GetHeight()   ,
                                    pvImage     ->GetWidth()    ,
                                    CV_8UC4                     ,
                                    m_poQtImage ->bits()            );

        // Вывидим размеры нового изображения
        qDebug() <<  " new Mat" << m_poCVMat->cols << " x " <<  m_poCVMat->rows;

    }

    // Преобразуем полученный буфер в нужный формат
    PvBufferConverter bufferConverter;

    // Проверить, поддерживается ли преобразование
    if ( !bufferConverter.IsConversionSupported( pvImage                        -> GetPixelType(),
                                                 (m_poImageBuffer->GetImage())  -> GetPixelType()  ) )
    {
        // QMessageBox::warning( this, "Not support", "Not format support");
        qDebug() << "Not format support";

        return;
    }

    // Коневертируем формат
    res = bufferConverter.Convert( aBuffer, m_poImageBuffer );

    qDebug() <<  " Source format"       << pvImage->GetPixelType() ;
    qDebug() <<  " Distination format"  << (m_poImageBuffer->GetImage())->GetPixelType();

    switch ( res.GetCode() )
    {
        case PvResult::Code::INVALID_DATA_FORMAT:
            qDebug() << "Convertion - Invalid format";
            break;

        case PvResult::Code::GENERIC_ERROR:
            qDebug() << "Convertion - Generic error";
            break;

        case PvResult::Code::OK:
            qDebug() << "Convertion - OK";
            break;
    }

    // Выведим параметры буфера результата
    qDebug() <<  " Distination Buffer" << (m_poImageBuffer->GetImage())->GetWidth() << " x " <<  (m_poImageBuffer->GetImage())->GetHeight();

    // Ищем лица
    detectAndDraw();

    this->update();
    //this->repaint();

    qDebug() << "Update....";
}


///
/// \brief Callback from PvDisplayThread
///

void DisplayThread::OnBufferRetrieved( PvBuffer *aBuffer )
{
}


///
/// \brief Callback from PvDisplayThread
///

void DisplayThread::OnBufferDone( PvBuffer *aBuffer )
{
}

void DisplayThread::paintEvent(QPaintEvent *event)
{
    // Класс для рисования
    QPainter painter( (QWidget*)this );

//    if( m_poQtImage != 0 )
//    {
        // Выводим изображение
        painter.drawImage( 0, 0,
                           m_poQtImage->scaled( this->width(),
                                                this->height(),
                                                Qt::KeepAspectRatio )   );

        qDebug() << m_poQtImage->width() << " x " << m_poQtImage->height();

        qDebug() << "paintEvent !!";
//    }
}

void DisplayThread::showEvent(QShowEvent *event)
{
    // Создать изображение Qt c таким же размером
     m_poQtImage = new QImage( QSize( this->width(), this->height() ), QImage::Format_RGB32 );
     m_poQtImage->fill( QColor(0x80, 0x80, 0x80));

    qDebug() << " showEvent ";
    qDebug() << m_poQtImage->width() << " x " << m_poQtImage->height();

}

void DisplayThread::detectAndDraw()
{
    // Список прямоугольников для лиц
    std::vector < cv::Rect >   faces;
    // Временный фрейм
    cv::Mat frame_gray;

    // Преобразовать цвета
    cv::cvtColor( *m_poCVMat, frame_gray, CV_RGB2GRAY  );
    // Выровнять гистограмму
    cv::equalizeHist( frame_gray, frame_gray );

    // Определить список лиц
    m_pCVFaceCascade.detectMultiScale(  frame_gray              ,
                                        faces                   ,
                                        1.1                     ,
                                        2                       ,
                                        0|CV_HAAR_SCALE_IMAGE   ,
                                        cv::Size(200, 200)      );

    qDebug() << "Faces number: " << faces.size();

    // Для каждого лица в списке
    for( size_t i = 0; i < faces.size(); i++ )
    {
        cv::rectangle(  *m_poCVMat  ,
                         faces[i]   ,
                         cv::Scalar( 0, 255, 0 ),
                         3                          );
/*
//        // Скопировать само лицо
//        cv::Mat faceROI = frame_gray( faces[i] );

//        // Зарезервировать массив глаз
//        std::vector<cv::Rect> eyes;
//        // Находим расположение глаз
//        m_pCVEyeCascade.detectMultiScale( faceROI, eyes, 1.1, 2, ( 0 | CV_HAAR_SCALE_IMAGE ) );
//        // Если в списке 2 глаза
//        if( eyes.size() == 2)
//        {
//            // Координаты центра лица
//            cv::Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );

//            // Нарисовать овал на изображении
//            cv::ellipse( m_oCVMat, center, cv::Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, cv::Scalar( 255, 0, 0 ), 2, 8, 0 );

//            // Для каждого глаза в списке
//            for( size_t j = 0; j < eyes.size(); j++ )
//            {
//                // Координты центра глаза
//                cv::Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2,
//                                      faces[i].y + eyes[j].y + eyes[j].height/2 );

//                // Расчитать радиус
//                int radius = cvRound( (eyes[j].width + eyes[j].height) * 0.25 );
//                cv::circle( m_oCVMat, eye_center, radius, cv::Scalar( 255, 0, 255 ), 3, 8, 0 );
//            }
//        }
*/
    }
}



