#include "zoomcameratestwidget.h"
#include "ui_zoomcameratestwidget.h"

#define DEFAULT_PAYLOAD_SIZE ( 1920 * 1080 * 2 )

ZoomCameraTestWidget::ZoomCameraTestWidget(QWidget *parent) :
    QWidget                         ( parent    )
    , m_bNeedInit                   ( false     )
    , m_poDeviceCtrlWnd             ( 0         )
    , m_poCommunicationCtrlWnd      ( 0         )
    , m_poStreamParametersCtrlWnd   ( 0         )
    , m_bAcquiringImages            ( false     )
    , m_poAcquisitionStateManager   ( 0         )
    , ui                            ( new Ui::ZoomCameraTestWidget )
{
    ui->setupUi( this );

    // Настроить слайдер Zoom
    QSlider*    zoomSlider = ui->horizontalSlider_zoom;
    // Установить диапазон
    zoomSlider->setRange( 0, 16384);

    // Обнулить указатели
    m_poDevice      = 0;
    m_poStream      = 0;
    m_poPipeline    = 0;
}

ZoomCameraTestWidget::~ZoomCameraTestWidget()
{
    //
    if ( m_poAcquisitionStateManager != 0 )
    {
        delete m_poAcquisitionStateManager;
        m_poAcquisitionStateManager = 0;
    }

    // Если был создан экземпляр процесса
    if ( m_poDevice != 0 )
    {
        // Объект создан фабрикой класса
        PvDevice::Free( m_poDevice );
    }

    // Если был создан  экземпляр потока
    if ( m_poStream != 0 )
    {
        // Объект создан фабрикой класса
        PvStream::Free( m_poStream );
    }

    if ( m_poPipeline != 0 )
    {
        delete m_poPipeline;
        m_poPipeline = 0;
    }

    delete ui;
}

void ZoomCameraTestWidget::StartStreaming()
{

    (ui->widget)->Start        ( m_poPipeline, m_poDevice->GetParameters() );
    (ui->widget)->SetPriority  ( THREAD_PRIORITY_ABOVE_NORMAL              );

    // Start pipeline
    m_poPipeline->Start();

    qDebug() << "Pipeline started...";
}

void ZoomCameraTestWidget::StopStreaming()
{
    // Остановить поток отображения
    (ui->widget)->Stop( false );

    if ( m_poPipeline != 0 )
    {
        // Stop stream thread
        if ( m_poPipeline->IsStarted() )
        {
            m_poPipeline->Stop();
        }
    }

    // Ждать завершения всех операций
    ui->widget->WaitComplete();
}

void ZoomCameraTestWidget::OnParameterUpdate(PvGenParameter *aParameter)
{
    bool        bBufferResize = false;
    PvString    lName;

    // Получить имя параметра для изменения
    aParameter->GetName( lName );

    qDebug() << " ZoomCameraTestWidget::OnParameterUpdate" ;

    // Если параметр Zoom
    if ( lName == "Zoom"   )
    {
        qDebug() << " On Zoom Parameter Update ... " ;
    }

    // Если нужно изменить режим отображения и комбо бокс доступен
    if ( ( lName == "AcquisitionMode"   )
         /*&& ( mModeCombo.GetSafeHwnd() != 0 ) */)
    {
        bool lAvailable = false;
        bool lWritable  = false;

        // Проверить, доступен ли сейчас параметр
        aParameter->IsAvailable( lAvailable );
        // Если доступен
        if ( lAvailable )
        {
            // Доступен ли он для записи
            aParameter->IsWritable( lWritable );
        }

        // mModeCombo.EnableWindow( lAvailable && lWritable );

        PvGenEnum *lEnum = dynamic_cast<PvGenEnum *>( aParameter );

//        if ( lEnum != 0 )
//        {
//            int64_t lEEValue = 0;
//            lEnum->GetValue( lEEValue );

//            for ( int i = 0; i < mModeCombo.GetCount(); i++ )
//            {
//                DWORD_PTR lData = mModeCombo.GetItemData( i );
//                if ( lData == lEEValue )
//                {
//                    mModeCombo.SetCurSel( i );
//                    break;
//                }
//            }
//        }
    }

}

void ZoomCameraTestWidget::showEvent(QShowEvent *event)
{
    // Отметить, что инициализация уже прошла
    m_bNeedInit = false;

    EnableInterface();
}

