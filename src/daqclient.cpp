#include "daqclient.h"
#include "ui_daqclient.h"

#include <iostream>



using namespace std;

/*
 * Constructor: DAQClient
 *
 * DAQClient constructor,
 * - Signals and slots are connected,
 * - Fiji is launch,
 * - variables are intialized,
 * - previous values are loaded from last session,
 *
 * Paremeters:
 *  parent .- parent QWidget.
 *
 */
DAQClient::DAQClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DAQClient){

    ui->setupUi(this);
    this->setFixedSize(this->width(),this->height());
    setWindowTitle("XPAD DAQ v3.1.7");

    // Initializing socket
    m_tcp_socket = new QTcpSocket(this);

    // Creating DAQ Viewer
    m_viewer = new DAQViewer;

    //Connecting buttons with SLOTS
    connect(ui->pushButton_ConnectEthernetServer, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->pushButton_AskReady, SIGNAL(clicked()), this, SLOT(init()));
    connect(ui->pushButton_DigitalTest, SIGNAL(clicked()), this, SLOT(digitalTest()));
    connect(ui->pushButton_LoadDefaultConfigGValues, SIGNAL(clicked()), this, SLOT(loadDefaultConfigGValues()));
    connect(ui->pushButton_LoadConfigG, SIGNAL(clicked()), this, SLOT(loadConfigG()));
    connect(ui->pushButton_ITHLIncrease, SIGNAL(clicked()), this, SLOT(ITHLIncrease()));
    connect(ui->pushButton_ITHLDecrease, SIGNAL(clicked()), this, SLOT(ITHLDecrease()));
    connect(ui->pushButton_LoadFlatConfigL, SIGNAL(clicked()), this, SLOT(loadFlatConfigL()));
    connect(ui->pushButton_ShowDACL, SIGNAL(clicked()), this, SLOT(showDACL()));
    connect(ui->pushButton_Expose, SIGNAL(clicked()), this, SLOT(setExposureParameters()));
    connect(ui->toolButton_BrowseAcquisitionPath, SIGNAL(clicked()), this, SLOT(changeImageAcquisitionPath()));
    connect(ui->pushButton_CalibrationOTN, SIGNAL(clicked()), this, SLOT(calibrationOTN()));
    connect(ui->pushButton_CalibrationOTN_Pulse, SIGNAL(clicked()), this, SLOT(calibrationOTNPulse()));
    connect(ui->pushButton_CalibrationBEAM, SIGNAL(clicked()), this, SLOT(calibrationBEAM()));
    connect(ui->pushButton_LoadCalibrationFromFile, SIGNAL(clicked()), this, SLOT(loadCalibrationFromFile()));
    connect(ui->pushButton_SaveCalibrationToFile, SIGNAL(clicked()), this, SLOT(saveCalibrationToFile()));
    connect(ui->pushButton_ScanDACL, SIGNAL(clicked()), this, SLOT(scanDACL()));
    connect(ui->pushButton_ResetDetector, SIGNAL(clicked()), this, SLOT(resetDetector()));
    connect(ui->pushButton_Quit, SIGNAL(clicked()), this, SLOT(quit_DAQ()));
    connect(ui->pushButton_AbortProcess, SIGNAL(clicked()), this, SLOT(abortCurrentProcess()));
    connect(ui->pushButton_stackedWidget_Connectivity, SIGNAL(clicked()), this, SLOT(setStackedWidgetConnectivity()));;
    connect(ui->pushButton_stackedWidget_Expose, SIGNAL(clicked()), this, SLOT(setStackedWidgetExpose()));
    connect(ui->pushButton_stackedWidget_Calibration, SIGNAL(clicked()), this, SLOT(setStackedWidgetCalibration()));
    connect(ui->pushButton_stackedWidget_Advanced, SIGNAL(clicked()), this, SLOT(setStackedWidgetAdvanced()));
    connect(ui->pushButton_CreateWhiteImage, SIGNAL(clicked(bool)), this, SLOT(createWhiteImage()));
    connect(ui->pushButton_DeleteWhiteImage, SIGNAL(clicked(bool)), this, SLOT(deleteWhiteImage()));
    connect(ui->pushButton_GetWhiteImagesInDir, SIGNAL(clicked(bool)), this, SLOT(getWhiteImagesInDir()));
    connect(ui->pushButton_ReadTemperature, SIGNAL(clicked(bool)), this, SLOT(readTemperature()));
    connect(ui->comboBox_LoadConfigGlobalRegister, SIGNAL(currentIndexChanged(int)), this, SLOT(showConfigGDefaultValues()));
    connect(m_tcp_socket, SIGNAL(connected()), this, SLOT(connectionOpen()));
    connect(m_tcp_socket, SIGNAL(disconnected()), this, SLOT(connectionClose()));
    connect(m_tcp_socket, SIGNAL(readyRead()), this, SLOT(dataReceivedInTcp()));
    connect(m_viewer, SIGNAL(acquiredButtonPressed()), this, SLOT(setExposureParameters()));
    connect(m_viewer, SIGNAL(setContinuousAcquisitionON()), this, SLOT(setContinuousAcquisitionON()));
    connect(m_viewer, SIGNAL(setContinuousAcquisitionOFF()), this, SLOT(setContinuousAcquisitionOFF()));

    //Showing imXPAD LOGO
#ifdef _WIN32
    QString path1 = QApplication::applicationDirPath() + "/Resources/logo.gif";
    QPixmap pixmap(path1.toStdString().c_str(),"GIF");
#elif __linux__
    QPixmap pixmap("/opt/imXPAD/XPAD_DAQ/Resources/logo.gif","GIF");
#else
    QString path1 = QApplication::applicationDirPath() + "/../Resources/logo.gif";
    QPixmap pixmap(path1.toStdString().c_str(),"GIF");

#endif
    ui->label_logo->setPixmap(pixmap);
    ui->label_logo->show();

#ifdef _WIN32
    m_movie = new QMovie(QApplication::applicationDirPath() + "/Resources/loading.gif");
#elif __linux__
    m_movie = new QMovie("/opt/imXPAD/XPAD_DAQ/Resources/loading.gif");
#else
    m_movie = new QMovie(QApplication::applicationDirPath() + "/../Resources/loading.gif");
#endif

    //Setting FIJI for image visualisation and creating directories where images and calibration will be stored
    QDir dir;
    QString path = QDir::homePath() + "/XPAD_DAQ";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Images";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Calibration";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Calibration/Slow";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Calibration/Medium";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Calibration/Fast";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_DAQ/Calibration/Beam";
    dir.mkdir(path);

#ifdef __APPLE__
    //system("/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Misc...\" ,\"divide=Infinity run\" &");
    //QProcess *process = new QProcess(this);
    //QString program = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //QString program = QApplication::applicationDirPath() + "/../Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //m_process1->start(program);


#elif _WIN64
    //QProcess *process = new QProcess(this);
    //QDir::setCurrent("C:/Program Files (x86)/imXPAD/Fiji.app");
    //QString program = "ImageJ-win64.exe --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //process->start(program);

#elif _WIN32
    //QProcess *process = new QProcess(this);
    //QDir::setCurrent("C:/Program Files/imXPAD/Fiji.app");
    //QString program = "ImageJ-win32.exe --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //process->start(program);

#elif __linux__

    //system("/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Misc...\" ,\"divide=Infinity run\" &");
    //system("/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Misc...\" ,\"divide=Infinity run\" &");
    //m_process1 = new QProcess(this);
    //QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //m_process1->start(program);
    //m_process1->waitForFinished();
    //m_process2 = new QProcess(this);
    //program = "/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Misc...\" ,\"divide=Infinity run\"";
    //m_process2->start(program);
    //m_process2->waitForFinished();

#endif

    //Limits in lineEdit
    ui->lineEdit_Port->setValidator(new QIntValidator(1, 65535, this));
    ui->lineEdit_LoadConfigGValue->setValidator(new QIntValidator(0, 1000, this));
    ui->lineEdit_LoadFlatConfigLValue->setValidator(new QIntValidator(0, 63, this));
    ui->lineEdit_NumberImages->setValidator(new QIntValidator(0, 10000, this));
    ui->lineEdit_ExposeParamTime->setValidator(new QIntValidator(0, 65535, this));
    ui->lineEdit_ExpoParamWaitingTimeBetweenImages->setValidator(new QIntValidator(0, 65535, this));
    ui->lineEdit_ExposeParamTimeCalibBEAM->setValidator(new QIntValidator(0, 65535, this));
    ui->lineEdit_ITHLMaxCalibBEAM->setValidator(new QIntValidator(0, 200, this));

    //Setting initial colors and GUI Settings
    m_imXPAD_color = QColor(25, 156, 146, 255);
    m_imXPAD_green = QColor(125,165,58,255);
    m_imXPAD_red = QColor(220,40,40,255);
    m_imXPAD_orange = QColor(243,159,32,255);
    m_imXPAD_button = QColor(72,79,79,255);
    m_imXPAD_button_pushed = QColor(108,112,113,255);
    m_imXPAD_button_text = QColor(245,242,235,255);
    m_imXPAD_special_button = QColor(108,112,113,255);

    this->setAskReadyColor(m_imXPAD_red);
    this->setConnectEthernetServerColor(m_imXPAD_red);
    this->setStackedWidgetConnectivity();

    QPalette palette = ui->pushButton_AbortProcess->palette();
    palette.setColor(ui->pushButton_AbortProcess->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_AbortProcess->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_AbortProcess->setAutoFillBackground(true);
    ui->pushButton_AbortProcess->setPalette(palette);

    palette.setColor(ui->pushButton_Quit->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_Quit->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_Quit->setAutoFillBackground(true);
    ui->pushButton_Quit->setPalette(palette);
    this->setGUI(false);
    ui->label_ProcessingProgress->setVisible(false);
    //Initialisation of flags
    m_connection_type = 0;                           //connection type set to USB
    m_ethernet_connected_status = 0;                 //connection status to disconnected
    m_scan_DACL_val = 64;
    m_abort_process_state = false;
    m_waiting_animation_flag = false;
    m_continuous_acquisition = false;
    m_askready_flag = 0;
    m_flat_field_correction_flag = true;
    m_burst_output_file_path= "/opt/imXPAD/tmp_corrected/";
    ui->label_CalibrationPath->setText(QDir::homePath() + "/XPAD_DAQ/Calibration/");

    //Read the parameters stored in parameters.dat file
    this->connectionType();
    this->readParameters();

    m_viewer->show();
    QCoreApplication::processEvents();
}


/*
 * Destructor: DAQClient
 *
 * If connected to server, DAQClient will disconnect from server and will
 * save to file the configuration parameters before closing itself.
 *
 */
DAQClient::~DAQClient(){

    this->writeParameters();
    m_movie->deleteLater();
    //m_viewer->hide();
    //m_viewer->close();
    m_viewer->deleteLater();

    if(m_ethernet_connected_status == 1)
        this->connectToServer();

    m_tcp_socket->deleteLater();
    delete ui;

}

void DAQClient::closeEvent(QCloseEvent *){
    qApp->quit();
}

/*
 * Section: Connectivity Functions
 *
 */

void DAQClient::connectToServer(){
    if(m_tcp_socket->state() != QAbstractSocket::ConnectedState)
        m_tcp_socket->connectToHost(ui->lineEdit_IP->text(), ui->lineEdit_Port->text().toInt());
    else
        this->quit();
}

void DAQClient::connectionOpen(){

    //ui Functions:
    showMessage("Connected to Server");
    this->setConnectEthernetServerColor(m_imXPAD_green);             
    ui->lineEdit_Port->setEnabled(false);
    ui->lineEdit_IP->setEnabled(false);

    ui->pushButton_AskReady->setEnabled(true);
    ui->pushButton_ResetDetector->setEnabled(true);
    ui->pushButton_ConnectEthernetServer->setText("Disconnect from Ethernet Server");
    ui->radioButton_TransferImage->setChecked(true);

    ui->checkBox_showImageInVisualizer->setEnabled(true);

    //Set flags:
    m_ethernet_connected_status = 1;   
    m_viewer->setFirstTimeOn();

    //Control Functions:
    //Reads the prompt issue from first connection
    m_tcp_socket->readLine();
    QThread::msleep(150);
    this->getConnectionID();
    this->init();   
}

void DAQClient::connectionClose(){

    //ui functions:
    showMessage("Disconnected from Server");
    this->setConnectEthernetServerColor(m_imXPAD_red);
    this->setAskReadyColor(m_imXPAD_red);
    ui->lineEdit_Port->setEnabled(true);
    ui->lineEdit_IP->setEnabled(true);
    ui->label_DetectorType->setText("Connection type: ");
    ui->label_DetectorModel->setText("Detector model: ");

    this->setGUI(false);
    this->setGUIConnectivity(false);
    ui->checkBox_showImageInVisualizer->setEnabled(false);

    m_burst_number = 0;
    ui->lcdNumber_BurstNumber->display(m_burst_number);
    ui->lcdNumber_BurstNumber->update();
    m_viewer->setLCDNumber(m_burst_number);
    QApplication::processEvents();

    this->showWaitingAnimation(false);
    ui->pushButton_ConnectEthernetServer->setText("Connect to Ethernet Server");

    //Set flags
    m_waiting_animation_flag = false;
    m_ethernet_connected_status = 0;
}

/*
 * Section: XPAD Functions
 */

void DAQClient::init(){
    if (m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            this->getDetectorType();

            this->sendCommand("Init\n");
            this->receiveMessage();
            //this->evaluateAnswer();

            if (m_integer_answer == 0){
                this->getDetectorModel();
                this->getModuleMask();
                if (m_integer_answer > 0){
                    QString texto = "Module mask: 0x" + QString::number(m_module_mask,16);
                    this->showMessage(texto);
                }

                this->showMessage("Ask ready OK");
                this->setAskReadyColor(m_imXPAD_green);
                ui->pushButton_AskReady->setFocus();
                this->setGUI(true);
            } else {
                this->getDetectorModel();
                this->getModuleMask();
                if (m_integer_answer > 0){
                    QString texto = "Module mask: 0x" + QString::number(m_module_mask,16);
                    this->showError(texto);
                }

                this->setAskReadyColor(m_imXPAD_red);
                ui->pushButton_AskReady->setFocus();
                this->setGUI(false);
            }
        }
    }
}

void DAQClient::getModuleNumber(){
    if (m_ethernet_connected_status == 1){
        this->sendCommand("GetModuleNumber\n");
        this->receiveMessage();
        this->evaluateAnswer();
    }
}

void DAQClient::getChipNumber(){
    if (m_ethernet_connected_status == 1){
        this->sendCommand("GetChipNumber\n");
        this->receiveMessage();
        this->evaluateAnswer();
    }
}

void DAQClient::getModuleMask(){
    if (m_ethernet_connected_status == 1){
        this->sendCommand("GetModuleMask\n");
        this->receiveMessage();
        this->evaluateAnswer();
    }
}

void DAQClient::getChipMask(){
    if (m_ethernet_connected_status == 1){
        this->sendCommand("GetChipMask\n");
        this->receiveMessage();
        this->evaluateAnswer();
    }
}

void DAQClient::getDetectorType(){
    this->sendCommand("GetDetectorType\n");
    this->receiveMessage();
    QString message = "Connection type: " + m_string_answer;
    ui->label_DetectorType->setText(message);

    if (m_string_answer.contains("USB"))
        m_connection_type = 0;
    else
        m_connection_type = 1;

    this->connectionType();

}

void DAQClient::getDetectorModel(){
    this->sendCommand("GetDetectorModel\n");
    this->receiveMessage();
    QString message = "Detector model: " + m_string_answer;
    ui->label_DetectorModel->setText(message);

    m_detector_model = m_string_answer;

    this->adaptGUIForDetectorModel();
}

void DAQClient::getImageSize(){
    QString command = "GetImageSize\n";
    this->sendCommand(command);
    this->receiveMessage();
    this->evaluateAnswer();
}

void DAQClient::getConnectionID(){
    this->sendCommand("GetBurstNumber\n");
    this->receiveMessage();
    this->evaluateAnswer();
}

void DAQClient::askReady(){
    if (m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            this->sendCommand("AskReady\n");
            this->receiveMessage();
            this->evaluateAnswer();
            this->sendCommand("GetModuleMask\n");
            this->receiveMessage();
            this->evaluateAnswer();
        }

    }
}

