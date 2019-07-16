#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include "..\CanUsb.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void ReadUartData(void);

    CanUsbReturnType CanUsb_SendUart(uint8_t *pu8Data, uint8_t u8Len);

private slots:
    void on_bt_OpenCom_released();

    void on_bt_Send_released();

private:
    Ui::MainWindow *ui;
    QStringList     m_ports;
    bool            m_opened;
    QSerialPort     m_serial;
    CanConfigType   m_CanCfg;
    CanUsbHandleType pvHandle;
};

#endif // MAINWINDOW_H
