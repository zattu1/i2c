#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "SLABCP2112.h"

#define VID 0x10C4
#define PID 0xEA90
#define DEFAULT_IIC_ADDR     0x44
#define	NACK_ON_ADDR			2
#define	CMD_BREAK			0x3093
#define	CMD_SOFT_RST		0x30A2
#define	CMD_ENABLE_HEAT		0x306D
#define	CMD_DISABLE_HEAT	0x3066
#define	CMD_READ_SREG		0xF32D
#define	CMD_CLEAR_SREG		0x3041
#define	CMD_FETCH_DATA		0xE000
#define HIGH_REP_WITH_STRCH 0x2C06

typedef enum {
    NO_ERROR = 0,
    ERROR_PARAM = -1,
    ERROR_COMM = -2,
    ERROR_OTHERS = -128,
} eError;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    virtual void showEvent(QShowEvent *e) override;
    virtual void closeEvent(QCloseEvent *e) override;

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_readButton_clicked();

private:
    BOOL Connect(DWORD deviceNum);
    void UpdateDeviceInformation(bool connected);
    BOOL Disconnect();
    void SetFromDevice();
    void GetSmbusConfig(bool silent);
    void GetTimeouts(bool silent);
    void ReadLatch(bool silent);
    void GetUsbConfig(bool silent);
    void GetLock(bool silent);
    void GetManufacturer(bool silent);
    void GetProduct(bool silent);
    void GetSerial(bool silent);
    QString HidSmbus_DecodeErrorStatus(HID_SMBUS_STATUS status);
    QString HidSmbus_DecodePower(HID_SMBUS_STATUS status);
    QString HidSmbus_DecodeTransferStatus(HID_SMBUS_S0 status0);
    QString HidSmbus_DecodeTransferStatuses(HID_SMBUS_S0 status0, HID_SMBUS_S1 status1);
    eError init();
    eError soft_reset();
    eError read_meas_data_single_shot(ushort cfg_cmd, float *temp, float *hum);
    float get_temp(ushort temp);
    float get_hum(ushort hum);
    eError send_command(ushort cmd);
    eError I2C_write_bytes(uchar *data, uchar len);
    eError read_bytes(uchar *data, uchar data_len);
    uchar crc8(const uchar *data, int len);

private:
    Ui::MainWindow *ui;

    HID_SMBUS_DEVICE m_hidSmbus;
    uchar m_IIC_ADDR;
    uchar m_I2CAddress;
};
#endif // MAINWINDOW_H