void DAQClient::digitalTest(){

    if (m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString file_name = QDir::homePath() + "/XPAD_DAQ/Images/DigitalTest";
            m_expose_filename = file_name;

            ui->lineEdit_NumberImages->setText(QString::number(1));
            ui->checkBox_showImageInVisualizer->setChecked(true);
            ui->checkBox_GeometricalCorrections->setChecked(false);
            ui->checkBox_FlatFieldCorrections->setChecked(false);
            //ui->radioButton_Ascii->setChecked(true);
            ui->radioButton_TransferImage->setChecked(true);

            QString command = "SetGeometricalCorrectionFlag false\n";
            this->sendCommand(command);
            this->receiveMessage();

            m_viewer->setFirstTimeOn();

            ui->lcdNumber_ImageNumber->display(0);
            ui->lcdNumber_ImageNumber->update();
            m_viewer->setLCDNumber(0);
            QApplication::processEvents();

            command = "DigitalTest "  + ui->comboBox_DigitalTestMode->currentText() + "\n";
            this->sendCommand(command);
            this->showWaitingMessage("Digital test in progress...");
            this->showWaitingAnimation(true);
        }
    }
}

/*
 * Section: XPAD Configuration
 */

void DAQClient::loadDefaultConfigGValues(){

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            this->showMessage("Loading global configuration with values:\nAMPTP = 0, IMFP = 25, IOTA = 40, IPRE = 60, ITHL = 32, ITUNE = 120, IBUFF = 0");

            QString command = "LoadDefaultConfigG\n" ;
            this->sendCommand(command);
            this->receiveMessage();
            if (m_integer_answer == 0)
                this->readConfigG();
        }
    }
}

void DAQClient::readConfigG(){

    if(m_ethernet_connected_status == 1){

        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString command = "ReadConfigG " + ui->comboBox_LoadConfigGlobalRegister->currentText() + "\n";
            this->sendCommand(command);
            this->receiveMessage();
            this->evaluateAnswer();
        }
    }
}

void DAQClient::loadConfigG(){

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString message = "Loading " + ui->comboBox_LoadConfigGlobalRegister->currentText() + " with value: " + ui->lineEdit_LoadConfigGValue->text();
            this->showMessage(message);

            QString command = "LoadConfigG " + ui->comboBox_LoadConfigGlobalRegister->currentText() + " " + ui->lineEdit_LoadConfigGValue->text() + "\n" ;
            this->sendCommand(command);
            this->receiveMessage();
            this->readConfigG();
        }
    }
}

void DAQClient::ITHLIncrease(){
    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            this->sendCommand("ITHLIncrease\n");
            this->receiveMessage();

            ui->comboBox_LoadConfigGlobalRegister->setCurrentIndex(1);
            this->readConfigG();
        }
    }
}

void DAQClient::ITHLDecrease(){
    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            this->sendCommand("ITHLDecrease\n");
            this->receiveMessage();

            ui->comboBox_LoadConfigGlobalRegister->setCurrentIndex(1);
            this->readConfigG();
        }
    }
}

void DAQClient::loadFlatConfigL(){

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            unsigned short value = ui->lineEdit_LoadFlatConfigLValue->text().toInt();

            //The value is right shift three bits.
            value = value; //*8 + 1;

            if(m_ethernet_connected_status == 1){
                QString command = "LoadFlatConfigL " + QString::number(value) + "\n";
                this->sendCommand(command);
                this->receiveMessage();
                this->evaluateAnswer();
            }
        }
    }
}

void DAQClient::showDACL(){

    bool show_in_fiji = false;

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            this->sendCommand("ReadConfigL\n");
            QByteArray data_buffer = this->receiveParametersFile();
            this->receiveMessage();

            if (!data_buffer.isNull()){

                int line_number = m_module_number*IMG_LINE;
                int column_number = m_chip_number*IMG_COLUMN;


                QString file_name = QDir::homePath() + "/XPAD_DAQ/Images/DACL.dat";
                QFile file(file_name);

                if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                    QMessageBox::critical(this, tr("DAQ Client"),
                                          file_name + tr(" could not be open.\n\n")
                                          + tr("Local configuration COULD NOT BE SHOWN"));
                    this->showError("ERROR: File could not be open. Local configuration WAS NOT SHOWN");
                    return;
                }
                else{
                    //Written to ASCII file
                    QTextStream out(&file);
                    out << data_buffer;
                    file.flush();
                    file.close();

                    if (ui->checkBox_showImageInFiji->isChecked())
                        show_in_fiji = true;
                    else
                        ui->checkBox_showImageInFiji->setChecked(true);

                    ui->radioButton_Ascii->setChecked(true);
                    this->showImage(file_name, line_number, column_number);

                    QThread::sleep(1);

#ifdef __APPLE__
                    //imageJ_command = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\" &";
                    //system(imageJ_command.toStdString().c_str());

                    //QString program = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QString program = QApplication::applicationDirPath() + "/../Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QProcess *process1 = new QProcess(this);
                    process1->start(program);


#elif _WIN64
                    QDir::setCurrent("C:/Program Files (x86)/imXPAD/Fiji.app");
                    QString program = "ImageJ-win64.exe --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QProcess *process1 = new QProcess(this);
                    process1->start(program);

#elif _WIN32
                    QDir::setCurrent("C:/Program Files (x86)/imXPAD/Fiji.app");
                    QString program = "ImageJ-win32.exe --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QProcess *process1 = new QProcess(this);
                    process1->start(program);
#elif __linux__
                    //For RAW data
                    //imageJ_command = "/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\" &";
                    //system(imageJ_command.toStdString().c_str());
                    //imageJ_command = "/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\" &";
                    //system(imageJ_command.toStdString().c_str());

#ifdef __x86_64__

                    QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QProcess *process1 = new QProcess(this);
                    process1->start(program);

#else

                    QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Histogram\" ,\"bins=256 use x_min=0 x_max=62 y_max=Auto\"";
                    QProcess *process2 = new QProcess(this);
                    process2->start(program);

#endif
#endif

                    if (!show_in_fiji)
                        ui->checkBox_showImageInFiji->setChecked(false);

                    this->showMessage("Local configuration was readed SUCCESFULLY");
                }
            }
            else
                this->showError("ERROR: Transmition of data FAILED.");
        }
    }
}

/*
 * Section: XPAD Image Acquisition
 */

/*
 * Function: setExposureParameters
 *
 * Set the exposure parameters.
 *
 * These paremeters are given as integer in the following order:
 * - images number
 * - exposure time in microseconds
 * - waiting time between images in microseconds
 * - overflow time in microseconds
 * - input signals
 * - output signals
 * - geometrical corrections flag (0-disabled, 1-enabled)
 * - flat field corrections flag (0-disabled, 1-enabled)
 * - acquisition mode (0-standard, 1-burst DDR, 2-burst SSD)
 * - image transfert flag (0-images remain in the SERVER, 1-images are transfered via tcp socket)
 * - image file format (0-ASCII, 1-Binary)
 * - path where images will be stored in the SERVER
 *
 * Command sent to server:
 * - SetExposureParameter [Parameters]
 *
 * Example:
 * - SetExposureParameters 1 5000000 5000 4000 0 0 1 0 1 0 0 /opt/imXPAD/tmp_corrected/
 *
 */
void DAQClient::setExposureParameters(){
    unsigned int images_number = ui->lineEdit_NumberImages->text().toUInt();
    unsigned int time = ui->lineEdit_ExposeParamTime->text().toUInt();
    unsigned int waiting_time_between_images = ui->lineEdit_ExpoParamWaitingTimeBetweenImages->text().toInt();
    unsigned int overflow_time = 4000;
    unsigned short input_signal =ui->comboBox_InputSignal->currentIndex();
    unsigned short output_signal = ui->comboBox_OutputSignal->currentIndex();
    unsigned short geometrical_corrections_flag = ui->checkBox_GeometricalCorrections->isChecked();
    unsigned short flat_field_corrections_flag = ui->checkBox_FlatFieldCorrections->isChecked();
    unsigned short acquisition_mode = ui->comboBox_AcquisitionMode->currentIndex();
    unsigned short transfer_flag = ui->radioButton_TransferImage->isChecked();
    unsigned short image_format_file = 1;
    unsigned int number_stack_images = ui->lineEdit_StackImages->text().toUInt();

    unsigned int integer_time = 0;

    QMessageBox::StandardButton reply;
    double float_time;

    switch (ui->comboBox_ExpoParamTimeUnits->currentIndex()){
    case 0:
        integer_time = (unsigned int)(time*3600*1e6);       //hour to us
        float_time = time*3600*1e6; break;
    case 1:
        integer_time = (unsigned int)(time*60*1e6);        //min to us
        float_time = (double)(time*60*1e6); break;
    case 2:
        integer_time = (unsigned int)(time*1e6);           //s to us
        float_time = (double)(time*1e6); break;
    case 3:
        integer_time = (unsigned int)(time*1e3);           //ms to us
        float_time = (double)(time*1e3); break;
    case 4:
        integer_time = (unsigned int)(time);
        float_time = (double)(time);
    }

    unsigned int integer_waiting_time_between_images = 0;

    switch (ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->currentIndex()){
    case 0:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*3600*1e6); break;      //hour to us
    case 1:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*60*1e6); break;        //min to us
    case 2:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*1e6); break;           //s to us
    case 3:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*1e3); break;           //ms to us
    case 4:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images);
    }

    unsigned int total_time = integer_waiting_time_between_images + integer_time;

    if (total_time < 15000 && m_connection_type == 0){
        reply = QMessageBox::warning(this, tr("DAQ Client"),
                                     tr("Waiting time between images + Exposure time is smaller than 15 ms, This is not allowed in USB connections"), QMessageBox::Ok);
        return;
    }
    else if (float_time > 4294967296){
        reply = QMessageBox::warning(this, tr("DAQ Client"),
                                     tr("Exposure time cannot be higher than 1 hour -  11 minutes -  58 seconds"), QMessageBox::Ok);
        return;
    };

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")  && images_number>0){
            QString message = "SetExposureParameters " + QString::number(images_number) + " "
                    + QString::number(integer_time) + " " + QString::number((integer_waiting_time_between_images)) + " "
                    + QString::number(overflow_time) + " "
                    + QString::number(input_signal) + " " + QString::number(output_signal) + " "
                    + QString::number(geometrical_corrections_flag) + " " + QString::number(flat_field_corrections_flag) + " "
                    + QString::number(transfer_flag) + " " + QString::number(image_format_file) + " "
                    + QString::number(acquisition_mode) + " " + QString::number(number_stack_images) + " "
                    + m_burst_output_file_path + "\n";
            this->sendCommand(message);
            this->receiveMessage();
            this->evaluateAnswer();
            if (m_integer_answer == 0){
                this->getImageSize();
                this->startExposure();
            }
        }
    }
}

void DAQClient::startExposure(){
    QString file_path = ui->lineEdit_AcquiredImagePath->text();

    if(m_ethernet_connected_status == 1){

        QString file_name = file_path;

        if (file_name != NULL){

            //ui->lineEdit_AcquiredImagePath->setText(file_name);
            m_expose_filename = file_name;

            ui->lcdNumber_ImageNumber->display(0);
            ui->lcdNumber_ImageNumber->update();
            m_viewer->setLCDNumber(0);
            QApplication::processEvents();

            this->setGUI(false);

            this->sendCommand("StartExposure\n");
            this->showWaitingMessage("Images acquisition in progress...");
            this->showWaitingAnimation(true);

        }
        else
            QMessageBox::warning(this, tr("DAQ Client"),                                 tr("Please specify a file name for the new images to be acquired!"), QMessageBox::Ok);

    }
}

