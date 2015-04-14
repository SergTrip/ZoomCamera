#include "zoomcameratestwidget.h"
#include "ui_zoomcameratestwidget.h"

#define DEFAULT_PAYLOAD_SIZE ( 1920 * 1080 * 2 )

ZoomCameraTestWidget::ZoomCameraTestWidget(QWidget *parent) :
    QWidget                     ( parent)
    , m_bNeedInit               ( false )
    , mDeviceWnd                ( NULL  )
    , mCommunicationWnd         ( NULL  )
    , mStreamParametersWnd      ( NULL  )
    , m_poDisplayThread         ( NULL  )
    , m_bAcquiringImages          ( false )
    , m_poAcquisitionStateManager  ( NULL  )
    , ui                        ( new Ui::ZoomCameraTestWidget )
{
    ui->setupUi( this );

    // Создать экземпляр потока изображения
    m_poDisplayThread = new DisplayThread( &m_oDisplay );

    // Обнулить указатели
    m_poDevice      = NULL;
    m_poStream      = NULL;
    m_poPipeline    = NULL;
}

ZoomCameraTestWidget::~ZoomCameraTestWidget()
{
    // Если был создан экземпляр потока
    if ( m_poDisplayThread != NULL )
    {
        // удалить экземпляр
        delete m_poDisplayThread;
        // поменять значение
        m_poDisplayThread = NULL;
    }

    //
    if ( m_poAcquisitionStateManager != NULL )
    {
        delete m_poAcquisitionStateManager;
        m_poAcquisitionStateManager = NULL;
    }

    // Если был создан экземпляр процесса
    if ( m_poDevice != NULL )
    {
        // Объект создан фабрикой класса
        PvDevice::Free( m_poDevice );
    }

    // Если был создан  экземпляр потока
    if ( m_poStream != NULL )
    {
        // Объект создан фабрикой класса
        PvStream::Free( m_poStream );
    }

    if ( m_poPipeline != NULL )
    {
        delete m_poPipeline;
        m_poPipeline = NULL;
    }

    delete ui;
}

void ZoomCameraTestWidget::StartStreaming()
{
    // Start threads
    m_poDisplayThread->Start        ( m_poPipeline, m_poDevice->GetParameters() );
    m_poDisplayThread->SetPriority  ( THREAD_PRIORITY_ABOVE_NORMAL              );

    // Start pipeline
    m_poPipeline->Start();
}

void ZoomCameraTestWidget::StopStreaming()
{
    // Stop display thread
    if ( m_poDisplayThread != NULL )
    {
        m_poDisplayThread->Stop( false );
    }

    if ( m_poPipeline != NULL )
    {
        // Stop stream thread
        if ( m_poPipeline->IsStarted() )
        {
            m_poPipeline->Stop();
        }
    }

    // Wait for display thread to be stopped
    if ( m_poDisplayThread != NULL )
    {
        m_poDisplayThread->WaitComplete();
    }
}

