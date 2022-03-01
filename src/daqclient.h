#ifndef DAQCLIENT_H
#define DAQCLIENT_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QTimer>
#include <QTcpSocket>
#include <QtWidgets>
#include <QtNetwork>

#include <iostream>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/time.h>
#elif __APPLE__
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <direct.h>
#elif _WIN64
#include <direct.h>
#endif

#include "daqviewer.h"
#include "receiveimagesthread.h"

/* Global Registers */
#define AMPTP                   31
#define IMFP                    59
#define IOTA                    60
#define IPRE                    61
#define ITHL                    62
#define ITUNE                   63
#define IBUFF                   64

#define SLOW                    0
#define MEDIUM                  1
#define FAST                    2

#define IMG_LINE	120
#define IMG_COLUMN 	80


class QTcpSocket;
class QNetworkSession;

namespace Ui {
class DAQClient;
}

/*
 * Class: DAQClient
 *
 * A client that allows to control and to acquire images from XPAD detectors.
 *
 */
class DAQClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit DAQClient(QWidget *parent = 0);
    ~DAQClient();

protected:
    void closeEvent(QCloseEvent *);

public slots:
    //****Connectivity Functions****/
    void connectToServer();                                         //!< Connect the DAQ to a SERVER using the IP and PORT specified in the ui.
    void connectionOpen();                                          //!< Once TCP connections is stablished, it initialise the connection with XPAD Detector.
    void connectionClose();                                         //!< Block the access to buttons when the connection to XPAD server is closed.

    //****XPAD Functions****/
    void init();                                                    //!< Initialize the detector.
    void getModuleNumber();                                         //!< Get the number of modules in XPAD detector.
    void getChipNumber();                                           //!< Get the number of chips in one module.
    void getModuleMask();                                           //!< Get the mask corresponding to the number of modules in the XPAD detector.
    void getChipMask();                                             //!< Get the mask for the chips in one module.
    void getDetectorType();                                         //!< Get type of connection use by the detector to connect to the server.
    void getDetectorModel();                                        //!< Get the XPAD detector model attached to the SERVER.
    void getImageSize();                                            //!< Get the current image size.
    void getConnectionID();                                         //!< Get the connection ID.

    void askReady();                                                //!< Test the communication to XPAD detector.
    void digitalTest();                                             //!< Performs a Digital Test.


    //****XPAD Configuration****/
    void loadDefaultConfigGValues();                                //!< Load default values to all registers of global configuration
    void readConfigG();                                             //!< Read the value of a single register of global configuration
    void loadConfigG();                                             //!< Load a value to a certain register of global configuration
    void ITHLIncrease();                                            //!< Increase the ITHL values for all registers in one unit.
    void ITHLDecrease();                                            //!< Decrease the ITHL values for all registers in one unit.
    void loadFlatConfigL();                                         //!< Load the same local configuration value for all pixels.
    void showDACL();                                                //!< Get the local configuration stored in the detector and display it with its corresponding histogram.

    //****XPAD Image Acquisition****/
    void setExposureParameters();                                   //!< Set the exposure parameters.
    void startExposure();                                           //!< Start acquiring images using the expose parameters previously sent to the detector.
    void scanDACL();                                                //!< Perform a ScanDACL using exposure parameters.


    //****XPAD Calibrations****/
    void calibrationOTN();                                          //!< Performs a Over-The-Noise Calibration to set the pixel thresholds just above the noise.
    void calibrationOTNPulse();                                     //!< Performs a Over-The-Noise Calibration using the pulse function to better find the noise and set properly the pixel thresholds just above this noise.
    void calibrationBEAM();                                         //!< Set the pixel threshold at a derised energy.
    void loadCalibrationFromFile();                                 //!< Load global and configuration files into the detector.
    void saveCalibrationToFile();                                   //!< Saves the global and local configuration stored in the detector to a file.

    //****XPAD Status****/
    void getStatus();                                               //!< Retreive the current status of the detector.

    //***XPAD Rest***/
    void resetDetector();                                           //!< Reset module of XPAD detector

    //***XPAD Exit***/
    void quit();                                                    //!< Tells XPAD server to free the thread reserved for the current connection.
    void quit_DAQ();                                                //!< Quits DAQ CLient

    //***XPAD Abort***/
    void abortCurrentProcess();                                     //!< Send an abort to Server
    void setAbortState(bool state);                                 //!< Set the abort status
    bool getAbortState();                                           //!< Read abort status

    //****Other Functions****/
    void connectionType();                                          //!< Change the connection type USB or PCI.