void DAQClient::scanDACL(){

    ui->checkBox_showImageInFiji->setChecked(false);
    ui->checkBox_GeometricalCorrections->setChecked(false);
    ui->checkBox_FlatFieldCorrections->setChecked(false);
    ui->lineEdit_NumberImages->setText(QString::number(1));
    ui->lineEdit_ExpoParamWaitingTimeBetweenImages->setText(QString::number(15));
    ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->setCurrentIndex(3);
    ui->radioButton_TransferImage->setChecked(true);

    unsigned int images_number = ui->lineEdit_NumberImages->text().toUInt();
    unsigned int time = ui->lineEdit_ExposeParamTime->text().toUInt();
    unsigned int waiting_time_between_images = ui->lineEdit_ExpoParamWaitingTimeBetweenImages->text().toInt();
    unsigned int overflow_time = 4000;
    unsigned short input_signal =ui->comboBox_InputSignal->currentIndex();
    unsigned short output_signal = ui->comboBox_OutputSignal->currentIndex();
    unsigned short geometrical_corrections = ui->checkBox_GeometricalCorrections->isChecked();
    unsigned short flat_field_corrections = ui->checkBox_FlatFieldCorrections->isChecked();
    unsigned short acquisition_mode = ui->comboBox_AcquisitionMode->currentIndex();
    unsigned short transfer_flag = ui->radioButton_TransferImage->isChecked();
    unsigned short image_format_file = ui->radioButton_Binary->isChecked();

    unsigned int integer_time = 0;

    switch (ui->comboBox_ExpoParamTimeUnits->currentIndex()){
    case 0:
        integer_time = (unsigned int)(time*3600*1e6); break;      //hour to us
    case 1:
        integer_time = (unsigned int)(time*60*1e6); break;        //min to us
    case 2:
        integer_time = (unsigned int)(time*1e6); break;           //s to us
    case 3:
        integer_time = (unsigned int)(time*1e3); break;           //ms to us
    case 4:
        integer_time = (unsigned int)(time);
    }

    unsigned int integer_waiting_time_between_images = 0;

    switch (ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->currentIndex()){
    case 0:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*3600*1e6); break;      //hour to us
    case 1:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*60*1e6); break;        //min to us
    case 2:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*1e6); break;           //s to us
    case 3:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images*1e3); break;           //ms to us
    case 4:
        integer_waiting_time_between_images = (unsigned int)(waiting_time_between_images);
    }

    unsigned int total_time = integer_waiting_time_between_images + integer_time;
    QMessageBox::StandardButton reply;
    if (total_time < 15000 && m_connection_type == 0){
        reply = QMessageBox::warning(this, tr("DAQ Client"),
                                     tr("Waiting time between images + Exposure time is smaller than 15 ms, This is not allowed in USB connections"), QMessageBox::Ok);
        return;
    }

    if (m_ethernet_connected_status == 1){

        this->getStatus();

        QString file_path = ui->lineEdit_AcquiredImagePath->text();

        QString file_name;
        if (ui->radioButton_Binary->isChecked())
            file_name = QFileDialog::getSaveFileName(this, tr("Save File"), file_path, tr("Binary image files (*.bin)"));
        else
            file_name = QFileDialog::getSaveFileName(this, tr("Save File"), file_path, tr("Text image files (*.dat)"));

        ui->lineEdit_AcquiredImagePath->setText(file_name);
        m_expose_filename = file_name;

        ui->lcdNumber_ImageNumber->display(0);
        ui->lcdNumber_ImageNumber->update();
        m_viewer->setLCDNumber(0);
        QApplication::processEvents();

        if (file_name != NULL){
            if (m_detector_status.contains("Idle")){
                QString message_box = "SetExposureParameters " + QString::number(images_number) + " "
                        + QString::number(integer_time) + " " + QString::number((integer_waiting_time_between_images)) + " "
                        + QString::number(overflow_time) + " "
                        + QString::number(input_signal) + " " + QString::number(output_signal) + " "
                        + QString::number(geometrical_corrections) + " " + QString::number(flat_field_corrections) + " "
                        + QString::number(transfer_flag) + " " + QString::number(image_format_file) + " "
                        + QString::number(acquisition_mode) + " " + m_burst_output_file_path + "\n";
                this->sendCommand(message_box);
                this->receiveMessage();
                if (m_integer_answer == 0){

                    m_scan_DACL_val = 0;

                    ui->lineEdit_LoadFlatConfigLValue->setText(QString::number(m_scan_DACL_val));
                    this->loadFlatConfigL();
                    QThread::usleep(20);
                    this->getImageSize();
                    this->startExposure();
                }
            }
        }
    }
}

/*
 * Section: XPAD Calibrations
 */

void DAQClient::calibrationOTN(){

    unsigned short OTNConfiguration = ui->comboBox_OTNConfiguration->currentIndex();

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            QMessageBox::warning(this, tr("DAQ Client"),
                                 tr("Please ensure that the detector in NOT EXPOSE to any X-rays."),QMessageBox::Ok);
            QMessageBox message_box;
            message_box.setText("OVER THE NOISE calibration takes several minutes to be performed.");
            message_box.setInformativeText("Do you want to continue?");
            message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            int ret = message_box.exec();

            switch(ret){
            case QMessageBox::Yes:{

                this->setGUI(false);

                QString message = "CalibrationOTN " + QString::number(OTNConfiguration) + "\n";
                this->sendCommand(message);
                this->showWaitingMessage("OVER THE NOISE calibration in progress...");
                this->showWaitingAnimation(true);
                break;
            }
            }
        }
    }
}

void DAQClient::calibrationOTNPulse(){

    unsigned short OTNConfiguration = ui->comboBox_OTNConfiguration->currentIndex();

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            QMessageBox::warning(this, tr("DAQ Client"),
                                 tr("Please ensure that the detector in NOT EXPOSE to any X-rays."),QMessageBox::Ok);
            QMessageBox message_box;
            message_box.setText("OVER THE NOISE calibration with PULSE takes several minutes to be performed.");
            message_box.setInformativeText("Do you want to continue?");
            message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            int ret = message_box.exec();

            switch(ret){
            case QMessageBox::Yes:{

                this->setGUI(false);

                QString message = "CalibrationOTNPulse " + QString::number(OTNConfiguration) + "\n";
                this->sendCommand(message);
                this->showWaitingMessage("OVER THE NOISE calibration with PULSE in progress...");
                this->showWaitingAnimation(true);
                break;
            }
            }
        }
    }
}

void DAQClient::calibrationBEAM(){

    unsigned int time =  ui->lineEdit_ExposeParamTimeCalibBEAM->text().toUInt();
    unsigned int ITHL_max = ui->lineEdit_ITHLMaxCalibBEAM->text().toUInt();
    unsigned int BEAM_configuration = ui->comboBox_BEAMConfiguration->currentIndex();

    unsigned int integer_time = 0;

    switch (ui->comboBox_ExpoParamTimeUnitsCalibBEAM->currentIndex()){
    case 0:
        integer_time = (unsigned int)(time*3600*1e6); break;      //hour to us
    case 1:
        integer_time = (unsigned int)(time*60*1e6); break;        //min to us
    case 2:
        integer_time = (unsigned int)(time*1e6); break;           //s to us
    case 3:
        integer_time = (unsigned int)(time*1e3); break;           //ms to us
    case 4:
        integer_time = (unsigned int)(time);
    }

    unsigned int total_waiting_time_min = ((ITHL_max - 20) + 64) * (integer_time/1000000)/60 + 2;

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            QMessageBox message_box;
            QString message = "BEAM calibration will take " + QString::number(total_waiting_time_min) + " minutes to be performed.\nPlease assure minimum 2000 hits/pixels using exposure time";
            message_box.setText(message);
            message_box.setInformativeText("Do you want to continue?");
            message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            int ret = message_box.exec();

            switch(ret){
            case QMessageBox::Yes:{

                this->setGUI(false);

                QString message = "CalibrationBEAM " + QString::number(integer_time) + " " + QString::number(ITHL_max) + " " + QString::number(BEAM_configuration) + "\n";
                this->sendCommand(message);
                this->showWaitingMessage("BEAM calibration in progress...");
                this->showWaitingAnimation(true);
                break;
            }
            }
        }
    }
}

void DAQClient::loadCalibrationFromFile(){
    QString image_file_name;
    QString file_path = ui->label_CalibrationPath->text();

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QMessageBox::warning(this, tr("DAQ Client"),
                                 tr("Please ensure that the detector in NOT EXPOSE to any X-rays."),QMessageBox::Ok);

            QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), file_path, tr("Local Configuration Files (*.cfg)"));

            if (file_name != NULL){

                //m_calibration_filename = file_name;
                ui->label_CalibrationPath->setText(QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName());

                image_file_name = QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName() + ".cfg";
                m_global_configuration_path = image_file_name;

                QFile file(image_file_name);

                if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                    QMessageBox::critical(this, tr("DAQ Client"),
                                          image_file_name + tr(" could not be open.\n\n")
                                          + tr("Global configuration file WAS NOT LOADED."));
                    this->showWarning("ERROR: File could not be open. Calibration WAS NOT LOADED properly.");
                    return;
                }
                else{

                    this->setGUI(false);
                    this->sendCommand("SetWhiteImage " + ui->lineEdit_FlatFieldFileBaseName->text() + "\n");
                    this->receiveMessage();

                    if (m_integer_answer != 0){
                        QMessageBox::warning(this, tr("DAQ Client"),
                                             tr(" White image file doesn't exists. ")
                                             + tr("Flat field correction will be disabled."));
                        ui->checkBox_FlatFieldCorrections->setEnabled(false);
                        ui->checkBox_FlatFieldCorrections->setChecked(false);
                        m_flat_field_correction_flag = false;

                    }else{
                        ui->checkBox_FlatFieldCorrections->setEnabled(true);
                        m_flat_field_correction_flag = true;
                    }

                    QByteArray data = file.readAll();
                    file.close();

                    this->sendCommand("LoadConfigGFromFile\n");
                    this->sendParametersFile(data);
                    this->showWaitingMessage("Loading global configuration in progress...");
                    this->showWaitingAnimation(true);

                }
            }
        }
    }
}

void DAQClient::saveCalibrationToFile(){
    QString image_file_name;
    QString file_path = ui->label_CalibrationPath->text();
    int ret = 0;

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), file_path, tr("Global configuration files (*.cfg)"));

            if (file_name != NULL){

                ui->label_CalibrationPath->setText(QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName());

                image_file_name = QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName() + ".cfg";
                m_global_configuration_path = image_file_name;

                QFile file(image_file_name);

                m_tcp_socket->flush();

                if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                    QMessageBox::critical(this, tr("DAQ Client"),
                                          image_file_name + tr(" could not be open.\n\n")
                                          + tr("Global configuration file WAS NOT SAVED."));
                    this->showError("ERROR: File could not be open. Calibration WAS NOT SAVED properly");
                    return;
                }
                else{
                    QString register_id;
                    int register_id_num;

                    QTextStream out(&file);

                    ///All registers are being read
                    for (unsigned short registro=0; registro < 7; registro++){
                        switch(registro){
                        case 0: register_id="AMPTP"; register_id_num = 31; break;
                        case 1: register_id="IMFP"; register_id_num = 59; break;
                        case 2: register_id="IOTA"; register_id_num = 60; break;
                        case 3: register_id="IPRE"; register_id_num = 61; break;
                        case 4: register_id="ITHL"; register_id_num = 62; break;
                        case 5: register_id="ITUNE"; register_id_num = 63; break;
                        case 6: register_id="IBUFF"; register_id_num = 64;
                        }

                        //Each register value is read from the detector
                        QString command = "ReadConfigG " + register_id + "\n";
                        this->sendCommand(command);
                        this->receiveMessage();

                        QString answer = m_string_answer;

                        QStringList data_list = answer.split(QRegExp("[ ;\t\n]"), QString::SkipEmptyParts);

                        int i=0;
                        int module_mask = 1;
                        while (i<data_list.length()){
                            if (!m_string_answer.isEmpty()){
                                //Register values are being stored in the file
                                out << module_mask << " ";
                                out << register_id_num << " ";
                                for(int count=0; count<8; count++){
                                    if (count > 0)
                                        out << data_list[i] << " ";
                                    i++;
                                }
                                out << endl;
                                module_mask = module_mask << 1;
                            }
                            else{
                                ret = -1;
                            }
                        }
                    }

                    file.flush();
                    file.close();

                    if (ret>=0){
                        this->showMessage("Global configuration was written to file SUCCESSFULLY");
                    }
                    else{
                        this->showError("ERROR: Global configuration FAILED to be written to file");
                    }

                }

                this->getStatus();

                if (m_detector_status.contains("Idle")){
                    image_file_name = QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName() + ".cfl";
                    m_local_configuration_path = image_file_name;

                    file.setFileName(image_file_name);

                    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                        QMessageBox::critical(this, tr("DAQ Client"),
                                              image_file_name + tr(" could not be open.\n\n")
                                              + tr("Global configuration file WAS NOT SAVED."));
                        this->showError("ERROR: File could not be open. Calibration WAS NOT SAVED properly");
                        return;
                    }
                    else{

                        QTextStream out(&file);

                        this->sendCommand("ReadConfigL\n");
                        QByteArray data = this->receiveParametersFile();
                        this->receiveMessage();

                        if (!data.isNull()){
                            out << data;
                            this->showMessage("Local configuration was written to file SUCCESSFULLY");
                        }
                        else{
                            this->showError("ERROR: Local configuration FAILED to be written to file");
                        }
                        file.flush();
                        file.close();
                    }
                }
            }
        }
    }
}