void ZoomCameraTestWidget::showEvent(QShowEvent *event)
{
    // Вызвать функцию базовго класса
    // CDialog::OnInitDialog();

    // Получить размеры клиенского окна
    // GetClientRect( m_oClientRect );

    // Отметить, что инициализация уже прошла
    m_bNeedInit = false;

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    // SetIcon(m_hIcon, true);			// Set big icon
    // SetIcon(m_hIcon, false);         // Set small icon

    // Переменная для хранения размера окна выводв потока
    //QRect displayRect;
    // Получаем размеры окна
    //GetDlgItem( IDC_DISPLAYPOS )->GetClientRect     ( &displayRect );
    // Преобразовуем в координаты экрана
    //GetDlgItem( IDC_DISPLAYPOS )->ClientToScreen    ( &displayRect );

    // Преобразовуем в какуто другую координатную базу
    //ScreenToClient( &displayRect );

    // Создать окно
    //m_oDisplay.Create             ( GetSafeHwnd(), 10000 );

    // Установить позицию и размеры
//    m_oDisplay.SetPosition        ( displayRect.left()  ,
//                                    displayRect.top()   ,
//                                    displayRect.Width() ,
//                                    displayRect.Height()    );
    // Установить цвет фона
//    m_oDisplay.SetBackgroundColor ( 0x80, 0x80, 0x80 );

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
    Q_ASSERT( aDI != NULL );

    // Еще раз проверить указатель
    if ( aDI == NULL )
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
    /*
    PvString lManufacturerStr   = aDI->GetVendorName();
    PvString lModelNameStr      = aDI->GetModelName();
    PvString lDeviceVersionStr  = aDI->GetVersion();

    // GigE Vision only parameters
    PvString lIPStr     = "N/A";
    PvString lMACStr    = "N/A";

    if ( lDIGEV != NULL )
    {
        // IP (GigE Vision only)
        lIPStr = lDIGEV->GetIPAddress();

        // MAC address (GigE Vision only)
        lMACStr = lDIGEV->GetMACAddress();
    }

    // USB3 Vision only parameters
    PvString lDeviceGUIDStr = "N/A";
    if ( lDIU3V != NULL )
    {
        // Device GUID (USB3 Vision only)
        lDeviceGUIDStr = lDIU3V->GetDeviceGUID();
    }

    // Device name (User ID)
    PvString lNameStr = aDI->GetUserDefinedName();

    // Устанавливаем информацию об устройстве
    mManufacturerEdit.SetWindowText   ( lManufacturerStr );
    mModelEdit.SetWindowText          ( lModelNameStr );
    mIPEdit.SetWindowText             ( lIPStr );
    mMACEdit.SetWindowText            ( lMACStr );
    mGUIDEdit.SetWindowText           ( lDeviceGUIDStr );
    mNameEdit.SetWindowText           ( lNameStr );

    // Get acquisition mode GenICam parameter
    PvGenEnum*  lMode           = lGenDevice->GetEnum( "AcquisitionMode" );
    int64_t     lEntriesCount   = 0;

    // Узнать количество доступных режимов работы камеры
    lMode->GetEntriesCount( lEntriesCount );

    // Заполняем выпадающих список режимов работы камеры
    // Fill acquisition mode combo box
    mModeCombo.ResetContent();
    // Для каждого режима
    for ( uint32_t i = 0; i < lEntriesCount; i++ )
    {
        const PvGenEnumEntry* lEntry = NULL;
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
}

void ZoomCameraTestWidget::Disconnect()
{

    if ( m_poDevice != NULL )
    {
        // Unregister all events of the parameters in the device's node map
        PvGenParameterArray *lGenDevice = m_poDevice->GetParameters();
        for ( uint32_t i = 0; i < lGenDevice->GetCount(); i++ )
        {
            lGenDevice->Get( i )->UnregisterEventSink( this );
        }
    }

    // Close all configuration child windows
    CloseGenWindow( &mDeviceWnd );
    CloseGenWindow( &mCommunicationWnd );
    CloseGenWindow( &mStreamParametersWnd );

    // If streaming, stop streaming
    StopStreaming();

    // Release acquisition state manager
    if ( m_poAcquisitionStateManager != NULL )
    {
        delete m_poAcquisitionStateManager;
        m_poAcquisitionStateManager = NULL;
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

    if ( m_poDevice != NULL )
    {
        // Объект создан фабрикой класса
        PvDevice::Free( m_poDevice );
        m_poDevice = NULL;
    }

    if ( m_poStream != NULL )
    {
        // Объект создан фабрикой класса
        PvStream::Free( m_poStream );
        m_poStream = NULL;
    }

    EnableInterface();
}

void ZoomCameraTestWidget::StartAcquisition()
{
    // Get payload size from device
    int64_t lPayloadSizeValue = DEFAULT_PAYLOAD_SIZE;

    if ( ( m_poDevice != NULL ) && m_poDevice->IsConnected() )
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
}

void ZoomCameraTestWidget::StopAcquisition()
{
    m_poAcquisitionStateManager->Stop();
}

//void ZoomCameraTestWidget::ShowGenWindow(PvGenBrowserWnd **aWnd, PvGenParameterArray *aParams, const CString &aTitle)
//{
//    if ( ( *aWnd ) != NULL )
//    {
//        if ( ( *aWnd )->GetHandle() != 0 )
//        {
//            CWnd lWnd;
//            lWnd.Attach( ( *aWnd )->GetHandle() );

//            // Window already visible, give it focus and bring it on top
//            lWnd.BringWindowToTop();
//            lWnd.SetFocus();

//            lWnd.Detach();
//            return;
//        }

//        // Window object exists but was closed/destroyed. Free it before re-creating
//        CloseGenWindow( aWnd );
//    }

//    // Create, assign parameters, set title and show modeless
//    ( *aWnd ) = new PvGenBrowserWnd;
//    ( *aWnd )->SetTitle( PvString( aTitle ) );
//    ( *aWnd )->SetGenParameterArray( aParams );
//    ( *aWnd )->ShowModeless( GetSafeHwnd() );
//}

void ZoomCameraTestWidget::CloseGenWindow(PvGenBrowserWnd **aWnd)
{
    // If the window object does not even exist, do nothing
    if ( ( *aWnd ) == NULL )
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
    ( *aWnd ) = NULL;
}

void ZoomCameraTestWidget::EnableInterface()
{
    // Устройство создано и подключено
    bool lConnected = ( m_poDevice != NULL ) && m_poDevice->IsConnected();

    // Настроить состояние элементов диалога
//    GetDlgItem( IDC_CONNECT_BUTTON          )-> EnableWindow( !lConnected );
//    GetDlgItem( IDC_DISCONNECT_BUTTON       )-> EnableWindow( lConnected );

//    GetDlgItem( IDC_COMMUNICATION_BUTTON    )-> EnableWindow( lConnected );
//    GetDlgItem( IDC_DEVICE_BUTTON           )-> EnableWindow( lConnected );
//    GetDlgItem( IDC_STREAMPARAMS_BUTTON     )-> EnableWindow( lConnected );

    bool lLocked = false;
    // Если класс состояния уже создан
    if ( m_poAcquisitionStateManager != NULL )
    {
        lLocked = m_poAcquisitionStateManager->GetState() == PvAcquisitionStateLocked;
    }

    // Установить состояние кнопок
//    mPlayButton.EnableWindow(   lConnected && !lLocked  );
//    mStopButton.EnableWindow(   lConnected && lLocked   );

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
    if ( !lResult.IsOK() || ( lFinder.GetSelected() == NULL ) )
    {
        return;
    }

    // Включить курсор ожидания
    //CWaitCursor lCursor;

    // Вызывть перерисовку диалога
    // UpdateWindow();

    // Получить информацию о выбранном устойстве
    const PvDeviceInfo* lDeviceInfo = lFinder.GetSelected();

    // Если информация не аустая
    if ( lDeviceInfo != NULL )
    {
        // Подключить устройство
        Connect( lDeviceInfo );
    }
    // Иначе
    else
    {
        // Сообщаем о проблемах
//        QMessageBox( "No device selected." , "Error",
//                     MB_OK | MB_ICONINFORMATION );
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
