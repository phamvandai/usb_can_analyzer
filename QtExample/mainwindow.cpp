#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

MainWindow *pxObj = NULL;

void RxCallback(CanFrameType *pxFrame)
{
    qDebug() << "Rx - ID:" << hex << pxFrame->u32FrameId << ", DLC:" << hex << pxFrame->u8DLC;

    QString str;
    for(int idx = 0; idx < pxFrame->u8DLC; idx++) {
        str += QString::number(pxFrame->pu8Data[idx],16);
        str += "-";
    }

    qDebug() << str;
}

void StatusCallback(uint8_t u8TxErr, uint8_t u8RxErr)
{
    qDebug() << "TxErr: " << u8TxErr << ", RxErr: " << u8RxErr;
}

CanUsbReturnType SendUartCallback(uint8_t *pu8Data, uint8_t u8Len)
{
    return pxObj->CanUsb_SendUart(pu8Data, u8Len);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_opened(false)
{
    ui->setupUi(this);

    int idx = 0;
    for(QSerialPortInfo port: QSerialPortInfo::availablePorts()) {
        m_ports += port.portName();
        ui->listPort->insertItem(idx++, port.portName() + ":" + port.description());
    }

    m_CanCfg.Mode = CAN_MODE_NORMAL;
    m_CanCfg.Type = CAN_TYPE_STD;
    m_CanCfg.Baudrate = CAN_BAUDRATE_250k;
    m_CanCfg.u32FilterId = 0x0;
    m_CanCfg.u32MaskId = 0x0;

    pvHandle = CanUsb_Init(RxCallback, StatusCallback, SendUartCallback);

    pxObj = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_bt_OpenCom_released()
{
    if(m_opened == false) {
        m_serial.setPortName(m_ports[ui->listPort->currentIndex()]);
        m_serial.setBaudRate(2000000);
        if(m_serial.open(QIODevice::ReadWrite) == false) {
            QMessageBox::information(this, tr("Error"), "Open COM port error");
            return;
        }

        connect(&m_serial, &QSerialPort::readyRead, this, &MainWindow::ReadUartData);

        m_opened = true;
        ui->listPort->setEnabled(false);
        ui->bt_OpenCom->setText("Close");

        CanUsb_Configure(pvHandle, &m_CanCfg);
        uint32_t au32List[] = {0x100};
        CanUsb_EnableReceiveSelected(pvHandle, au32List, 1);
    }
    else {
        m_serial.close();
        m_opened = false;
        ui->listPort->setEnabled(true);
        ui->bt_OpenCom->setText("Open");
    }
}

void MainWindow::ReadUartData(void)
{
    QByteArray data = m_serial.readAll();
    for(int idx = 0; idx < data.size(); idx++) {
        CanUsb_ReceiveByteUart(pvHandle, data.at(idx));
    }
}

CanUsbReturnType MainWindow::CanUsb_SendUart(uint8_t *pu8Data, uint8_t u8Len)
{
    if(!m_opened) {
        return CAN_USB_ERR;
    }

    m_serial.write((char*) pu8Data, u8Len);

    return CAN_USB_OK;
}

void MainWindow::on_bt_Send_released()
{
    uint8_t au8Data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    CanFrameType frm;
    frm.Type = CAN_TYPE_STD;
    frm.u8DLC = 8;
    frm.u32FrameId = 0x200;
    frm.pu8Data = au8Data;

    CanUsb_Send(pvHandle, &frm);
}
