#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_hidSmbus(nullptr)
    , m_IIC_ADDR(DEFAULT_IIC_ADDR << 0x1)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    if(m_hidSmbus != nullptr)
        Disconnect();
}

BOOL MainWindow::Connect(DWORD	deviceNum)
{
    BOOL		connected = FALSE;
    string     serial;

    HID_SMBUS_STATUS status = HidSmbus_Open(&m_hidSmbus, deviceNum, VID, PID);

    qDebug() << "HidSmbus_Open():" << HidSmbus_DecodeErrorStatus(status);
    // Attempt to open the device
    if (status == HID_SMBUS_SUCCESS) {
        connected = TRUE;
        // Update the device information now that we are connected to it
        // (this will give us the part number and version if connected)
        UpdateDeviceInformation(true);
        // Update all device settings for all tabs
        SetFromDevice();
        eError r = init();
        qDebug() << r;
    }else {
        qDebug() << "Connection Error:" << HidSmbus_DecodeErrorStatus(status);
    }
    return connected;
}

void MainWindow::UpdateDeviceInformation(bool connected)
{
    BOOL					opened;
    HID_SMBUS_DEVICE_STR	deviceString;
    WORD					vid;
    WORD					pid;
    WORD					releaseNumber;
    BYTE					partNumber;
    BYTE					version;

    char devicePathString[PATH_MAX];
    string				vidString;
    string				pidString;
    string				releaseNumberString;
    string				partNumberString;
    string				versionString;
    string				serialString;
    string				pathString;
    string				manufacturerString;
    string				productString;

    // If we're already connected to the device, then we can call the
    // opened version of the device information functions
    if (connected == TRUE &&
        HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS &&
        opened == TRUE) {
        // Update device information (opened)

        if (HidSmbus_GetOpenedAttributes(m_hidSmbus, &vid, &pid, &releaseNumber) == HID_SMBUS_SUCCESS) {
            qDebug() << "VID =" << QString("0x%1").arg(vid, 4, 16, QChar('0'));
            qDebug() << "PID =" << QString("0x%1").arg(pid, 4, 16, QChar('0'));
            qDebug() << "Release Number =" << QString("%1.%2").arg((UINT)(releaseNumber >> 8)).arg((UINT)((BYTE)releaseNumber));
        }
        if (HidSmbus_GetPartNumber(m_hidSmbus, &partNumber, &version) == HID_SMBUS_SUCCESS) {
            qDebug() << "Part Number =" << partNumber;
            qDebug() << "Version =" << version;
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_SERIAL_STR) == HID_SMBUS_SUCCESS) {
            qDebug() << "Serial =" << deviceString;
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, devicePathString, HID_SMBUS_GET_PATH_STR) == HID_SMBUS_SUCCESS) {
            qDebug() << "Device Path =" << devicePathString;
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_MANUFACTURER_STR) == HID_SMBUS_SUCCESS) {
            qDebug() << "Manufacturer =" << deviceString;
        }
        if (HidSmbus_GetOpenedString(m_hidSmbus, deviceString, HID_SMBUS_GET_PRODUCT_STR) == HID_SMBUS_SUCCESS) {
            qDebug() << "Product =" << deviceString;
        }
    }
}

BOOL MainWindow::Disconnect()
{
    bool disconnected = false;

    // Disconnect from the current device
    HID_SMBUS_STATUS status = HidSmbus_Close(m_hidSmbus);
    m_hidSmbus = nullptr;

    qDebug() << "==================== HidSmbus_Close():" << HidSmbus_DecodeErrorStatus(status);
    // Output an error message if the close failed
    if (status != HID_SMBUS_SUCCESS) {
        qDebug() << "Connection Error:" << HidSmbus_DecodeErrorStatus(status);
    }else {
        disconnected = TRUE;
    }
    return disconnected;
}

void MainWindow::SetFromDevice()
{
    // Update control values
    GetSmbusConfig(false);
    GetTimeouts(false);
    ReadLatch(false);
    GetUsbConfig(false);
    GetLock(false);
    GetManufacturer(false);
    GetProduct(false);
    GetSerial(false);
}