void ZoomCameraTestWidget::closeEvent(QCloseEvent *event)
{
    // Make sure we cleanup before we leave
    Disconnect();
}

void ZoomCameraTestWidget::Connect(const PvDeviceInfo *aDI)
{
    // Если указатель рабочий
    Q_ASSERT( aDI != 0 );

    // Еще раз проверить указатель
    if ( aDI == 0 )
    {
        return;
    }

    // Привести идентификатор к идентификатору конкретного устройства
    const PvDeviceInfoGEV *lDIGEV = dynamic_cast<const PvDeviceInfoGEV *>( aDI );
    const PvDeviceInfoU3V *lDIU3V = dynamic_cast<const PvDeviceInfoU3V *>( aDI );

    // Just in case we came here still connected...
    Disconnect();

    // Device connection, packet size negotiation and stream opening
    PvResult lResult = PvResult::Code::NOT_CONNECTED;

    // Попробовать подключить устройство
    m_poDevice = PvDevice::CreateAndConnect( aDI, &lResult );
    // Если не удалось
    if ( !lResult.IsOK() )
    {
        // Отключить устройство
        Disconnect();
        // Выйти
        return;
    }

    // Создает поток (фабрика слассов)
    m_poStream = PvStream::CreateAndOpen( aDI->GetConnectionID(), &lResult );
    // Если не удаось
    if ( !lResult.IsOK() )
    {
        // Закрыть соединение
        Disconnect();
        // Вернуться
        return;
    }

    // Необходимо только для сетевых устройств
    // GigE Vision devices only connection steps
    if ( aDI->GetType() == PvDeviceInfoTypeGEV )
    {
        PvDeviceGEV* lDeviceGEV = static_cast<PvDeviceGEV *>( m_poDevice    );
        PvStreamGEV* lStreamGEV = static_cast<PvStreamGEV *>( m_poStream    );

        PvString lLocalIpAddress    = lStreamGEV->GetLocalIPAddress();
        uint16_t lLocalPort         = lStreamGEV->GetLocalPort();

        // Perform automatic packet size negotiation
        lDeviceGEV->NegotiatePacketSize();

        // Now that the stream is opened, set the destination on the device
        lDeviceGEV->SetStreamDestination( lLocalIpAddress, lLocalPort );
    }

    // Создаем канал
    m_poPipeline = new PvPipeline( m_poStream );

    // Register to all events of the parameters in the device's node map
    PvGenParameterArray* lGenDevice = m_poDevice->GetParameters();
    // Регистрация всех событий
    for ( uint32_t i = 0; i < lGenDevice->GetCount(); i++ )
    {
        lGenDevice->Get( i )->RegisterEventSink( this );
    }

    //**** Подготовка строк инфомации о подключаемом устройстве ****************
    PvString lManufacturerStr   = aDI->GetVendorName();
    PvString lModelNameStr      = aDI->GetModelName();
    PvString lDeviceVersionStr  = aDI->GetVersion();

    // GigE Vision only parameters
    PvString lIPStr     = "N/A";
    PvString lMACStr    = "N/A";

    if ( lDIGEV != 0 )
    {
        // IP (GigE Vision only)
        lIPStr = lDIGEV->GetIPAddress();

        // MAC address (GigE Vision only)
        lMACStr = lDIGEV->GetMACAddress();
    }

    // USB3 Vision only parameters
    PvString lDeviceGUIDStr = "N/A";
    if ( lDIU3V != 0 )
    {
        // Device GUID (USB3 Vision only)
        lDeviceGUIDStr = lDIU3V->GetDeviceGUID();
    }

    // Device name (User ID)
    PvString lNameStr = aDI->GetUserDefinedName();

    // Устанавливаем информацию об устройстве
//    mManufacturerEdit.SetWindowText   ( lManufacturerStr );
//    mModelEdit.SetWindowText          ( lModelNameStr );
//    mIPEdit.SetWindowText             ( lIPStr );
//    mMACEdit.SetWindowText            ( lMACStr );
//    mGUIDEdit.SetWindowText           ( lDeviceGUIDStr );
//    mNameEdit.SetWindowText           ( lNameStr );

    // Get acquisition mode GenICam parameter
    PvGenEnum*  lMode           = lGenDevice->GetEnum( "AcquisitionMode" );
    int64_t     lEntriesCount   = 0;

    // Узнать количество доступных режимов работы камеры
    lMode->GetEntriesCount( lEntriesCount );

    //*****  Заполняем выпадающих список режимов работы камеры
    /*
    // Fill acquisition mode combo box
    mModeCombo.ResetContent();
    // Для каждого режима
    for ( uint32_t i = 0; i < lEntriesCount; i++ )
    {
        const PvGenEnumEntry* lEntry = 0;
        lMode->GetEntryByIndex( i, &lEntry );

        if ( lEntry->IsAvailable() )
        {
            PvString lEEName;
            lEntry->GetName( lEEName );

            int64_t lEEValue;
            lEntry->GetValue( lEEValue );

            int lIndex = mModeCombo.AddString( lEEName.GetUnicode() );
            mModeCombo.SetItemData( lIndex, static_cast<DWORD_PTR>( lEEValue ) );
        }
    }

    // Set mode combo box to value currently used by the device
    int64_t lValue = 0;
    lMode->GetValue( lValue );
    for ( int i = 0; i < mModeCombo.GetCount(); i++ )
    {
        if ( lValue == mModeCombo.GetItemData( i ) )
        {
            mModeCombo.SetCurSel( i );
            break;
        }
    }
    */

    // *************************************************************************

    // НУЖНО УСТАНОВИТЬ РЕЖИМ ПО УМОЛЧАНИЮ !!!!

    // Create acquisition state manager
    m_poAcquisitionStateManager = new PvAcquisitionStateManager( m_poDevice, m_poStream );
    m_poAcquisitionStateManager->RegisterEventSink( this );

    m_bAcquiringImages = false;

    // Force an update on all the parameters on acquisition mode
    OnParameterUpdate( lMode );

    // Ready image reception
    StartStreaming();

    // Sync up UI
    EnableInterface();

    qDebug() << "Connecting done...";
}

