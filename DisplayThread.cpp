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
    m_poImageBuffer ( 0     ),
    m_poQtImage     ( 0     )
//  mDisplayWnd( aDisplayWnd )
{

    //m_oQtImage = QImage ( "../../Data/MyBAse/IMAG0313.jpg" );
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
 //   mDisplayWnd->Display( *aBuffer );

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
        // Закрасим новое изображение
        m_poQtImage->fill( Qt::red );
        // Вывидим размеры нового изображения
        qDebug() <<  " new QImage" << pvImage->GetWidth() << " x " <<  pvImage->GetHeight();

        // Привязываем данные фрейма к озображнию
        // res = m_poImageBuffer->Attach( (uchar*)m_poQtImage->bits(), m_poQtImage->byteCount() );
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
    // m_poQtImage = new QImage( QSize( this->width(), this->height() ), QImage::Format_RGB888 );
    // m_poQtImage->fill( Qt::green );

    m_poQtImage = new QImage( "../../Data/MyBAse/IMAG0313.jpg" );

    qDebug() << " showEvent ";
    qDebug() << m_poQtImage->width() << " x " << m_poQtImage->height();

}