/*
 * Section: XPAD Status
 */

void DAQClient::getStatus(){
    if (m_ethernet_connected_status == 1){
        this->setAbortState(false);

        this->sendCommand("GetDetectorStatus\n");
        this->receiveMessage();

        m_detector_status = m_string_answer;       

        if (!m_detector_status.contains("Idle")){
            QString message = "WARNING: Detector BUSY, its status is: " + m_detector_status;
            this->showWarning(message);
            this->setAskReadyColor(m_imXPAD_orange);
        }
        else
            this->setAskReadyColor(m_imXPAD_green);
    }
}

/*
 * Section: XPAD Reset
 */

void DAQClient::resetDetector(){

    QString message = "Reseting detector...";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, tr("DAQ Client"),
                                 message + tr("\n\n")
                                 + tr("Are you sure you want to reset the detector?\nAfter reset you will have to load a new calibration file."), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes){
        if(m_ethernet_connected_status == 1){
            this->sendCommand("ResetDetector\n");
            this->receiveMessage();
            this->evaluateAnswer();
        }
    }
}


/*
 * Section: XPAD Exit
 */
void DAQClient::quit(){

    this->sendCommand("Exit\n");
}

void DAQClient::quit_DAQ(){

    QMessageBox message_box;
    QString message = "Closing XPAD DAQ...";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, tr("DAQ Client"),
                                 message + tr("\n\n")
                                 + tr("Are you sure you want to close the program?"), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes){
        if(m_ethernet_connected_status == 1)
            this->connectToServer();
        this->writeParameters();
        exit(0);
    }
}

/*
 * Section: XPAD Abort
 */
void DAQClient::abortCurrentProcess(){

    this->setAbortState(true);

    QTcpSocket *abortTcpSocket;                 //!< Socket use to cancel expose command.
    abortTcpSocket = new QTcpSocket(this);
    abortTcpSocket->connectToHost(ui->lineEdit_IP->text(),
                                  ui->lineEdit_Port->text().toInt());
    abortTcpSocket->write("AbortCurrentProcess\n");
    abortTcpSocket->waitForBytesWritten();
    abortTcpSocket->waitForReadyRead();
    abortTcpSocket->write("Exit\n");
    abortTcpSocket->waitForBytesWritten();
    abortTcpSocket->disconnectFromHost();
    abortTcpSocket->deleteLater();

    m_scan_DACL_val = 64;
    this->setContinuousAcquisitionOFF();
    m_viewer->setContinuousAcquisitionCheckBox(false);

    QMessageBox::warning(this, tr("DAQ Client"),
                         tr("ABORT was taken into account!"));
}

void DAQClient::setAbortState(bool state){
    m_abort_process_state = state;
}

bool DAQClient::getAbortState(){
    return m_abort_process_state;
}

/*
 * Section: Other Functions
 */

void DAQClient::connectionType(){

    if (m_connection_type == 0){

        ui->comboBox_OutputSignal->clear();
        ui->comboBox_OutputSignal->addItem("exposure busy");
        ui->comboBox_OutputSignal->addItem("shutter busy");
        ui->comboBox_OutputSignal->addItem("busy update overflow");
        ui->comboBox_OutputSignal->addItem("pixel counter enabled");
        ui->comboBox_OutputSignal->addItem("external gate");
        ui->comboBox_OutputSignal->addItem("exposure read done");
        ui->comboBox_OutputSignal->addItem("data transfer");
        ui->comboBox_OutputSignal->addItem("RAM ready image busy");
        ui->comboBox_OutputSignal->addItem("XPAD to local-DDR");
        ui->comboBox_OutputSignal->addItem("local-DDR to PC");
    }
    else if (m_connection_type == 1){           //Disable controls for PCI connectivity

        ui->comboBox_OutputSignal->clear();
        ui->comboBox_OutputSignal->addItem("exposure busy");
        ui->comboBox_OutputSignal->addItem("shutter busy");
        ui->comboBox_OutputSignal->addItem("busy update overflow");
        ui->comboBox_OutputSignal->addItem("pixel counter enabled");
        ui->comboBox_OutputSignal->addItem("external gate");
        ui->comboBox_OutputSignal->addItem("exposure read done");
        ui->comboBox_OutputSignal->addItem("data transfer");
        ui->comboBox_OutputSignal->addItem("RAM ready image busy");
        ui->comboBox_OutputSignal->addItem("XPAD to local-DDR");
        ui->comboBox_OutputSignal->addItem("local-DDR to PC");
    }
}

//************************/

//****PRIVATE SLOTS****/

//************************/
/*
 * Section: Server Functions
 */
void DAQClient::loadConfigLFromFile(){
    QString image_file_name;
    QString file_path = ui->label_CalibrationPath->text();

    QString file_name = file_path;

    if (file_name != NULL){

        image_file_name = QFileInfo(file_name).path() + "/" + QFileInfo(file_name).baseName() + ".cfl";
        m_global_configuration_path = image_file_name;

        QFile file(image_file_name);

        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::critical(this, tr("DAQ Client"),
                                  image_file_name + tr(" could not be open.\n\n")
                                  + tr("Local configuration file WAS NOT LOADED."));
            this->showError("ERROR: File could not be open. Calibration WAS NOT LOADED properly");
            return;
        }
        else{
            this->setGUI(false);

            QByteArray data = file.readAll();
            this->sendCommand("LoadConfigLFromFile\n");
            this->sendParametersFile(data);
            file.close();

            this->showWaitingMessage("Loading local configuration in progress...");
            this->showWaitingAnimation(true);
        }
    }
}

/*
 * Section: Message Functions
 */

void DAQClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this, tr("DAQ Client"),
                              tr("The host was not found.\n\n Please check the "
                                 "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::critical(this, tr("DAQ Client"),
                              tr("The connection was refused by the peer. "
                                 "\n\nMake sure the SERVER is running, and the PORT and IP are correct"));
        break;
    default:
        break;
        QMessageBox::critical(this, tr("DAQ Client"),
                              tr("The following error occurred: %1.")
                              .arg(m_tcp_socket->errorString()));
    }
    this->setGUI(false);
    this->setAskReadyColor(m_imXPAD_red);
    this->setConnectEthernetServerColor(m_imXPAD_red);
    ui->pushButton_ConnectEthernetServer->setText("Connect to Ethernet Server");

    m_tcp_socket->disconnectFromHost();
    this->connectionClose();
}

void DAQClient::showMessage(QString message){

    QString message_original = message;
    QString time = QDateTime::currentDateTime().toString("dd/MM/yyyy h:mm:ss\t");
    message = time + " " + message_original;

    //Change color to black
    QTextCharFormat text_format;
    text_format = ui->plainTextEdit_Message->currentCharFormat();
    text_format.setForeground(QBrush(m_imXPAD_green));
    ui->plainTextEdit_Message->setCurrentCharFormat(text_format);

    ui->plainTextEdit_Message->appendPlainText(message.toStdString().c_str());
    cout << message.toStdString().c_str() << endl;
    cout.flush();

    this->setAskReadyColor(m_imXPAD_green);

}

void DAQClient::showError(QString message){

    QString message_original = message;
    QString time = QDateTime::currentDateTime().toString("dd/MM/yyyy h:mm:ss\t");
    message = time + " " + message_original;

    //Change color to red
    QTextCharFormat text_format;
    text_format = ui->plainTextEdit_Message->currentCharFormat();
    text_format.setForeground(QBrush(m_imXPAD_red));
    ui->plainTextEdit_Message->setCurrentCharFormat(text_format);

    ui->plainTextEdit_Message->appendPlainText(message.toStdString().c_str());
    cout << message.toStdString().c_str() << endl;
    cout.flush();
}

void DAQClient::showWarning(QString message){

    QString message_original = message;
    QString time = QDateTime::currentDateTime().toString("dd/MM/yyyy h:mm:ss\t");
    message = time + " " + message_original;

    //Change color to orange
    QTextCharFormat text_format;
    text_format = ui->plainTextEdit_Message->currentCharFormat();
    text_format.setForeground(QBrush(m_imXPAD_orange));
    ui->plainTextEdit_Message->setCurrentCharFormat(text_format);

    ui->plainTextEdit_Message->appendPlainText(message.toStdString().c_str());
    cout << message.toStdString().c_str() << endl;
    cout.flush();
}

void DAQClient::showWaitingMessage(QString message){

    QString message_original = message;
    QString time = QDateTime::currentDateTime().toString("dd/MM/yyyy h:mm:ss\t");
    message = time + " " + message_original;

    //Change color to black
    QTextCharFormat text_format;
    text_format = ui->plainTextEdit_Message->currentCharFormat();
    text_format.setForeground(QBrush(m_imXPAD_button_text));
    ui->plainTextEdit_Message->setCurrentCharFormat(text_format);

    ui->plainTextEdit_Message->appendPlainText(message.toStdString().c_str());
    cout << message.toStdString().c_str() << endl;

}
/*
 * Section: Send/Receive Functions
 */

void DAQClient::sendCommand(QString command){

    if (m_tcp_socket->state()==QTcpSocket::ConnectedState){

        QThread::msleep(50);

        if(m_ethernet_connected_status == 1){

            m_tcp_socket->write(command.toStdString().c_str());
            m_tcp_socket->waitForBytesWritten();

            m_command_sent = command;
        }
        else{
            this->showError("ERROR: First connect to server");
        }

        m_tcp_socket->flush();
    }
    else{
        QMessageBox::information(this, tr("DAQ Client"),
                                 tr("ERROR SENDING commands to server."
                                    "\n\nDAQ will be closed. Please RESTART XPAD server."));
        exit(0);
    }
}

void DAQClient::receiveMessage(){

    this->showWaitingAnimation(true);
    this->setGUI(false);
    this->setGUIConnectivity(false);
    //this->setAskReadyColor(m_imXPAD_green);

    if(m_tcp_socket->state() == QTcpSocket::ConnectedState){

        while(1){
            QCoreApplication::processEvents();
            if ( m_tcp_socket->bytesAvailable() != 0){
                break;
            }
            m_tcp_socket->waitForReadyRead(16);;
        }

        m_buffer.clear();
        m_buffer = m_tcp_socket->readLine();

        //this->showError(m_buffer);

        do{
            //Receiving answer from XPAD server
            if (m_buffer.contains('*')){


                if ( m_buffer.contains('\"')){

                    int pos1 = m_buffer.indexOf("\"",0);
                    int pos2 = m_buffer.indexOf("\"",pos1 + 1);
                    m_buffer.chop(m_buffer.size() - pos2);
                    m_buffer = m_buffer.mid(pos1 + 1);

                    m_string_answer = m_buffer;

                    //this->showError(m_string_answer);
                }

                else{

                    int pos1 = m_buffer.indexOf("*",0);
                    int pos2 = m_buffer.indexOf("\n",pos1 + 1);
                    m_buffer.chop(m_buffer.size() - pos2);
                    m_buffer = m_buffer.mid(pos1 + 1);

                    m_integer_answer = m_buffer.toInt();

                    //this->showError(QString::number(m_integer_answer));
                }
            }

            //Receiving ERROR
            else if (m_buffer.contains('!')){

                int pos = m_buffer.indexOf("!",0);
                m_buffer = m_buffer.mid(pos + 2);
                m_buffer.chop(1);

                QString message;
                message.append(m_buffer);
                this->showError(message);
                message.clear();
            }

            //Receiving WARNING
            else if (m_buffer.contains('#')){

                int pos = m_buffer.indexOf("#",0);
                m_buffer = m_buffer.mid(pos + 2);
                m_buffer.chop(1);

                QString message;
                message.append(m_buffer);
                this->showWarning(message);
                message.clear();
            }

            m_tcp_socket->waitForReadyRead(10);
            m_buffer.clear();
            m_buffer = m_tcp_socket->readLine();


        } while (!m_buffer.contains('>') && m_tcp_socket->state()==QTcpSocket::ConnectedState);
        m_tcp_socket->flush();
    }
    else{
        QMessageBox::information(this, tr("DAQ Client"),
                                 tr("ERROR RECEIVING messages from server."
                                    "\n\nDAQ will be closed. Please RESTART XPAD server."));
        exit(0);
    }

    this->showWaitingAnimation(false);
    this->setGUI(true);
    this->setGUIConnectivity(true);
}