void ZoomCameraTestWidget::Disconnect()
{

    if ( m_poDevice != 0 )
    {
        // Unregister all events of the parameters in the device's node map
        PvGenParameterArray *lGenDevice = m_poDevice->GetParameters();
        for ( uint32_t i = 0; i < lGenDevice->GetCount(); i++ )
        {
            lGenDevice->Get( i )->UnregisterEventSink( this );
        }
    }

    // Close all configuration child windows
    CloseGenWindow( &m_poDeviceCtrlWnd             );
    CloseGenWindow( &m_poCommunicationCtrlWnd      );
    CloseGenWindow( &m_poStreamParametersCtrlWnd   );

    // If streaming, stop streaming
    StopStreaming();

    // Release acquisition state manager
    if ( m_poAcquisitionStateManager != 0 )
    {
        delete m_poAcquisitionStateManager;
        m_poAcquisitionStateManager = 0;
    }

    // Reset device ID - can be called by the destructor when the window
    // no longer exists, be careful...
//    if ( GetSafeHwnd() != 0 )
//    {
//        mManufacturerEdit.SetWindowText ( _T( "" ) );
//        mModelEdit.SetWindowText        ( _T( "" ) );
//        mIPEdit.SetWindowText           ( _T( "" ) );
//        mMACEdit.SetWindowText          ( _T( "" ) );
//        mGUIDEdit.SetWindowText         ( _T( "" ) );
//        mNameEdit.SetWindowText         ( _T( "" ) );
//    }

    if ( m_poDevice != 0 )
    {
        // Объект создан фабрикой класса
        PvDevice::Free( m_poDevice );
        m_poDevice = 0;
    }

    if ( m_poStream != 0 )
    {
        // Объект создан фабрикой класса
        PvStream::Free( m_poStream );
        m_poStream = 0;
    }

    EnableInterface();
}

void ZoomCameraTestWidget::StartAcquisition()
{
    // Get payload size from device
    int64_t lPayloadSizeValue = DEFAULT_PAYLOAD_SIZE;

    if ( ( m_poDevice != 0 ) && m_poDevice->IsConnected() )
    {
        lPayloadSizeValue = m_poDevice->GetPayloadSize();
    }

    // If payload size is valid, force buffers re-alloc - better than
    // adjusting as images are coming in
    if ( lPayloadSizeValue > 0 )
    {
        m_poPipeline->SetBufferSize( static_cast<uint32_t>( lPayloadSizeValue ) );
    }

    // Never hurts to start streaming on a fresh pipeline/stream...
    m_poPipeline->Reset();

    // Reset stream statistics
    m_poStream->GetParameters()->ExecuteCommand( "Reset" );

    m_poAcquisitionStateManager->Start();

    qDebug() << "Acquisition strted...";
}