private slots:
    //****Server Functions****/
    void loadConfigLFromFile();                                     //!< Load local congfiguration from file after Global configuration was loaded.

    //****Message Functions****/
    void displayError(QAbstractSocket::SocketError socketError);    //!< Show the error when connecting to XPAD server.
    void showMessage(QString message);                              //!< Show messages in Status window and console.
    void showError(QString message);                                //!< Show errors in Status window and console.
    void showWarning(QString message);                              //!< Show warnings in Status window and console.
    void showWaitingMessage(QString message);                       //!< Show waiting message in Status window and console.

    //****Send/Receive Functions****/
    void sendCommand(QString cmd);                                  //!< Send a command to XPAD server.
    void receiveMessage();                                          //!< Receive the answers from XPAD server after a command was sent.
    void evaluateAnswer();                                          //!< Evaluate the answer received from XPAD server depending of the command sent.
    bool receiveImage(QString fileName, int imageNumber);           //!< Receive the images following the XPAD 3.x image transfer protocol.
    bool readImage(QString fileName, int imageNumber);              //!< Read images from SSD.
    QByteArray receiveParametersFile();                             //!< Receive configuration files.
    void sendParametersFile(QByteArray data);                       //!< Send configuration files.
    void dataReceivedInTcp();                                       //!< Interruption called when data arrives via TCP.

     //****Displaying Images****/
    void showImage(QString fileName, quint32 lineNumber,
                   quint32 columnNumber);                           //!< Display image.
    void setContinuousAcquisitionON();                              //!< Set continuous acquisition mode ON.
    void setContinuousAcquisitionOFF();                             //!< Set continuous acquisition mode OFF.

    //****Manage White Image****/
    void createWhiteImage();                                        //!< Create a white image in server.
    void deleteWhiteImage();                                        //!< Delete a white image in server.
    void getWhiteImagesInDir();                                     //!< Reads name of white images availables in server.

    //****Read Temperature****/
    void readTemperature();                                         //!< Reads detector temperature differences.

    //****GIF Animations Functions****/
    void showWaitingAnimation(bool val);                            //!< Display Waiting animation.

    //****GUI Functions****/
    void adaptGUIForDetectorModel();                                //!< Depending on the model type, Advanced->Global Configuration GUI will be adapted.
    void changeImageAcquisitionPath();                              //!< Change FileDialog for binary or ASCII images depending on the radioButton.
    void setGUI(bool val);                                          //!< Enable or Unable the GUI.
    void setGUIConnectivity(bool bal);                              //!< Enable or Unable the GUI for Connectivity tab.
    void setGUIExposure(bool val);                                  //!< Enable or Unable the GUI for Exposure tab.
    void setGUICalibration(bool val);                               //!< Enable or Unable the GUI for Calibration tab.
    void setGUIAdvanced(bool val);                                  //!< Enable or Unable the GUI for Advanced tab.
    void showConfigGDefaultValues();                                //!< Set the default values in XpadDAQ GUI.

    //****Setting Colors****/
    void setAskReadyColor(QColor color);                            //!< Change the color of Ask Ready button.
    void setConnectEthernetServerColor(QColor color);               //!< Change the color of Connect to Server button.
    void setStackedWidgetConnectivity();                            //!< Set color values when connectivity tab is selected.
    void setStackedWidgetExpose();                                  //!< Set color values when expose tab is selected.
    void setStackedWidgetCalibration();                             //!< Set color values when calibration tab is selected.
    void setStackedWidgetAdvanced();                                //!< Set color values when advanced tab is selected.

    void on_comboBox_AcquisitionMode_currentIndexChanged(int index);   
    void on_radioButton_SaveImagesInServer_toggled(bool checked);
    void on_checkBox_GeometricalCorrections_clicked();
    void on_checkBox_TimerInServer_clicked(bool checked);

    //****Read/Write Parameters****/
    int readParameters();                                           //!< Read parameters from file to be loaded when DAQ is started.
    int writeParameters();                                          //!< Write parametes to file when DAQ is closed.