DWORD    bitRate;
void MainWindow::GetSmbusConfig(bool silent)
{
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        BOOL    sclLowTimeout;
        BOOL    autoRespond;
        WORD	writeTimeout;
        WORD	readTimeout;
        WORD	transferRetries;

        // Get the SMBus configuration
        HID_SMBUS_STATUS status = HidSmbus_GetSmbusConfig(m_hidSmbus, &bitRate, &m_I2CAddress, &autoRespond, &writeTimeout, &readTimeout, &sclLowTimeout, &transferRetries);
        qDebug() << "HidSmbus_GetSmbusConfig" << HidSmbus_DecodeErrorStatus(status);
        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << ">>>>>>>>>>>>>>>>>>> before";
            qDebug() << "Bit Rate =" << bitRate << "Hz";
            qDebug() << "Slave Address =" << QString("0x%1").arg(m_I2CAddress, 2, 16, QChar('0'));
            qDebug() << "Auto Send Read Response =" << (autoRespond==true? "true":"false");
            qDebug() << "Write Timeout =" << writeTimeout << "ms";
            qDebug() << "Read Timeout =" << readTimeout << "ms";
            qDebug() << "Transfer Retries =" << transferRetries;
            qDebug() << "SCL Low Timeout =" << (sclLowTimeout==true? "true":"false");

            status = HidSmbus_SetSmbusConfig(m_hidSmbus, bitRate, m_IIC_ADDR, false, 1000, 1000, sclLowTimeout, transferRetries);
            qDebug() << "HidSmbus_SetSmbusConfig" << HidSmbus_DecodeErrorStatus(status);
            if (status == HID_SMBUS_SUCCESS) {
                HID_SMBUS_STATUS status = HidSmbus_GetSmbusConfig(m_hidSmbus, &bitRate, &m_I2CAddress, &autoRespond, &writeTimeout, &readTimeout, &sclLowTimeout, &transferRetries);
                qDebug() << "HidSmbus_GetSmbusConfig" << HidSmbus_DecodeErrorStatus(status);
                if (status == HID_SMBUS_SUCCESS) {
                    qDebug() << ">>>>>>>>>>>>>>>>>>> after";
                    qDebug() << "Bit Rate =" << bitRate << "Hz";
                    qDebug() << "Slave Address =" << QString("0x%1").arg(m_I2CAddress, 2, 16, QChar('0'));
                    qDebug() << "Auto Send Read Response =" << (autoRespond==true? "true":"false");
                    qDebug() << "Write Timeout =" << writeTimeout << "ms";
                    qDebug() << "Read Timeout =" << readTimeout << "ms";
                    qDebug() << "Transfer Retries =" << transferRetries;
                    qDebug() << "SCL Low Timeout =" << (sclLowTimeout==true? "true":"false");
                }
            }
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetSmbusConfig():" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}
DWORD responseTimeout;
void MainWindow::GetTimeouts(bool silent)
{
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        // Get response timeouts
        HID_SMBUS_STATUS status = HidSmbus_GetTimeouts(m_hidSmbus, &responseTimeout);
        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << "Response Timeout =" << responseTimeout << "ms";
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetTimeouts():" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::ReadLatch(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        BYTE latchValue;

        // Read GPIO latch value
        HID_SMBUS_STATUS status = HidSmbus_ReadLatch(m_hidSmbus, &latchValue);
        if (status == HID_SMBUS_SUCCESS) {
            // Update controls to reflect latch value
            qDebug() << "LatchValue =" << QString("0x%1").arg(latchValue, 4, 16, QChar('0'));
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_ReadLatch:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::GetUsbConfig(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        WORD vid;
        WORD pid;
        BYTE power;
        BYTE powerMode;
        WORD releaseVersion;

        // Get USB Configuration
        HID_SMBUS_STATUS status = HidSmbus_GetUsbConfig(m_hidSmbus, &vid, &pid, &power, &powerMode, &releaseVersion);
        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << "Custom Vid:" << QString("0x%1").arg(vid, 4, 16, QChar('0'));
            qDebug() << "Custom Pid:" << QString("0x%1").arg(pid, 4, 16, QChar('0'));
            qDebug() << "Custom Power:" << power * 2;
            qDebug() << "Custom Power Mode:" << HidSmbus_DecodePower(powerMode);
            qDebug() << "Release Version:" << QString("%1.%2").arg(releaseVersion >> 8).arg(releaseVersion & 0xFF);
        }

        if (!silent)
        {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetUsbConfig:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::GetLock(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        BYTE lock = 0x00;

        // Get lock byte
        HID_SMBUS_STATUS status = HidSmbus_GetLock(m_hidSmbus, &lock);
        if (status == HID_SMBUS_SUCCESS) {
            // Check the field lock checkbox if the field is unlocked
            // Once a field is locked, it cannot be unlocked
            qDebug() << "HID_SMBUS_LOCK_VID:" << ((lock & HID_SMBUS_LOCK_VID)==HID_SMBUS_LOCK_VID);
            qDebug() << "HID_SMBUS_LOCK_PID:" << ((lock & HID_SMBUS_LOCK_PID)==HID_SMBUS_LOCK_PID);
            qDebug() << "HID_SMBUS_LOCK_POWER:" << ((lock & HID_SMBUS_LOCK_POWER)==HID_SMBUS_LOCK_POWER);
            qDebug() << "HID_SMBUS_LOCK_POWER_MODE:" << ((lock & HID_SMBUS_LOCK_POWER_MODE)==HID_SMBUS_LOCK_POWER_MODE);
            qDebug() << "HID_SMBUS_LOCK_RELEASE_VERSION:" << ((lock & HID_SMBUS_LOCK_RELEASE_VERSION)==HID_SMBUS_LOCK_RELEASE_VERSION);
            qDebug() << "HID_SMBUS_LOCK_MFG_STR:" << ((lock & HID_SMBUS_LOCK_MFG_STR)==HID_SMBUS_LOCK_MFG_STR);
            qDebug() << "HID_SMBUS_LOCK_PRODUCT_STR:" << ((lock & HID_SMBUS_LOCK_PRODUCT_STR)==HID_SMBUS_LOCK_PRODUCT_STR);
            qDebug() << "HID_SMBUS_LOCK_SERIAL_STR:" << ((lock & HID_SMBUS_LOCK_SERIAL_STR)==HID_SMBUS_LOCK_SERIAL_STR);
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetLock:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::GetManufacturer(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        HID_SMBUS_CP2112_MFG_STR	manufacturingString;
        BYTE						strlen					= 0;

        // Get manufacturer string
        HID_SMBUS_STATUS status = HidSmbus_GetManufacturingString(m_hidSmbus, manufacturingString, &strlen);
        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << "Manufacturing String:" << manufacturingString;
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetManufacturingString:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::GetProduct(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        HID_SMBUS_CP2112_PRODUCT_STR	productString;
        BYTE							strlen			= 0;

        // Get product string
        HID_SMBUS_STATUS status = HidSmbus_GetProductString(m_hidSmbus, productString, &strlen);
        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << "Product String:" << productString;
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetProductString:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::GetSerial(bool silent){
    BOOL opened;

    // Make sure that the device is opened
    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        HID_SMBUS_CP2112_SERIAL_STR		serialString;
        BYTE							strlen			= 0;

        // Get serial string
        HID_SMBUS_STATUS status = HidSmbus_GetSerialString(m_hidSmbus, serialString, &strlen);

        if (status == HID_SMBUS_SUCCESS) {
            qDebug() << "Serial String:" << serialString;
        }

        if (!silent) {
            // Output status to status bar
            // And play an audible alert if the status is not HID_SMBUS_SUCCESS
            qDebug() << "==================== HidSmbus_GetSerialString:" << HidSmbus_DecodeErrorStatus(status);
        }
    }
}

void MainWindow::on_connectButton_clicked()
{
    DWORD numDevices = 0;
    HidSmbus_GetNumDevices(&numDevices, VID, PID);
    qDebug() << "num devices" << numDevices;
    for(DWORD i=0; i<numDevices; i++) {
        Connect(i);
    }
}


void MainWindow::on_disconnectButton_clicked()
{
    Disconnect();
}

eError MainWindow::init()
{
    eError ret = NO_ERROR;
    ret = soft_reset();
    return ret;
}



eError MainWindow::soft_reset()
{
    eError ret = NO_ERROR;
    ret = send_command(CMD_SOFT_RST);
    return ret;
}

eError MainWindow::read_meas_data_single_shot(ushort cfg_cmd, float* temp, float* hum)
{
    eError ret = ERROR_COMM;
    uchar data[6] = {0};
    ushort temp_hex = 0, hum_hex = 0;
    BOOL opened;

    if (HidSmbus_IsOpened(m_hidSmbus, &opened) == HID_SMBUS_SUCCESS && opened) {
        if(send_command(cfg_cmd) == NO_ERROR &&
           read_bytes(data, sizeof(data)) == NO_ERROR) {
            temp_hex = (data[0] << 8) | data[1];
            hum_hex = (data[3] << 8) | data[4];

            *temp = get_temp(temp_hex);
            *hum = get_hum(hum_hex);
            ret = NO_ERROR;
        }
    }
    return ret;
}


float MainWindow::get_temp(ushort temp)
{
    return (temp / 65535.00) * 175 - 45;
}

float MainWindow::get_hum(ushort hum)
{
    return (hum / 65535.0) * 100.0;
}

eError MainWindow::send_command(ushort cmd)
{
    uchar data[2];
    data[0] = (cmd >> 8) & 0xff;
    data[1] = cmd & 0xff;
    qDebug() << QString("write data: 0x%1%2").arg(data[0], 2, 16, QChar('0')).arg(data[1], 2, 16, QChar('0'));
    return I2C_write_bytes(data, 2);
}

eError MainWindow::I2C_write_bytes(uchar* data, uchar len)
{
    HID_SMBUS_STATUS r;
    HID_SMBUS_S0 s0;
    HID_SMBUS_S1 s1;
    ushort numRetries, bytesRead;

    r = HidSmbus_WriteRequest(m_hidSmbus, m_I2CAddress, data, len);
    qDebug() << "HidSmbus_WriteRequest" << HidSmbus_DecodeErrorStatus(r);
    if(r == HID_SMBUS_SUCCESS) {
        r = HidSmbus_TransferStatusRequest(m_hidSmbus);
        qDebug() << "HidSmbus_TransferStatusRequest" << HidSmbus_DecodeErrorStatus(r);
        if(r == HID_SMBUS_SUCCESS) {
            r = HidSmbus_GetTransferStatusResponse(m_hidSmbus, &s0, &s1, &numRetries, &bytesRead);
            qDebug() << "HidSmbus_GetTransferStatusResponse" << HidSmbus_DecodeErrorStatus(r) << HidSmbus_DecodeTransferStatuses(s0, s1) << numRetries << bytesRead;
        }
    }
    if(r == HID_SMBUS_SUCCESS) {
        return NO_ERROR;
    } else {
        return ERROR_COMM;
    }
}

eError MainWindow::read_bytes(uchar* data, uchar data_len)
{
    eError ret = ERROR_COMM;
    uchar buffer[61];
    uchar read_len;
    uchar readed_len = 0;
    HID_SMBUS_STATUS r;
    HID_SMBUS_S0 status;

    r = HidSmbus_ReadRequest(m_hidSmbus, m_IIC_ADDR, data_len);
    qDebug() << "HidSmbus_ReadRequest" << HidSmbus_DecodeErrorStatus(r);
    if(r == HID_SMBUS_SUCCESS) {
        do {
            QThread::msleep(1);
            HidSmbus_ForceReadResponse(m_hidSmbus, data_len);
            r = HidSmbus_GetReadResponse(m_hidSmbus, &status, buffer, sizeof(buffer), &read_len);
            qDebug() << "HidSmbus_GetReadResponse" << HidSmbus_DecodeErrorStatus(r) << HidSmbus_DecodeTransferStatus(status) << read_len;
            if(r != HID_SMBUS_SUCCESS || status == HID_SMBUS_S0_ERROR)
                break;
            else if(read_len > 0) {
                memcpy(&data[readed_len], buffer, read_len);
                readed_len += read_len;
            }
        }while(readed_len != data_len);
        ret = NO_ERROR;
    }
    return ret;
}

uchar MainWindow::crc8(const uchar* data, int len) {

    const uchar POLYNOMIAL = 0x31;
    uchar crc = 0xFF;

    for (int j = len; j; --j) {
        crc ^= *data++;

        for (int i = 8; i; --i) {
            crc = (crc & 0x80)
                  ? (crc << 1) ^ POLYNOMIAL
                  : (crc << 1);
        }
    }
    return crc;
}

QString MainWindow::HidSmbus_DecodeErrorStatus(HID_SMBUS_STATUS status)
{
    QString str;

    switch (status) {
    case HID_SMBUS_SUCCESS:					str = "HID_SMBUS_SUCCESS";					break;
    case HID_SMBUS_DEVICE_NOT_FOUND:		str = "HID_SMBUS_DEVICE_NOT_FOUND";		break;
    case HID_SMBUS_INVALID_HANDLE:			str = "HID_SMBUS_INVALID_HANDLE";			break;
    case HID_SMBUS_INVALID_DEVICE_OBJECT:	str = "HID_SMBUS_INVALID_DEVICE_OBJECT";	break;
    case HID_SMBUS_INVALID_PARAMETER:		str = "HID_SMBUS_INVALID_PARAMETER";		break;
    case HID_SMBUS_INVALID_REQUEST_LENGTH:	str = "HID_SMBUS_INVALID_REQUEST_LENGTH";	break;
    case HID_SMBUS_READ_ERROR:				str = "HID_SMBUS_READ_ERROR";				break;
    case HID_SMBUS_WRITE_ERROR:				str = "HID_SMBUS_WRITE_ERROR";				break;
    case HID_SMBUS_READ_TIMED_OUT:			str = "HID_SMBUS_READ_TIMED_OUT";			break;
    case HID_SMBUS_WRITE_TIMED_OUT:			str = "HID_SMBUS_WRITE_TIMED_OUT";			break;
    case HID_SMBUS_DEVICE_IO_FAILED:		str = "HID_SMBUS_DEVICE_IO_FAILED";		break;
    case HID_SMBUS_DEVICE_ACCESS_ERROR:		str = "HID_SMBUS_DEVICE_ACCESS_ERROR";		break;
    case HID_SMBUS_DEVICE_NOT_SUPPORTED:	str = "HID_SMBUS_DEVICE_NOT_SUPPORTED";	break;
    case HID_SMBUS_UNKNOWN_ERROR:			str = "HID_SMBUS_UNKNOWN_ERROR";			break;
    default:								str = "Unknown Status";
    }
    return str;
}

QString MainWindow::HidSmbus_DecodePower(HID_SMBUS_STATUS status)
{
    QString str;

    switch (status) {
    case HID_SMBUS_BUS_POWER:			    str = "HID_SMBUS_BUS_POWER";		break;
    case HID_SMBUS_SELF_POWER_VREG_DIS:		str = "HID_SMBUS_SELF_POWER_VREG_DIS";		break;
    case HID_SMBUS_SELF_POWER_VREG_EN:		str = "HID_SMBUS_SELF_POWER_VREG_EN";		break;
    default:							    str = "Unknown Status";
    }

    return str;
}

QString MainWindow::HidSmbus_DecodeTransferStatus(HID_SMBUS_S0 status0)
{
    QString str;

    switch (status0) {
    case HID_SMBUS_S0_IDLE:			str = "Idle";				break;
    case HID_SMBUS_S0_BUSY:			str = "Busy";				break;
    case HID_SMBUS_S0_COMPLETE:		str = "Complete";			break;
    case HID_SMBUS_S0_ERROR:		str = "Error";				break;
    default:						str = "Unknown Status";	break;
    }

    return str;
}

char gStatusString[256];

QString MainWindow::HidSmbus_DecodeTransferStatuses(HID_SMBUS_S0 status0, HID_SMBUS_S1 status1)
{
    QString str;

    switch (status0) {
    case HID_SMBUS_S0_IDLE:			str = "Idle";			break;
    case HID_SMBUS_S0_BUSY:			str = "Busy - ";		break;
    case HID_SMBUS_S0_COMPLETE:		str = "Complete";		break;
    case HID_SMBUS_S0_ERROR:		str = "Error - ";		break;
    default:						str = "Unknown Status";	break;
    }

    if (status0 == HID_SMBUS_S0_BUSY) {
        switch (status1) {
        case HID_SMBUS_S1_BUSY_ADDRESS_ACKED:	str += "Address Acked";		break;
        case HID_SMBUS_S1_BUSY_ADDRESS_NACKED:	str += "Address Nacked";	break;
        case HID_SMBUS_S1_BUSY_READING:			str += "Read in Progress";	break;
        case HID_SMBUS_S1_BUSY_WRITING:			str += "Write in Progress";	break;
        default:								str += "Unknown Status";	break;
        }
    }
    else if (status0 == HID_SMBUS_S0_ERROR) {
        switch (status1)
        {
        case HID_SMBUS_S1_ERROR_TIMEOUT_NACK:			str += "Timeout (Address Nacked)";	break;
        case HID_SMBUS_S1_ERROR_TIMEOUT_BUS_NOT_FREE:	str += "Timeout (Bus Not Free)";	break;
        case HID_SMBUS_S1_ERROR_ARB_LOST:				str += "Arbitration Lost";			break;
        case HID_SMBUS_S1_ERROR_READ_INCOMPLETE:		str += "Read Incomplete";			break;
        case HID_SMBUS_S1_ERROR_WRITE_INCOMPLETE:		str += "Write Incomplete";			break;
        case HID_SMBUS_S1_ERROR_SUCCESS_AFTER_RETRY:	str += "Success After Retries";		break;
        default:										str += "Unknown Status";			break;
        }
    }

    return str;
}

void MainWindow::on_readButton_clicked()
{
    uchar data[6];
    float temp = 0.0, hum = 0.0;
    memset(data, 0, sizeof(data));
    if (read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum) != NO_ERROR) {
        qDebug() << "read temp failed!!";
    } else {
        qDebug() << "read data :";
        qDebug() << "temperature = " << temp << "℃";
        qDebug() << "humidity = " << hum << "%";
    }
}

