#ifndef ZOOMCAMERATESTWIDGET_H
#define ZOOMCAMERATESTWIDGET_H

#include <QWidget>
#include <QRect>
#include <QMessageBox>
#include <QDebug>

#include <PvDeviceFinderWnd.h>
#include <PvDevice.h>
#include <PvGenParameter.h>
#include <PvGenBrowserWnd.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvPipeline.h>
#include <PvDisplayWnd.h>
#include <PvAcquisitionStateManager.h>
#include <PvDeviceGEV.h>
#include <PvDeviceInfoGEV.h>
#include <PvDeviceInfoU3V.h>
#include <PvStreamGEV.h>

#include "DisplayThread.h"

namespace Ui {
class ZoomCameraTestWidget;
}

class ZoomCameraTestWidget : public QWidget,                    // Класс видежета
                                    PvGenEventSink,             // Сообщает об изменении параметров
                                    PvAcquisitionStateEventSink //
{
    Q_OBJECT

public:
    explicit ZoomCameraTestWidget(QWidget *parent = 0);
    ~ZoomCameraTestWidget();

    // Начать прием потока ???
    void StartStreaming();
    // Закончить прием потока
    void StopStreaming();

protected:
    // PvGenEventSink implementation
    // Вызывается при изменении параметров
    void OnParameterUpdate( PvGenParameter *aParameter );

    // Вызывается при отображении окна
    void showEvent  (QShowEvent * event);
    // Вызывается перед закрытием окна
    void closeEvent (QCloseEvent *event);

    // Процедура подключения
    void Connect    ( const PvDeviceInfo *aDI );
    // Процедура отключения
    void Disconnect ();

    // Начать захват изображений
    void StartAcquisition   ();
    // Остановить захват изображений
    void StopAcquisition    ();

    // Сформировать окно настроек
    void ShowGenWindow  ( PvGenBrowserWnd **aWnd, PvGenParameterArray *aParams, const QString &aTitle );
    // Закрыть стандартное окно
    void CloseGenWindow ( PvGenBrowserWnd **aWnd );

    // Меняет состояние элементов окна
    void EnableInterface();

public slots:
    // Нажатие кнопки Connect
    void onButtonConnectSlot();
    // Нажатие кнопки Disconnect
    void onButtonDisconnectSlot();

    // Нажатие кнопки Start
    void onButtonStartSlot();
    // Нажатие кнопки Stop
    void onButtonStopSlot();

    // Нажатие кнопки Communication Control
    void onButtonCommCtrlSlot();
    // Нажатие кнопки Device Control
    void onButtonDevCtrlSlot();
    // Нажатие кнопки Image Stream Control
    void onButtonImgStreamCtrlSlot();

    // Слот изменения значения увеличения
    void onSliderZoomMoveSlot( int value );

private:
    // Old Zoom Value
    int m_nOldZoomValue;

protected:
    // Флаг инициализации
    bool    m_bNeedInit;
    // Флаг процесса захвата изображения
    bool    m_bAcquiringImages;

    // Указатель на экземпляр устройства
    PvDevice*           m_poDevice;

    // Указатель на поток
    PvStream*           m_poStream;
    // Указатель на канал
    PvPipeline*         m_poPipeline;

    // Указатель на класс потока
 //   DisplayThread*      m_poDisplayThread;

    // Указатели на список окон
    // Device Control
    PvGenBrowserWnd*    m_poDeviceCtrlWnd;
    // Communication Control
    PvGenBrowserWnd*    m_poCommunicationCtrlWnd;
    // Image Stream Control
    PvGenBrowserWnd*    m_poStreamParametersCtrlWnd;

    // Объект окна для воспроизведения изображения
//    PvDisplayWnd        m_oDisplay;

    // Указатель на класс состояния получения изображения
    PvAcquisitionStateManager*  m_poAcquisitionStateManager;

private:
    Ui::ZoomCameraTestWidget*   ui;
};

#endif // ZOOMCAMERATESTWIDGET_H