private:


    //****GLOBAL flags****/
    ushort              m_connection_type;                          //!< Variable: m_connection_type  Flag that define USB or PCI connection.
    QString             m_detector_model;                           //!< Define the detector model.
    short               m_ethernet_connected_status;                //!< Flag that inform of the status connection to XPAD server.

    //****MESSAGE variables****/
    QString             m_command_sent;                             //!< Last command sent to XPAD server.
    QByteArray          m_buffer;                                   //!< Buffer where answer received from XPAD server are stored.
    int                 m_integer_answer;                           //!< Integer value which keeps the last answer received from XPAD server.
    QString             m_string_answer;                            //!< String variable which keeps the last answer received from XPAD server.
    QString             m_detector_status;                          //!< Detector status "Idle:", "Acquiring:", "Resetting:" or "Digital_Test:"

    //****SERVER variables****/
    Ui::DAQClient       *ui;
    QTcpSocket          *m_tcp_socket;                              //!< Socket use to communicate with XPAD server.

    //***DETECTOR variables***/
    unsigned int        m_module_number;                            //!< Number of modules in the detector.
    unsigned int        m_chip_number;                              //!< Number of chips in a module.
    unsigned int        m_module_mask;                              //!< Mask of modules.
    unsigned int        m_chip_mask;                                //!< Mask of chips.
    bool                m_abort_process_state;                      //!< Abort status.
    int                 m_burst_number;                             //!< Process ID.
    unsigned int        m_rows;                                     //!< Number of rows in RAW image.
    unsigned int        m_columns;                                  //!< Number of columns in RAW image.

    //***SAVING PARAMETERS variables***/
    QString             m_expose_filename;                          //!< Name used to store the path where images will be saved.
    bool                m_flat_field_correction_flag;               //!< Variable to store the flat field correction flag.

    //***WAITING ANIMATION variable***/
    bool                m_waiting_animation_flag;                   //!< Flag used to activate or deactivate the waiting animation.

    //***SCANDACL variable***/
    int                 m_scan_DACL_val;                            //!< Variable to store the current DACL value used in the scan.

    //***GENERAL variable***/
    QColor              m_imXPAD_color;                             //!< Colors used in the GUI.
    QColor              m_imXPAD_green;
    QColor              m_imXPAD_red;
    QColor              m_imXPAD_orange;
    QColor              m_imXPAD_button;
    QColor              m_imXPAD_button_pushed;
    QColor              m_imXPAD_button_text;
    QColor              m_imXPAD_special_button;

    int                 m_askready_flag;                            //!< Flag used to know the status of askReady and then set the color of the button.
    QString             m_global_configuration_path;                //!< Path were calibration files are stored.
    QString             m_local_configuration_path;                 //!< Path were calibration files are stored.
    bool                m_continuous_acquisition;                   //!< Flag to set continuous acquisition.
    QString             m_burst_output_file_path;                   //!< File were files are temporary stored in SERVER when transmission via SSD copy is used.

    DAQViewer           *m_viewer;
    QMovie              *m_movie;

};

#endif // DAQCLIENT_H