void DAQClient::evaluateAnswer(){
    int ret = m_integer_answer;
    QString message;
    QString sret = m_string_answer;

    this->setAskReadyColor(m_imXPAD_green);

    cout << "Command sent:\t-->\t" << m_command_sent.toStdString() << endl;

    if (m_command_sent.contains("GetModuleNumber")){
        if (ret > 0)
            m_module_number  =  ret;
    }

    else if (m_command_sent.contains("GetChipNumber")){
        if (ret > 0)
            m_chip_number  =  ret;
    }

    else if (m_command_sent.contains("GetModuleMask")){
        if (ret > 0){
            m_module_mask  =  ret;
        }
    }

    else if (m_command_sent.contains("GetChipMask")){
        if (ret > 0)
            m_chip_mask  =  ret;
    }

    else if (m_command_sent.contains("GetBurstNumber")){
        m_burst_number = m_integer_answer;
        ui->lcdNumber_BurstNumber->display(m_burst_number);
        ui->lcdNumber_BurstNumber->update();
    }

    else if (m_command_sent.contains("AskReady")){
        if (ret==0){
            this->showMessage("Ask ready OK");

            this->setAskReadyColor(m_imXPAD_green);
            ui->pushButton_AskReady->setFocus();
            this->setGUI(true);
        }
        else{
            this->setAskReadyColor(m_imXPAD_red);
            ui->pushButton_AskReady->setFocus();
            this->setGUI(false);
        }
    }

    else if (m_command_sent.contains("DigitalTest")){
        if(ret==0){

            message = "Digital Test image was saved in " + m_expose_filename;
            if (ui->radioButton_Binary->isChecked())
                message += ".bin";
            else
                message += ".dat";
            this->showMessage(message);
        }
    }

    else if (m_command_sent.contains("CreateWhiteImage")){
        if(ret==0)
            this->showMessage("White image was created SUCCESSFULLY");
    }

    else if (m_command_sent.contains("DeleteWhiteImage")){
        if(ret==0)
            this->showMessage("White image was deleted SUCCESSFULLY");
    }

    else if (m_command_sent.contains("GetImageSize")){
        QString rows = m_string_answer;
        QString columns = m_string_answer;
        int pos1 = m_string_answer.indexOf("x");
        rows.chop(m_string_answer.length() - pos1);
        columns = m_string_answer.mid(pos1+2, columns.size() - pos1+2);
        m_rows = rows.toUInt();
        m_columns = columns.toUInt();
        //cout << m_rows << " x " << m_columns << endl;
    }

    else if (m_command_sent.contains("LoadConfigGFromFile")){
        if(ret==0)
            this->showMessage("Global Configuration file was loaded SUCCESFULLY");
    }

    else if (m_command_sent.contains("LoadConfigLFromFile")){
        if(ret==0)
            this->showMessage("Local Configuration file was loaded SUCCESFULLY");
    }

    else if (m_command_sent.contains("ReadDetectorTemperature")){
        cout << sret.toStdString() << endl;
        if(!sret.isEmpty() && ret != -1){
            QStringList list;
            list = sret.split(QRegExp(" "), QString::SkipEmptyParts);

            ui->lineEdit_LoadConfigGValue_Chip1->setText(list[1]);
            ui->lineEdit_LoadConfigGValue_Chip2->setText(list[2]);
            ui->lineEdit_LoadConfigGValue_Chip3->setText(list[3]);
            ui->lineEdit_LoadConfigGValue_Chip4->setText(list[4]);
            ui->lineEdit_LoadConfigGValue_Chip5->setText(list[5]);
            ui->lineEdit_LoadConfigGValue_Chip6->setText(list[6]);
            ui->lineEdit_LoadConfigGValue_Chip7->setText(list[7]);

            int pos = sret.indexOf(list[1],0);
            sret = sret.mid(pos);

            QStringList modules = sret.split(QRegExp(";"));
            message = "Temperature read SUCCESSFULLY \n\nTemperature [??C] --> \n" + list[0] + " ";
            for (int i=0; i<modules.length(); i++)
                message.append(modules[i] + "\n");
            message.chop(2);
            this->showMessage(message);
        }
    }

    else if (m_command_sent.contains("ReadConfigG") || m_command_sent.contains("LoadConfigG")){
        cout << sret.toStdString() << endl;
        if(!sret.isEmpty() && ret != -1){
            QStringList list;
            list = sret.split(QRegExp(" "), QString::SkipEmptyParts);

            ui->lineEdit_LoadConfigGValue_Chip1->setText(list[1]);
            ui->lineEdit_LoadConfigGValue_Chip2->setText(list[2]);
            ui->lineEdit_LoadConfigGValue_Chip3->setText(list[3]);
            ui->lineEdit_LoadConfigGValue_Chip4->setText(list[4]);
            ui->lineEdit_LoadConfigGValue_Chip5->setText(list[5]);
            ui->lineEdit_LoadConfigGValue_Chip6->setText(list[6]);
            ui->lineEdit_LoadConfigGValue_Chip7->setText(list[7]);

            int pos = sret.indexOf(list[1],0);
            sret = sret.mid(pos);

            QStringList modules = sret.split(QRegExp(";"));
            message = "Global configuration read SUCCESSFULLY \n\nRegister "
                    + ui->comboBox_LoadConfigGlobalRegister->currentText()
                    + " --> \n" + list[0] + " ";
            for (int i=0; i<modules.length(); i++)
                message.append(modules[i] + "\n");
            message.chop(2);
            this->showMessage(message);
        }
    }

    else if(m_command_sent.contains("ITHLIncrease") || m_command_sent.contains("ITHLDecrease")){
        if(ret==0){

            if(m_command_sent.contains("ITHLIncrease"))
                message = "ITHL value was increased for all registers SUCCESSFULLY";
            else
                message = "ITHL value was decreased for all registers SUCCESSFULLY";
            this->showMessage(message);
        }
    }

    else if (m_command_sent.contains("LoadFlatConfigL")){
        if(ret==0){
            message = "Flat value : " + QString::number(ui->lineEdit_LoadFlatConfigLValue->text().toInt() * 8 + 1) + " = ("
                    + QString::number(ui->lineEdit_LoadFlatConfigLValue->text().toInt()) + " * 8) + 1 "
                    + " was loaded to local configuration SUCCESSFULLY";
            this->showMessage(message);
        }
    }
    else if (m_command_sent.contains("ReadConfigL")){
        if(ret==0)
            this->showMessage("Local configuration was saved to file SUCCESSFULLY");
    }


    else if (m_command_sent.contains("SetExposureParameters")){
        if(ret==0){
            this->showMessage("Exposure parameters were loaded SUCCESSFULLY");
        }
    }

    else if (m_command_sent.contains("StartExposure")){
        if(!ret)
            this->showMessage("Exposure finished SUCCESSFULLY");
    }

    else if (m_command_sent.contains("AbortExposure")){
        if(!ret)
            this->showMessage("Abort exposure applied with SUCCESS");
    }

    else if (m_command_sent.contains("CalibrationOTN")){
        if(ret==0){
            message = "Over-The-Noise calibration finished SUCCESSFULLY";
            this->showMessage(message);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this, tr("DAQ Client"),
                                         message + tr("\n\n")
                                         + tr("Do you want to SAVE calibration file before continue?"), QMessageBox::Save|QMessageBox::No);
            if (reply == QMessageBox::Save)
                this->saveCalibrationToFile();
        }
    }

    else if (m_command_sent.contains("CalibrationOTNPulse")){
        if(ret==0){
            message = "Over-The-Noise calibration with PULSE finished SUCCESSFULLY";
            this->showMessage(message);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this, tr("DAQ Client"),
                                         message + tr("\n\n")
                                         + tr("Do you want to SAVE calibration file before continue?"), QMessageBox::Save|QMessageBox::No);
            if (reply == QMessageBox::Save)
                this->saveCalibrationToFile();
        }
    }

    else if (m_command_sent.contains("CalibrationBEAM")){
        if(ret==0){
            message = "BEAM calibration finished SUCCESSFULLY";
            this->showMessage(message);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this, tr("DAQ Client"),
                                         message + tr("\n\n")
                                         + tr("Do you want to SAVE calibration file before continue?"), QMessageBox::Save|QMessageBox::No);
            if (reply == QMessageBox::Save)
                this->saveCalibrationToFile();
        }
    }

    else if (m_command_sent.contains("ResetDetector")){
        if(ret==0)
            this->showMessage("RESET applied SUCCESSFULLY");
    }
}

bool DAQClient::receiveImage(QString fileName, int imageNumber){

    quint32 data_size = 0;
    quint32 line_final_image = 0;
    quint32 column_final_image = 0;
    quint32 bytes_received = 0;

    unsigned int images_number = ui->lineEdit_NumberImages->text().toUInt();

    m_tcp_socket->read((char *)&data_size, sizeof(quint32));
    m_tcp_socket->read((char *)&line_final_image, sizeof(quint32));
    m_tcp_socket->read((char *)&column_final_image, sizeof(quint32));

    //cout << data_size << " " << line_final_image << " " << column_final_image << endl;

    char character = data_size & 0x000000FF;
    m_buffer.clear();
    m_buffer.resize(data_size);

    if(data_size > 0 && character != '*'){

        while(bytes_received < data_size){

            while(1){
                QCoreApplication::processEvents();
                if ( m_tcp_socket->bytesAvailable() != 0){
                    break;
                }
                m_tcp_socket->waitForReadyRead(16);
            }

            bytes_received += m_tcp_socket->read(m_buffer.data() + bytes_received, data_size - bytes_received);
            //cout << "\tReceived " << bytes_received << " out of " << data_size << " Bytes" << endl;
        }

        m_tcp_socket->write("\n");
        m_tcp_socket->waitForBytesWritten();

        QString image_file_name = fileName;

        if (m_scan_DACL_val<63)
            image_file_name +=  "_DACL_" + QString::number(m_scan_DACL_val+1);

        if (images_number > 1)
            image_file_name += "_" + QString::number(imageNumber+1);

        if (ui->radioButton_Binary->isChecked())
            image_file_name += ".bin";
        else
            image_file_name += ".dat";


        QFile file(image_file_name);



        if (ui->radioButton_Binary->isChecked()){
            if(file.open(QIODevice::WriteOnly)){
                file.write(m_buffer);
                file.flush();
                file.close();
            }else{
                QMessageBox::critical(this, tr("DAQ Client"),
                                      image_file_name + tr(" could not be open.\n\n")
                                      + tr("Image WAS NOT SAVED."));
                m_buffer.clear();
                return false;
            }
        }
        else{
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
                QTextStream out(&file);
                qint32 *values = new qint32[line_final_image*column_final_image];
                memcpy(values, m_buffer.data(), data_size);

                //Written to ASCII file
                for (quint32 j=0; j<line_final_image; j++){
                    for (quint32 i=0; i<column_final_image; i++)
                        out << values[j*column_final_image + i] << " ";
                    out << "\n";
                }
                file.flush();
                file.close();

                delete[] values;
            }else{
                QMessageBox::critical(this, tr("DAQ Client"),
                                      image_file_name + tr(" could not be open.\n\n")
                                      + tr("Image WAS NOT SAVED."));
                m_buffer.clear();
                return false;
            }
        }

        ui->lcdNumber_ImageNumber->display(imageNumber+1);
        ui->lcdNumber_ImageNumber->update();
        m_viewer->setLCDNumber(imageNumber+1);
        QApplication::processEvents();

        if(ui->checkBox_showImageInFiji->isChecked())
            this->showImage(image_file_name, line_final_image, column_final_image);

        if(ui->checkBox_showImageInVisualizer->isChecked()){
            m_viewer->show();
            m_viewer->printImage(column_final_image,line_final_image, m_buffer);
        }
        m_buffer.clear();
        return true;
    }
    else{
        /*QMessageBox::warning(this, tr("DAQ Client"),
                              tr("Abort received, acquisition stopped!"));*/
        QString data_size_value = "\n";
        m_tcp_socket->write(data_size_value.toStdString().c_str());
        m_tcp_socket->waitForBytesWritten();
        m_buffer.clear();
        return false;
    }
}

bool DAQClient::readImage(QString fileName, int imageNumber){

    unsigned int line_final_image = m_rows;
    unsigned int column_final_image = m_columns;

    //cout << m_rows << " " << m_columns << endl;

    unsigned int images_number = ui->lineEdit_NumberImages->text().toUInt();

    QString file_name = m_burst_output_file_path + "burst_" + QString::number(m_burst_number) + "_image_" + QString::number(imageNumber) + ".bin";

    QFile image(file_name);
    while(image.exists() == false);
    if(!image.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, tr("DAQ Client"),
                              file_name + tr(" could not be open."));
        return false;
    }

    m_buffer.clear();
    m_buffer = image.readAll();
    image.close();
    QFile::remove(file_name);


    QString image_file_name = fileName;

    if (m_scan_DACL_val<63)
        image_file_name +=  "_DACL_" + QString::number(m_scan_DACL_val+1);

    if (images_number > 1)
        image_file_name += "_" + QString::number(imageNumber+1);

    if (ui->radioButton_Binary->isChecked())
        image_file_name += ".bin";
    else
        image_file_name += ".dat";


    QFile file(image_file_name);



    if (ui->radioButton_Binary->isChecked()){
        if (file.open(QIODevice::WriteOnly)){
            file.write(m_buffer);
            file.flush();
            file.close();
        }else{
            QMessageBox::critical(this, tr("DAQ Client"),
                                  image_file_name + tr(" could not be open.\n\n")
                                  + tr("Image WAS NOT SAVED."));
            m_buffer.clear();
            return false;
        }
    }
    else{
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)){

            //Written to ASCII file
            QTextStream out(&file);
            QDataStream data(m_buffer);
            qint32 *values = new qint32[line_final_image*column_final_image];
            data.setByteOrder(QDataStream::LittleEndian);
            for (unsigned int i=0; i<line_final_image*column_final_image; i++)
                data >> values[i];
            for (unsigned int j=0; j<line_final_image; j++){
                for (unsigned int i=0; i<column_final_image; i++)
                    out << values[j*column_final_image + i] << " ";
                out << "\n";
            }
            delete[] values;
            file.flush();
            file.close();
        }else{
            QMessageBox::critical(this, tr("DAQ Client"),
                                  image_file_name + tr(" could not be open.\n\n")
                                  + tr("Image WAS NOT SAVED."));
            m_buffer.clear();
            return false;
        }
    }

    ui->lcdNumber_ImageNumber->display(imageNumber+1);
    ui->lcdNumber_ImageNumber->update();
    m_viewer->setLCDNumber(imageNumber+1);
    QApplication::processEvents();

    if(ui->checkBox_showImageInFiji->isChecked())
        this->showImage(image_file_name, line_final_image, column_final_image);

    if(ui->checkBox_showImageInVisualizer->isChecked()){
        m_viewer->show();
        m_viewer->printImage(column_final_image,line_final_image, m_buffer);
    }
    m_buffer.clear();
    return true;
}