void ZoomCameraTestWidget::StopAcquisition()
{
    m_poAcquisitionStateManager->Stop();
}

void ZoomCameraTestWidget::ShowGenWindow(   PvGenBrowserWnd     **aWnd,
                                            PvGenParameterArray  *aParams,
                                            const QString        &aTitle    )
{
    // Если предан не пустой указатель
    if ( ( *aWnd ) != 0 )
    {
        // Если ???
        if ( ( *aWnd )->GetHandle() != 0 )
        {
            // Найти виджет с таким хендлом
            QWidget* lWnd = 0;
            lWnd = QWidget::find( (WId)(( *aWnd )->GetHandle()) );

            // Если виджет нашелся
            if( lWnd != 0)
            {
                lWnd->activateWindow();
                // Установить фокус
                // lWnd->setFocus();
                return;
            }
            else
            {
                qDebug() << "Can't fined exesting window..";
            }

        }

        // Window object exists but was closed/destroyed.
        // Free it before re-creating
        CloseGenWindow( aWnd );
    }

    // Create, assign parameters, set title and show modeless
    ( *aWnd ) = new PvGenBrowserWnd;

    ( *aWnd )->SetTitle             ( PvString( (char*)aTitle.data())    );
    ( *aWnd )->SetGenParameterArray ( aParams               );
    ( *aWnd )->ShowModeless         ( (HWND)(this->winId()) );
}

void ZoomCameraTestWidget::CloseGenWindow(PvGenBrowserWnd **aWnd)
{
    // If the window object does not even exist, do nothing
    if ( ( *aWnd ) == 0 )
    {
        return;
    }

    // If the window object exists and is currently created (visible), close/destroy it
    if ( ( *aWnd )->GetHandle() != 0 )
    {
        ( *aWnd )->Close();
    }

    // Finally, release the window object
    delete ( *aWnd );
    ( *aWnd ) = 0;
}

void ZoomCameraTestWidget::EnableInterface()
{
    // Устройство создано и подключено
    bool lConnected = ( m_poDevice != 0 ) && m_poDevice->IsConnected();

    // Кнопки отключения и подключения устройств
    ui->pushButton_Connect      ->setEnabled( !lConnected );
    ui->pushButton_Disconnect   ->setEnabled( lConnected );

    // Кнопкидоступа к окнам настройки
    ui->pushButton_communicationControl ->setEnabled( lConnected );
    ui->pushButton_deviceControl        ->setEnabled( lConnected );
    ui->pushButton_imageDtreamControl   ->setEnabled( lConnected );

    bool lLocked = false;
    // Если класс состояния уже создан
    if ( m_poAcquisitionStateManager != 0 )
    {
        lLocked = m_poAcquisitionStateManager->GetState() == PvAcquisitionStateLocked;
    }

    // Установить состояние кнопок управления воспроизведением
    ui->pushButton_Start      ->setEnabled( lConnected && !lLocked  );
    ui->pushButton_Stop       ->setEnabled( lConnected && lLocked  );

    // If not connected, disable the acquisition mode control. If enabled,
    // it will be managed automatically by events from the GenICam parameters
    if ( !lConnected || lLocked )
    {
//        mModeCombo.EnableWindow( FALSE );
    }
}

void ZoomCameraTestWidget::onButtonConnectSlot()
{
    // create a device finder wnd and open the select device dialog
    PvDeviceFinderWnd lFinder;

    // Показать стандартный диалог выбора устройства
    PvResult lResult = lFinder.ShowModal();

    // Если результатом была отмена
    if ( lResult.GetCode() == PvResult::Code::ABORTED )
    {
        // Выдать сообщение
        // QMessageBox( _T( "Invalid selection. Please select a device." ), _T("SimpleGUIApplication") );

        qDebug() << "Invalid selection. Please select a device." ;
        return;
    }

    // Если не ОК и ничего не выбрано
    if ( !lResult.IsOK() || ( lFinder.GetSelected() == 0 ) )
    {
        return;
    }

    // Включить курсор ожидания
    //CWaitCursor lCursor;

    // Вызывть перерисовку диалога
    // UpdateWindow();

    // Получить информацию о выбранном устойстве
    const PvDeviceInfo* lDeviceInfo = lFinder.GetSelected();

    // Если информация не пустая
    if ( lDeviceInfo != 0 )
    {
        // Подключить устройство
        Connect( lDeviceInfo );
    }
    // Иначе
    else
    {
        qDebug() << "No device selected." ;
        return;
    }
}