QByteArray DAQClient::receiveParametersFile(){

    quint32 data_size = 0;
    quint32 bytes_received = 0;
    QByteArray data;

    m_tcp_socket->waitForReadyRead();

    QDataStream in(m_tcp_socket);
    in.setByteOrder(QDataStream::LittleEndian);

    cout << "Receiving parameters file" << endl;

    in >> data_size;

    char character = data_size & 0x000000FF;

    //this->showMessage(QString::number(data_size));
    m_tcp_socket->read(sizeof(quint32));

    if (data_size > 0 && data_size < data.MaxSize && character != '*'){
        quint32 bytes_received_before = 0;
        while(bytes_received < data_size){
            while(1){
                QCoreApplication::processEvents();
                if ( m_tcp_socket->bytesAvailable() != 0){
                    break;
                }
                m_tcp_socket->waitForReadyRead(16);;
            }
            data.append(m_tcp_socket->read((data_size - bytes_received)));
            bytes_received = data.size();
            bytes_received_before = bytes_received;
            cout << "\tReceived " << bytes_received << " out of " << data_size << " Bytes" << endl;
        }

        QString data_size_value = QString::number(data_size) + "\n";
        m_tcp_socket->write(data_size_value.toStdString().c_str());
        m_tcp_socket->waitForBytesWritten();

        return data;
    }else
        return NULL;

}

void DAQClient::sendParametersFile(QByteArray data){

    QByteArray output_buffer;

    QDataStream out(&output_buffer, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out << data;

    m_tcp_socket->write(output_buffer);
    m_tcp_socket->flush();
    m_tcp_socket->waitForBytesWritten();

    m_tcp_socket->waitForReadyRead();
    m_tcp_socket->readLine();
}

void DAQClient::dataReceivedInTcp(){
    if (m_waiting_animation_flag == true){

        int numImages = ui->lineEdit_NumberImages->text().toUInt();
        QString file_name = m_expose_filename;

        if(m_command_sent.contains("StartExposure") || m_command_sent.contains("DigitalTest")){

            // Vars
            //timeval start, end;
            //double diff, tg1, tg2;

            // Timing
//#ifdef __linux__
            //gettimeofday(&start, NULL);
            //tg1 = start.tv_sec + start.tv_usec / 1000000.0;
//#endif

            if(ui->radioButton_TransferImage->isChecked()){

                bool receivedImageState = true;

                for(int i=0; i<numImages; i++){

                    if (i>0){

                        while(1){
                            QCoreApplication::processEvents();
                            if ( m_tcp_socket->bytesAvailable() != 0){
                                break;
                            }
                            m_tcp_socket->waitForReadyRead(16);
                        }
                    }

                    receivedImageState = this->receiveImage(file_name, i);

                    if (receivedImageState == false){
                        break;
                    }

                }

                this->showWaitingAnimation(false);

            }else{

                for(int i=0; i<numImages; i++)
                    if (!this->getAbortState())
                        this->readImage(file_name, i);
                    else
                        break;

                this->showWaitingAnimation(false);
                QApplication::processEvents();
            }

            this->receiveMessage();
            this->evaluateAnswer();

//#ifdef __linux__
            // Reporting Statistics
            //gettimeofday(&end, NULL);
            //tg2 = end.tv_sec + end.tv_usec / 1000000.0;
            //diff = tg2 - tg1;
            //printf("Transmission time is: %f s\n", diff);
//#endif

            if (m_scan_DACL_val<63){
                m_scan_DACL_val++;
                ui->lineEdit_LoadFlatConfigLValue->setText(QString::number(m_scan_DACL_val));
                this->loadFlatConfigL();
                QThread::usleep(20);
                this->startExposure();
            }

            this->setGUI(true);
            this->setAbortState(false);

            if (m_continuous_acquisition)
                this->startExposure();
        }

        else if (m_command_sent.contains("LoadConfigGFromFile") || m_command_sent.contains("LoadConfigLFromFile")){
            this->showWaitingAnimation(false);

            if (this->getAbortState() == false){
                this->receiveMessage();
                this->evaluateAnswer();
                this->setGUI(true);
                this->setAbortState(false);
                if (m_command_sent.contains("LoadConfigGFromFile") && m_integer_answer == 0)
                    this->loadConfigLFromFile();
            }
            else{
                this->receiveMessage();
                this->setGUI(true);
                this->setAbortState(false);
            }

        }


        else if (m_command_sent.contains("CalibrationBEAM") || m_command_sent.contains("CalibrationOTNPulse") || m_command_sent.contains("CalibrationOTN")){
            this->showWaitingAnimation(false);

            this->receiveMessage();
            this->evaluateAnswer();
            this->setGUI(true);
            this->setAbortState(false);
        }

        else if (m_command_sent.contains("CreateWhiteImage")){
            this->showWaitingAnimation(false);

            this->receiveMessage();
            this->evaluateAnswer();
            this->setGUI(true);
            this->setAbortState(false);

            QMessageBox::warning(this, tr("DAQ Client"),
                                 tr("In order to use the new white image, please reload a calibration file."));
        }
    }
}

/*
 * Section: Display Images
 */

void DAQClient::showImage(QString fileName, quint32 lineNumber, quint32 columnNumber){
    //QString imageJ_command;

#ifdef __APPLE__

    if(ui->checkBox_showImageInFiji->isChecked()){
        if (ui->radioButton_Binary->isChecked()){
            //QProcess *process = new QProcess(this);
            //QString program = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Text Image... \" ,\"open=" + fileName + "\"";
            QString program = QApplication::applicationDirPath() + "/../Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Raw...\" ,\"open=" + fileName + " image=[32-bit Unsigned] width=" + QString::number(columnNumber) + " height=" + QString::number(lineNumber) + " offset=0 number=1 gap=0 little-endian\"";
            QProcess *process1 = new QProcess;
            process1->start(program);
        }
        else if (ui->radioButton_Ascii->isChecked()){
            //QProcess *process = new QProcess(this);
            //QString program = "/Applications/Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Text Image... \" ,\"open=" + fileName + "\"";
            QString program = QApplication::applicationDirPath() + "/../Fiji.app/Contents/MacOS/ImageJ-macosx --no-splash --run \"Text Image... \" ,\"open=" + fileName + "\"";
            QProcess *process1 = new QProcess;
            process1->start(program);
            cout << fileName.toStdString().c_str() << endl;
        }
    }

#elif _WIN64
    if(ui->checkBox_showImageInFiji->isChecked()){
        if(ui->radioButton_Binary->isChecked()){
            //QProcess *process = new QProcess(this);
            QDir::setCurrent("C:/Program Files (x86)/imXPAD/Fiji.app");
            QString program = "ImageJ-win64.exe --no-splash --run \"Raw...\" ,\"open=" + fileName + " image=[32-bit Unsigned] width=" + QString::number(columnNumber) + " height=" + QString::number(lineNumber) + " offset=0 number=1 gap=0 little-endian";
            fileName.replace("/","\\\\");
            QProcess *process1 = new QProcess;
            process1->start(program);
        }
        else if (ui->radioButton_Ascii->isChecked()){
            //QProcess *process = new QProcess(this);
            QDir::setCurrent("C:/Program Files (x86)/imXPAD/Fiji.app");
            QString program = "ImageJ-win64.exe --no-splash --run \"Text Image... \" ,\"open=" + fileName;
            fileName.replace("/","\\\\");
            QProcess *process1 = new QProcess;
            process1->start(program);
        }
    }

#elif _WIN32
    if(ui->checkBox_showImageInFiji->isChecked()){
        if(ui->radioButton_Binary->isChecked()){
            //QProcess *process = new QProcess(this);
            QDir::setCurrent("C:/Program Files/imXPAD/Fiji.app");
            QString program = "ImageJ-win32.exe --no-splash --run \"Raw...\" ,\"open=" + fileName + " image=[32-bit Unsigned] width=" + QString::number(columnNumber) + " height=" + QString::number(lineNumber) + " offset=0 number=1 gap=0 little-endian";
            fileName.replace("/","\\\\");
            QProcess *process1 = new QProcess;
            process1->start(program);
        }
        else if (ui->radioButton_Ascii->isChecked()){
            //QProcess *process = new QProcess(this);
            QDir::setCurrent("C:/Program Files/imXPAD/Fiji.app");
            QString program = "ImageJ-win32.exe --no-splash --run \"Text Image... \" ,\"open=" + fileName;
            fileName.replace("/","\\\\");
            QProcess *process1 = new QProcess;
            process1->start(program);
        }
    }
#elif __linux__

    if(ui->checkBox_showImageInFiji->isChecked()){
        if (ui->radioButton_Binary->isChecked()){
#ifdef __x86_64__
            QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Raw...\" ,\"open=" + fileName + " image=[32-bit Signed] width=" + QString::number(columnNumber) + " height=" + QString::number(lineNumber) + " offset=0 number=1 gap=0 little-endian\"";
            QProcess *process1 = new QProcess;
            process1->start(program);

#else

            QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Raw...\" ,\"open=" + fileName + " image=[32-bit Signed] width=" + QString::number(columnNumber) + " height=" + QString::number(lineNumber) + " offset=0 number=1 gap=0 little-endian\"";
            QProcess *process2 = new QProcess;
            process2->start(program);
#endif
        }
        else if (ui->radioButton_Ascii->isChecked()){
#ifdef __x86_64__
            QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux64 --no-splash --run \"Text Image... \" ,\"open=" + fileName + "\"";
            QProcess *process1 = new QProcess;
            process1->start(program);

#else

            QString program = "/opt/imXPAD/Fiji.app/ImageJ-linux32 --no-splash --run \"Text Image... \" ,\"open=" + fileName + "\"";
            QProcess *process2 = new QProcess;
            process2->start(program);

#endif
        }
    }
#endif

}

void DAQClient::setContinuousAcquisitionON(){
    m_continuous_acquisition = true;
}

void DAQClient::setContinuousAcquisitionOFF(){
    m_continuous_acquisition = false;
}

/*
 *
 * Section: Setting Colors
 *
 */

void DAQClient::createWhiteImage(){

    unsigned int time = ui->lineEdit_ExposeParamTime->text().toUInt();
    unsigned int integer_time = 0;

    switch (ui->comboBox_ExpoParamTimeUnits->currentIndex()){
    case 0:
        integer_time = (unsigned int)(time*3600*1e6); break;      //hour to us
    case 1:
        integer_time = (unsigned int)(time*60*1e6); break;        //min to us
    case 2:
        integer_time = (unsigned int)(time*1e6); break;           //s to us
    case 3:
        integer_time = (unsigned int)(time*1e3); break;           //ms to us
    case 4:
        integer_time = (unsigned int)(time);
    }


    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            if (!ui->lineEdit_FlatFieldFileBaseName->text().isEmpty()){

                QMessageBox message_box;
                message_box.setText("Please ensure that detector is EXPOSED to a FLAT X-ray beam with at least 1000 hits/pixel");
                message_box.setInformativeText("Do you want to continue?");
                message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                message_box.setDefaultButton(QMessageBox::Yes);
                int ret = message_box.exec();
                switch(ret){
                case QMessageBox::Yes:{
                    QString message = "SetExposureTime " + QString::number(integer_time);
                    this->sendCommand(message);
                    this->receiveMessage();

                    if (m_integer_answer == 0){
                        this->getImageSize();
                        message = "CreateWhiteImage " + ui->lineEdit_FlatFieldFileBaseName->text() + "\n";
                        this->sendCommand(message);
                        this->showWaitingMessage("Images acquisition in progress...");
                        this->showWaitingAnimation(true);

                    }

                }
                }
            }else
                QMessageBox::warning(this, tr("DAQ Client"),
                                     tr("Please specify a file name for the new White Image!"), QMessageBox::Ok);
        }
    }
}

void DAQClient::deleteWhiteImage(){

    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            QMessageBox message_box;
            message_box.setText("White image: " + ui->lineEdit_FlatFieldFileBaseName->text() + " will be deleted.");
            message_box.setInformativeText("Do you want to continue?");
            message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message_box.setDefaultButton(QMessageBox::Yes);
            int ret = message_box.exec();

            switch(ret){
            case QMessageBox::Yes:{

                QString message = "DeleteWhiteImage " + ui->lineEdit_FlatFieldFileBaseName->text() + "\n";
                this->sendCommand(message);
                this->receiveMessage();
                this->evaluateAnswer();
            }
            }
        }
    }
}

void DAQClient::getWhiteImagesInDir(){
    if(m_ethernet_connected_status == 1){
        this->getStatus();

        if (m_detector_status.contains("Idle")){
            QString message = "GetWhiteImagesInDir\n";
            this->sendCommand(message);
            this->receiveMessage();
            this->showMessage(m_string_answer);
        }
    }
}

/*
 *
 * Section: Read Temperature
 *
 */
void DAQClient::readTemperature(){
    if(m_ethernet_connected_status == 1){

        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString command = "ReadDetectorTemperature\n";
            this->sendCommand(command);
            this->receiveMessage();
            this->evaluateAnswer();
        }
    }
}

/*
 * Section: GIF Animations Functions
 */

void DAQClient::showWaitingAnimation(bool val){

    QPalette palette = ui->label_ProcessingProgress->palette();
    palette.setColor(ui->label_ProcessingProgress->foregroundRole(), m_imXPAD_orange);
    ui->label_ProcessingProgress->setPalette(palette);
    ui->label_ProcessingProgress->update();

    ui->label_waiting->setMovie(m_movie);

    if (val){
        m_movie->start();
        m_waiting_animation_flag = true;
        ui->label_ProcessingProgress->setVisible(true);
        this->setAskReadyColor(m_imXPAD_orange);
    }
    else{
        m_movie->stop();
        m_waiting_animation_flag = false;
        ui->label_waiting->setText(" ");
        ui->label_ProcessingProgress->setVisible(false);
        if (m_askready_flag == 2)
            this->setAskReadyColor(m_imXPAD_green);
        else
            this->setAskReadyColor(m_imXPAD_red);
    }
}





/*
 *
 * Section: GUI Functions
 *
 */

void DAQClient::adaptGUIForDetectorModel(){
    if (m_ethernet_connected_status == 1){

        if(m_connection_type == 0 && m_detector_model.contains("10")){
            ui->label_Chip1->setVisible(false);
            ui->label_Chip2->setVisible(false);
            ui->label_Chip3->setVisible(false);
            ui->label_Chip5->setVisible(false);
            ui->label_Chip6->setVisible(false);
            ui->label_Chip7->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip1->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip2->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip3->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip5->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip6->setVisible(false);
            ui->lineEdit_LoadConfigGValue_Chip7->setVisible(false);
            ui->label_Chip4->setText("Chip");
        }
        else{
            ui->label_Chip1->setVisible(true);
            ui->label_Chip2->setVisible(true);
            ui->label_Chip3->setVisible(true);
            ui->label_Chip5->setVisible(true);
            ui->label_Chip6->setVisible(true);
            ui->label_Chip7->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip1->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip2->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip3->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip5->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip6->setVisible(true);
            ui->lineEdit_LoadConfigGValue_Chip7->setVisible(true);
            ui->label_Chip4->setText("Chip 4");
        }
    }
}

void DAQClient::changeImageAcquisitionPath(){
    QString file_path = ui->lineEdit_AcquiredImagePath->text();
    QString file_name = file_path;

    if (ui->radioButton_Binary->isChecked())
        file_name = QFileDialog::getSaveFileName(this, tr("Save File"), file_path, tr("Binary image files (*.bin)"));
    else
        file_name = QFileDialog::getSaveFileName(this, tr("Save File"), file_path, tr("Text image files (*.dat)"));

    if(!file_name.isEmpty())
        ui->lineEdit_AcquiredImagePath->setText(file_name.left(file_name.lastIndexOf(".")));
}

void DAQClient::setGUI(bool val){
    this->setGUIExposure(val);
    this->setGUICalibration(val);
    this->setGUIAdvanced(val);
}

void DAQClient::setGUIConnectivity(bool val){
    ui->pushButton_AskReady->setEnabled(val);
    ui->pushButton_ResetDetector->setEnabled(val);
}

void DAQClient::setGUIExposure(bool val){
    ui->lineEdit_NumberImages->setEnabled(val);
    ui->comboBox_ExpoParamTimeUnits->setEnabled(val);
    ui->lineEdit_ExposeParamTime->setEnabled(val);
    ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->setEnabled(val);
    ui->lineEdit_ExpoParamWaitingTimeBetweenImages->setEnabled(val);
    ui->comboBox_InputSignal->setEnabled(val);
    ui->comboBox_OutputSignal->setEnabled(val);
    ui->comboBox_AcquisitionMode->setEnabled(val);
    //ui->lineEdit_StackImages->setEnabled(val);
    ui->pushButton_Expose->setEnabled(val);
    ui->toolButton_BrowseAcquisitionPath->setEnabled(val);
    ui->lineEdit_AcquiredImagePath->setEnabled(val);
    ui->checkBox_showImageInFiji->setEnabled(val);
    //ui->checkBox_showImageInVisualizer->setEnabled(val);
    ui->checkBox_GeometricalCorrections->setEnabled(val);
    if (m_flat_field_correction_flag)
        ui->checkBox_FlatFieldCorrections->setEnabled(val);
    ui->pushButton_GetWhiteImagesInDir->setEnabled(val);
    ui->radioButton_Ascii->setEnabled(val);
    ui->radioButton_Binary->setEnabled(val);
    ui->radioButton_SaveImagesInServer->setEnabled(val);
    ui->radioButton_TransferImage->setEnabled(val);
    ui->lineEdit_FlatFieldFileBaseName->setEnabled(val);

}

void DAQClient::setGUICalibration(bool val){
    ui->pushButton_LoadCalibrationFromFile->setEnabled(val);
    ui->pushButton_SaveCalibrationToFile->setEnabled(val);
    ui->pushButton_CalibrationOTN->setEnabled(val);
    ui->pushButton_CalibrationOTN_Pulse->setEnabled(val);
    ui->pushButton_CalibrationBEAM->setEnabled(val);
    ui->comboBox_OTNConfiguration->setEnabled(val);
    ui->comboBox_ExpoParamTimeUnitsCalibBEAM->setEnabled(val);
    ui->lineEdit_ExposeParamTimeCalibBEAM->setEnabled(val);
    ui->lineEdit_ITHLMaxCalibBEAM->setEnabled(val);
    ui->comboBox_BEAMConfiguration->setEnabled(val);
}

void DAQClient::setGUIAdvanced(bool val){
    ui->pushButton_LoadDefaultConfigGValues->setEnabled(val);
    ui->lineEdit_LoadConfigGValue->setEnabled(val);
    ui->gridLayout_2->setEnabled(val);
    ui->pushButton_DigitalTest->setEnabled(val);
    ui->comboBox_DigitalTestMode->setEnabled(val);

    ui->comboBox_LoadConfigGlobalRegister->setEnabled(val);
    ui->pushButton_LoadDefaultConfigGValues->setEnabled(val);
    ui->pushButton_LoadConfigG->setEnabled(val);
    ui->lineEdit_LoadConfigGValue->setEnabled(val);
    ui->pushButton_ITHLDecrease->setEnabled(val);
    ui->pushButton_ITHLIncrease->setEnabled(val);
    ui->pushButton_LoadFlatConfigL->setEnabled(val);
    ui->lineEdit_LoadFlatConfigLValue->setEnabled(val);
    ui->pushButton_ShowDACL->setEnabled(val);
    ui->pushButton_ScanDACL->setEnabled(val);
    ui->pushButton_CreateWhiteImage->setEnabled(val);
    ui->pushButton_DeleteWhiteImage->setEnabled(val);
    ui->pushButton_DigitalTest->setEnabled(val);
    ui->pushButton_ReadTemperature->setEnabled(val);
    ui->checkBox_TimerInServer->setEnabled(val);
}

void DAQClient::showConfigGDefaultValues()
{
    switch (ui->comboBox_LoadConfigGlobalRegister->currentIndex()){
    case 0: ui->lineEdit_LoadConfigGValue->setText("25");ui->pushButton_ITHLDecrease->setEnabled(false); ui->pushButton_ITHLIncrease->setEnabled(false);break;
    case 1: ui->lineEdit_LoadConfigGValue->setText("32");ui->pushButton_ITHLDecrease->setEnabled(true); ui->pushButton_ITHLIncrease->setEnabled(true);break;
    case 2: ui->lineEdit_LoadConfigGValue->setText("120");ui->pushButton_ITHLDecrease->setEnabled(false); ui->pushButton_ITHLIncrease->setEnabled(false);
    }
    this->readConfigG();
}

/*
 *
 * Section: Setting Colors
 *
 */