void ZoomCameraTestWidget::onButtonDisconnectSlot()
{
    // Поменять курсор на ожидание
    // CWaitCursor lCursor;
    // Отключить подключенное устройство
    Disconnect();
}

void ZoomCameraTestWidget::onButtonStartSlot()
{
    // Если устройство еще не подключено
    if ( !m_poDevice->IsConnected() )
    {
        // Ничего не делать
        return;
    }

    // Начать захват изображений
    StartAcquisition();

    // Обновить состояние интерфейса
    EnableInterface();
}

void ZoomCameraTestWidget::onButtonStopSlot()
{
    // Если устройство не подключено
    if ( !m_poDevice->IsConnected() )
    {
        // Ничего не делать
        return;
    }

    // Отметить, что изображения больше не захватываются
    m_bAcquiringImages = false;

    // Остановить захват
    StopAcquisition();

    // Обновить состояние интерфейса
    EnableInterface();
}

void ZoomCameraTestWidget::onButtonCommCtrlSlot()
{
    // Если устройство подключено
    if ( !m_poDevice->IsConnected() )
    {
        // Просто выйти
        return;
    }

    // Открыть окно настройки соединения
    ShowGenWindow(  &m_poCommunicationCtrlWnd               ,
                    m_poDevice->GetCommunicationParameters(),
                    "Communication Control"                      );
}

void ZoomCameraTestWidget::onButtonDevCtrlSlot()
{
    // Если устройство подключено
    if ( !m_poDevice->IsConnected() )
    {
        // Просто выйти
        return;
    }

    // Открыть окно настройки устройства
    ShowGenWindow(  &m_poDeviceCtrlWnd          ,
                    m_poDevice->GetParameters() ,
                    "Device Control"                );
}

void ZoomCameraTestWidget::onButtonImgStreamCtrlSlot()
{
    // Если устройство подключено
    if ( !m_poDevice->IsConnected() )
    {
        return;
    }
    // Открыть окно настройки потока изображений
    ShowGenWindow(  & m_poStreamParametersCtrlWnd   ,
                    m_poStream->GetParameters()     ,
                    "Image Stream Control"              );
}

void ZoomCameraTestWidget::onSliderZoomMoveSlot(int value)
{

    // Если устройство подключено
    if ( !m_poDevice->IsConnected() )
    {
        // Просто выйти
        return;
    }

    // Получаем массив параметров
    PvGenParameterArray* lGenDevice = m_poDevice->GetParameters();

    // Если указатель пуст
    if( lGenDevice == 0 )
    {
        // Не удалось получить параметры
        qDebug() << "Change Zoom: Can't reciev device parameters !!! ";
    }

    // Get Current Zomm Value from Camera
//    int CurrenZoom = lGenDevice->GetIntegerValue("Zoom", value);

//    switch( res.GetCode() )
//    {
//        case PvResult::Code::OK:
//            qDebug() << "Change Zoom: OK..... ";
//            qDebug() << "New Zoom Value: " << value;
//            break;

//        case PvResult::Code::NOT_FOUND:
//            qDebug() << "Change Zoom: Parameter nom found !!! ";
//            break;
//    }

    // Calculate a moving speed
    // int  nNewSpeedValue = value - CurrenZoom;

    // Set a moving speed

    // Start a move


    // Пробуем изменить значение
    PvResult res = lGenDevice->SetIntegerValue("Zoom", value);

    switch( res.GetCode() )
    {
        case PvResult::Code::OK:
            qDebug() << "Change Zoom: OK..... ";
            qDebug() << "New Zoom Value: " << value;
            break;

        case PvResult::Code::NOT_FOUND:
            qDebug() << "Change Zoom: Parameter nom found !!! ";
            break;
    }
}