void DAQClient::setAskReadyColor(QColor color){

    QPalette palette = ui->pushButton_AskReady->palette();
    palette.setColor(ui->pushButton_AskReady->backgroundRole(), QColor(222,220,213,255));
    palette.setColor(ui->pushButton_AskReady->foregroundRole(),color);
    ui->pushButton_AskReady->setPalette(palette);
    ui->pushButton_AskReady->update();

    palette = ui->pushButton_stackedWidget_Connectivity->palette();
    palette.setColor(ui->pushButton_stackedWidget_Connectivity->backgroundRole(),color);
    palette.setColor(ui->pushButton_stackedWidget_Connectivity->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Connectivity->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Connectivity->setPalette(palette);

    //Connectivity button color
    if (color == m_imXPAD_red){
        ui->pushButton_AskReady->setText("Check detector status");
        m_askready_flag = 0;
    }
    else if (color == m_imXPAD_orange){
        ui->pushButton_AskReady->setText("Detector is BUSY, please wait...");
        m_askready_flag = 1;
    }
    else{
        ui->pushButton_AskReady->setText("Detector READY to acquire");
        m_askready_flag = 2;
    }

    m_viewer->setAcquireButtonColor(color);
}

void DAQClient::setConnectEthernetServerColor(QColor color){

    QPalette palette = ui->pushButton_ConnectEthernetServer->palette();
    palette.setColor(ui->pushButton_ConnectEthernetServer->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_ConnectEthernetServer->foregroundRole(),color);
    ui->pushButton_ConnectEthernetServer->setAutoFillBackground(true);
    ui->pushButton_ConnectEthernetServer->setPalette(palette);
    ui->pushButton_ConnectEthernetServer->update();
}

void DAQClient::setStackedWidgetConnectivity(){
    ui->stackedWidget->setCurrentIndex(0);

    QPalette palette = ui->pushButton_stackedWidget_Connectivity->palette();

    if (m_askready_flag == 2){
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->backgroundRole(),m_imXPAD_green);
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->foregroundRole(),m_imXPAD_button_text);
    }
    else if (m_askready_flag == 1){
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->backgroundRole(),m_imXPAD_orange);
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->foregroundRole(),m_imXPAD_button_text);
    }
    else{
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->backgroundRole(),m_imXPAD_red);
        palette.setColor(ui->pushButton_stackedWidget_Connectivity->foregroundRole(),m_imXPAD_button_text);
    }
    ui->pushButton_stackedWidget_Connectivity->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Connectivity->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Expose->palette();
    palette.setColor(ui->pushButton_stackedWidget_Expose->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Expose->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Expose->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Expose->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Calibration->palette();
    palette.setColor(ui->pushButton_stackedWidget_Calibration->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Calibration->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Calibration->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Calibration->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Advanced->palette();
    palette.setColor(ui->pushButton_stackedWidget_Advanced->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Advanced->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Advanced->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Advanced->setPalette(palette);
}

void DAQClient::setStackedWidgetExpose(){
    ui->stackedWidget->setCurrentIndex(1);

    QPalette palette = ui->pushButton_stackedWidget_Expose->palette();
    palette.setColor(ui->pushButton_stackedWidget_Expose->backgroundRole(),m_imXPAD_button_pushed);
    palette.setColor(ui->pushButton_stackedWidget_Expose->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Expose->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Expose->setPalette(palette);

    palette.setColor(ui->pushButton_Expose->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_Expose->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_Expose->setAutoFillBackground(true);
    ui->pushButton_Expose->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Calibration->palette();
    palette.setColor(ui->pushButton_stackedWidget_Calibration->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Calibration->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Calibration->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Calibration->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Advanced->palette();
    palette.setColor(ui->pushButton_stackedWidget_Advanced->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Advanced->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Advanced->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Advanced->setPalette(palette);

}

void DAQClient::setStackedWidgetCalibration(){
    ui->stackedWidget->setCurrentIndex(2);

    QPalette palette = ui->pushButton_stackedWidget_Calibration->palette();
    palette.setColor(ui->pushButton_stackedWidget_Calibration->backgroundRole(),m_imXPAD_button_pushed);
    palette.setColor(ui->pushButton_stackedWidget_Calibration->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Calibration->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Calibration->setPalette(palette);

    palette.setColor(ui->pushButton_CalibrationOTN_Pulse->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_CalibrationOTN_Pulse->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_CalibrationOTN_Pulse->setAutoFillBackground(true);
    ui->pushButton_CalibrationOTN_Pulse->setPalette(palette);

    palette.setColor(ui->pushButton_CalibrationOTN->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_CalibrationOTN->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_CalibrationOTN->setAutoFillBackground(true);
    ui->pushButton_CalibrationOTN->setPalette(palette);

    palette.setColor(ui->pushButton_CalibrationBEAM->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_CalibrationBEAM->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_CalibrationBEAM->setAutoFillBackground(true);
    ui->pushButton_CalibrationBEAM->setPalette(palette);

    palette.setColor(ui->pushButton_LoadCalibrationFromFile->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_LoadCalibrationFromFile->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_LoadCalibrationFromFile->setAutoFillBackground(true);
    ui->pushButton_LoadCalibrationFromFile->setPalette(palette);

    palette.setColor(ui->pushButton_SaveCalibrationToFile->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_SaveCalibrationToFile->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_SaveCalibrationToFile->setAutoFillBackground(true);
    ui->pushButton_SaveCalibrationToFile->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Expose->palette();
    palette.setColor(ui->pushButton_stackedWidget_Expose->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Expose->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Expose->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Expose->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Advanced->palette();
    palette.setColor(ui->pushButton_stackedWidget_Advanced->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Advanced->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Advanced->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Advanced->setPalette(palette);

    palette = ui->pushButton_CreateWhiteImage->palette();
    palette.setColor(ui->pushButton_GetWhiteImagesInDir->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_GetWhiteImagesInDir->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_GetWhiteImagesInDir->setAutoFillBackground(true);
    ui->pushButton_GetWhiteImagesInDir->setPalette(palette);

    palette = ui->pushButton_CreateWhiteImage->palette();
    palette.setColor(ui->pushButton_CreateWhiteImage->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_CreateWhiteImage->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_CreateWhiteImage->setAutoFillBackground(true);
    ui->pushButton_CreateWhiteImage->setPalette(palette);

    palette = ui->pushButton_DeleteWhiteImage->palette();
    palette.setColor(ui->pushButton_DeleteWhiteImage->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_DeleteWhiteImage->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_DeleteWhiteImage->setAutoFillBackground(true);
    ui->pushButton_DeleteWhiteImage->setPalette(palette);
}

void DAQClient::setStackedWidgetAdvanced(){
    ui->stackedWidget->setCurrentIndex(3);

    QPalette palette = ui->pushButton_stackedWidget_Advanced->palette();
    palette.setColor(ui->pushButton_stackedWidget_Advanced->backgroundRole(),m_imXPAD_button_pushed);
    palette.setColor(ui->pushButton_stackedWidget_Advanced->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Advanced->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Advanced->setPalette(palette);

    palette.setColor(ui->pushButton_ResetDetector->backgroundRole(), m_imXPAD_special_button);
    palette.setColor(ui->pushButton_ResetDetector->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ResetDetector->setAutoFillBackground(true);
    ui->pushButton_ResetDetector->setPalette(palette);

    palette.setColor(ui->pushButton_DigitalTest->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_DigitalTest->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_DigitalTest->setAutoFillBackground(true);
    ui->pushButton_DigitalTest->setPalette(palette);

    palette.setColor(ui->pushButton_ScanDACL->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_ScanDACL->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ScanDACL->setAutoFillBackground(true);
    ui->pushButton_ScanDACL->setPalette(palette);

    palette.setColor(ui->pushButton_LoadConfigG->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_LoadConfigG->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_LoadConfigG->setAutoFillBackground(true);
    ui->pushButton_LoadConfigG->setPalette(palette);

    palette.setColor(ui->pushButton_LoadDefaultConfigGValues->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_LoadDefaultConfigGValues->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_LoadDefaultConfigGValues->setAutoFillBackground(true);
    ui->pushButton_LoadDefaultConfigGValues->setPalette(palette);

    palette.setColor(ui->pushButton_LoadFlatConfigL->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_LoadFlatConfigL->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_LoadFlatConfigL->setAutoFillBackground(true);
    ui->pushButton_LoadFlatConfigL->setPalette(palette);

    palette.setColor(ui->pushButton_ShowDACL->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_ShowDACL->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ShowDACL->setAutoFillBackground(true);
    ui->pushButton_ShowDACL->setPalette(palette);

    palette.setColor(ui->pushButton_ITHLIncrease->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_ITHLIncrease->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ITHLIncrease->setAutoFillBackground(true);
    ui->pushButton_ITHLIncrease->setPalette(palette);

    palette.setColor(ui->pushButton_ITHLDecrease->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_ITHLDecrease->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ITHLDecrease->setAutoFillBackground(true);
    ui->pushButton_ITHLDecrease->setPalette(palette);

    palette.setColor(ui->pushButton_ReadTemperature->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_ReadTemperature->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ReadTemperature->setAutoFillBackground(true);
    ui->pushButton_ReadTemperature->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Calibration->palette();
    palette.setColor(ui->pushButton_stackedWidget_Calibration->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Calibration->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Calibration->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Calibration->setPalette(palette);

    palette = ui->pushButton_stackedWidget_Expose->palette();
    palette.setColor(ui->pushButton_stackedWidget_Expose->backgroundRole(),m_imXPAD_button);
    palette.setColor(ui->pushButton_stackedWidget_Expose->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_stackedWidget_Expose->setAutoFillBackground(true);
    ui->pushButton_stackedWidget_Expose->setPalette(palette);
}

void DAQClient::on_comboBox_AcquisitionMode_currentIndexChanged(int index)
{
    if(index < 3){
        ui->lineEdit_StackImages->setEnabled(false);
        ui->label_StackImages->setEnabled(false);
    }
    else{
        ui->lineEdit_StackImages->setEnabled(true);
        ui->label_StackImages->setEnabled(true);
    }
}

void DAQClient::on_checkBox_GeometricalCorrections_clicked(){
    m_viewer->setFirstTimeOn();
}

void DAQClient::on_radioButton_SaveImagesInServer_toggled(bool checked){
    if (checked == true){
        if (ui->lineEdit_IP->text().contains("localhost",Qt::CaseInsensitive) || ui->lineEdit_IP->text().contains("127.0.0.1")){
            ui->label_PathInServer->setEnabled(true);
            ui->lcdNumber_BurstNumber->setEnabled(true);
        }
        else{
            QMessageBox::warning(this, tr("DAQ Client"),
                                 tr("This option is only available if IP is either \"localhost\" or \"127.0.0.1\""),QMessageBox::Ok);
            ui->radioButton_SaveImagesInServer->setChecked(false);
            ui->radioButton_TransferImage->setChecked(true);
        }
    }
    else{
        ui->label_PathInServer->setEnabled(false);
        ui->lcdNumber_BurstNumber->setEnabled(false);

    }
}

void DAQClient::on_checkBox_TimerInServer_clicked(bool checked){
    if(m_ethernet_connected_status == 1){

        this->getStatus();

        if (m_detector_status.contains("Idle")){

            QString status;

            if (checked)
                status = "true";
            else
                status = "false";

            QString command = "ShowTimer " + status + "\n";
            this->sendCommand(command);
            this->receiveMessage();
            this->evaluateAnswer();
        }
    }
}

/*
 * Section: Read/Write Parameters
 */

int DAQClient::readParameters (){

    QString     data;
    QStringList list;


    QString file_name = QDir::homePath() + "/XPAD_DAQ/parameters.dat";

    QFile file(file_name);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        /*QMessageBox::critical(this, tr("DAQ Client"),
                              file_name + tr(" could not be open.\n\n")
                              + tr("Parameters from previous session will not be loaded."));
        this->showError("ERROR: Parameters from previous session will not be loaded.");*/
        return -1;
    }
    else{
        while (!file.atEnd()){
            data = file.readLine();

            if (!data.contains("#")){
                list = data.split(QRegExp("[\t\n]"), QString::SkipEmptyParts);

                //if (data.contains("connection_type"))
                //    ui->comboBox_ConnexionType->setCurrentIndex(list.at(list.size() - 1).toInt());
                if (data.contains("port"))
                    ui->lineEdit_Port->setText(list.at(list.size() - 1));
                else if (data.contains("IP"))
                    ui->lineEdit_IP->setText(list.at(list.size() - 1));
                //else if (data.contains("detector_model"))
                //    ui->comboBox_DetectorModel->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("test_mode"))
                    ui->comboBox_DigitalTestMode->setCurrentIndex(list.at(list.size() - 1).toInt());

                else if (data.contains("global_configuration_value"))
                    ui->lineEdit_LoadConfigGValue->setText(list.at(list.size() - 1));
                else if (data.contains("global_configuration_register"))
                    ui->comboBox_LoadConfigGlobalRegister->setCurrentIndex(list.at(list.size() - 1).toInt());

                else if (data.contains("local_configuration_value"))
                    ui->lineEdit_LoadFlatConfigLValue->setText(list.at(list.size() - 1));

                else if (data.contains("number_of_images"))
                    ui->lineEdit_NumberImages->setText(list.at(list.size() - 1));
                else if (data.contains("exposure_time"))
                    ui->lineEdit_ExposeParamTime->setText(list.at(list.size() - 1));
                else if (data.contains("time_units"))
                    ui->comboBox_ExpoParamTimeUnits->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("time_between_images_value"))
                    ui->lineEdit_ExpoParamWaitingTimeBetweenImages->setText(list.at(list.size() - 1));
                else if (data.contains("time_between_images_units"))
                    ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("input_signal"))
                    ui->comboBox_InputSignal->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("output_signal"))
                    ui->comboBox_OutputSignal->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("acquisition_mode"))
                    ui->comboBox_AcquisitionMode->setCurrentIndex(list.at(list.size() -1).toInt());
                else if (data.contains("stack_images"))
                    ui->lineEdit_StackImages->setText(list.at(list.size() - 1));
                else if (data.contains("image_format"))
                    ui->radioButton_Binary->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("image_format"))
                    ui->radioButton_Binary->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("transfer_image"))
                    ui->radioButton_TransferImage->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("show_image_fiji"))
                    ui->checkBox_showImageInFiji->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("show_image_visualizer"))
                    ui->checkBox_showImageInVisualizer->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("geometrical_corrections"))
                    ui->checkBox_GeometricalCorrections->setChecked((bool)list.at(list.size() - 1).toInt());
                else if (data.contains("flat_field_corrections"))
                    ui->checkBox_FlatFieldCorrections->setChecked((bool)list.at(list.size() - 1).toInt());


                else if (data.contains("OTNConfiguration"))
                    ui->comboBox_OTNConfiguration->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("BEAMConfiguration"))
                    ui->comboBox_BEAMConfiguration->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("BEAM_expose_time"))
                    ui->lineEdit_ExposeParamTimeCalibBEAM->setText(list.at(list.size() - 1));
                else if (data.contains("BEAM_time_unit"))
                    ui->comboBox_ExpoParamTimeUnitsCalibBEAM->setCurrentIndex(list.at(list.size() - 1).toInt());
                else if (data.contains("BEAM_ITHL_max"))
                    ui->lineEdit_ITHLMaxCalibBEAM->setText(list.at(list.size() - 1));


                else if (data.contains("filePath"))
                    ui->lineEdit_AcquiredImagePath->setText(list.at(list.size() - 1));
                else if (data.contains("flat_field_image_filename"))
                    ui->lineEdit_FlatFieldFileBaseName->setText(list.at(list.size() - 1));
            }
        }
        file.close();
        return 0;
    }
}

int DAQClient::writeParameters (){

    QString file_name = QDir::homePath() + "/XPAD_DAQ/parameters.dat";

    QFile file(file_name);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this, tr("DAQ Client"),
                              file_name + tr(" could not be open.\n\n")
                              + tr("Parameters of this session will not be saved."));
        this->showError("ERROR: Parameters of this session will not be saved.");
        return -1;
    }
    else{
        QTextStream out(&file);

        out << "## C O N N E C T I V I T Y ##" << endl;
        //out << "connection_type" << "\t\t\t\t" << ui->comboBox_ConnexionType->currentIndex() << endl;
        out << "port" << "\t\t\t\t" << ui->lineEdit_Port->text().toStdString().data() << endl;
        out << "IP" << "\t\t\t\t" << ui->lineEdit_IP->text().toStdString().data() << endl;
        //out << "detector_model" << "\t\t\t\t" << ui->comboBox_DetectorModel->currentIndex() << endl;
        out << endl;
        out << "##  D I G I T A L    T E S T ##" << endl;
        out << "test_mode" << "\t\t\t\t"  << ui->comboBox_DigitalTestMode->currentIndex() << endl;
        out << endl;
        out << "##  G L O B A L    C O N F I G U R A T I O N ##" << endl;
        out << "global_configuration_value" << "\t\t\t\t"  << ui->lineEdit_LoadConfigGValue->text().toStdString().data() << endl;
        out << "global_configuration_register" << "\t\t\t\t" << ui->comboBox_LoadConfigGlobalRegister->currentIndex() << endl;
        out << endl;
        out << "##  L O C A L    C O N F I G U R A T I O N ##" << endl;
        out << "local_configuration_value" << "\t\t\t\t"  << ui->lineEdit_LoadFlatConfigLValue->text().toStdString().data() << endl;
        out << endl;
        out << "##  E X P O S U R E    P A R A M E T E R S ##" << endl;
        out << "number_of_images" << "\t\t\t\t"  << ui->lineEdit_NumberImages->text().toStdString().data() << endl;
        out << "exposure_time" << "\t\t\t\t"  << ui->lineEdit_ExposeParamTime->text().toStdString().data() << endl;
        out << "time_units" << "\t\t\t\t"  << ui->comboBox_ExpoParamTimeUnits->currentIndex() << endl;
        out << "time_between_images_value" << "\t\t\t\t" << ui->lineEdit_ExpoParamWaitingTimeBetweenImages->text().toStdString().data() << endl;
        out << "time_between_images_units" << "\t\t\t\t" << ui->comboBox_ExpoParamWaitingTimeBetweenImagesUnits->currentIndex() << endl;
        out << "input_signal" << "\t\t\t\t" << ui->comboBox_InputSignal->currentIndex() << endl;
        out << "output_signal" << "\t\t\t\t" << ui->comboBox_OutputSignal->currentIndex() << endl;
        out << "acquisition_mode" << "\t\t\t\t" << ui->comboBox_AcquisitionMode->currentIndex() << endl;
        out << "stack_images" << "\t\t\t\t"  << ui->lineEdit_StackImages->text().toStdString().data() << endl;
        out << "image_format" << "\t\t\t\t" << ui->radioButton_Binary->isChecked() << endl;
        out << "transfer_image" << "\t\t\t\t" << ui->radioButton_TransferImage->isChecked() << endl;
        out << "show_image_fiji" << "\t\t\t\t" << ui->checkBox_showImageInFiji->isChecked() << endl;
        out << "show_image_visualizer" << "\t\t\t\t" << ui->checkBox_showImageInVisualizer->isChecked() << endl;
        out << "geometrical_corrections" << "\t\t\t\t" << ui->checkBox_GeometricalCorrections->isChecked() << endl;
        out << "flat_field_corrections" << "\t\t\t\t" << ui->checkBox_FlatFieldCorrections->isChecked() << endl;
        out << endl;
        out << "##  C A L I B R A T I O N    C O N F I G U R A T I O N ##" << endl;
        out << "OTNConfiguration" << "\t\t\t\t" << ui->comboBox_OTNConfiguration->currentIndex() << endl;
        out << "BEAMConfiguration" << "\t\t\t\t" << ui->comboBox_BEAMConfiguration->currentIndex() << endl;
        out << "BEAM_expose_time" << "\t\t\t\t" << ui->lineEdit_ExposeParamTimeCalibBEAM->text().toStdString().data() << endl;
        out << "BEAM_time_unit" << "\t\t\t\t" << ui->comboBox_ExpoParamTimeUnitsCalibBEAM->currentIndex() << endl;
        out << "BEAM_ITHL_max" << "\t\t\t\t" << ui->lineEdit_ITHLMaxCalibBEAM->text().toStdString().data() << endl;
        out << endl;
        out << "##  F I L E    P A T H  ##" << endl;
        if (!ui->lineEdit_AcquiredImagePath->text().isEmpty())
            out << "filePath" << "\t\t\t\t" << ui->lineEdit_AcquiredImagePath->text() << endl;
        else{
            QString path = QDir::homePath() + "/XPAD_DAQ/Images/Image";
            out << "filePath" << "\t\t\t\t" << path.toStdString().c_str() << endl;
        }
        if (!ui->lineEdit_FlatFieldFileBaseName->text().isEmpty())
            out << "flat_field_image_filename" << "\t\t\t\t" << ui->lineEdit_FlatFieldFileBaseName->text() << endl;
        else{
            out << "flat_field_image_filename" << "\t\t\t\t" << " " << endl;
        }
        file.flush();
        file.close();
        return 0;
    }
}
