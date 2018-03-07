/**
 * Created by bfyoung on 2018/3/7.
 */

#include "serialportwidget.h"
#include "ui_serialportwidget.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <iostream>
#include <thread>

SerialPortWidget::SerialPortWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialPortWidget),
    m_serialDevice(nullptr),
    m_portStatus(PortStatus::closed)
{
    ui->setupUi(this);
    ui->refreshPorts->setIcon(QIcon(":uploader/images/view-refresh.svg"));
    getSerialPorts();
    connect(ui->refreshPorts, SIGNAL(clicked()), this, SLOT(getSerialPorts()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(onConnect()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(onSend()));
    ui->sendButton->setDisabled(true);
}

SerialPortWidget::~SerialPortWidget()
{
    delete ui;
}

void SerialPortWidget::getSerialPorts()
{
    disconnectDevice();
    // Populate the telemetry combo box:
    ui->portsList->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // sort the list by port number (nice idea from PT_Dreamer :))
    qSort(ports.begin(), ports.end(),  [](const QSerialPortInfo &s1, const QSerialPortInfo &s2) {
        return s1.portName() < s2.portName();
    });
    foreach(QSerialPortInfo port, ports) {
        ui->portsList->addItem(port.portName());
    }

}

void SerialPortWidget::connectDevice()
{
    m_portStatus = PortStatus ::error;
    QString deviceName = ui->portsList->currentText();
    m_serialDevice = new QSerialPort(deviceName);
    if (m_serialDevice->open(QIODevice::ReadWrite)) {
        if (m_serialDevice->setBaudRate(QSerialPort::Baud115200)
            && m_serialDevice->setDataBits(QSerialPort::Data8)
            && m_serialDevice->setParity(QSerialPort::NoParity)
            && m_serialDevice->setStopBits(QSerialPort::OneStop)
            && m_serialDevice->setFlowControl(QSerialPort::NoFlowControl)) {
            m_portStatus = PortStatus ::open;
        }
    }
    if (m_portStatus == PortStatus::open) {
        ui->connectButton->setText(tr("Disconnect"));
        ui->sendButton->setDisabled(false);

        connect(m_serialDevice, SIGNAL(readyRead()), this, SLOT(onRead()));
        std::cout << "open" << std::endl;
//        std::thread([&](){
//            while(true) {
//                char c[1];
//                // TODO why the wait ? (gcs uploader dfu does not have it)
//                m_serialDevice->waitForBytesWritten(1);
//                if (m_serialDevice->bytesAvailable() || m_serialDevice->waitForReadyRead(0)) {
//                    m_serialDevice->read(c, 1);
//                    ui->receiveTextEdit->append(QString(c));
//                }
//                std::this_thread::sleep_for(std::chrono::microseconds(1));
//            }
//        });
    } else {
        std::cout << "error" << std::endl;
        ui->sendButton->setDisabled(true);
    }
}

void SerialPortWidget::disconnectDevice()
{
    if (nullptr != m_serialDevice) {
        disconnect(m_serialDevice, SIGNAL(readyRead()), this, SLOT(onRead()));
        m_serialDevice->deleteLater();
        m_serialDevice = nullptr;
        m_portStatus = PortStatus::closed;
        ui->connectButton->setText(tr("connect"));
        std::cout << "closed" << std::endl;
        ui->sendButton->setDisabled(true);
    }
}


void SerialPortWidget::onConnect()
{
    switch (m_portStatus) {
        case PortStatus::closed:
        case PortStatus::error:
            connectDevice();
            break;
        case PortStatus::open:
            disconnectDevice();
            break;
    }
}

void SerialPortWidget::onSend()
{
    uint8_t toBLCmd[] = {0xb5, 0x62, 0x00, 0x03, 0x4c, 0x00, 0x4c};
    if (m_serialDevice->write(reinterpret_cast<char *>(toBLCmd), 7) <= 0) {
        std::cout << "Could not write serial" << std::endl;
        return;
    }
}

void SerialPortWidget::onRead()
{
    QByteArray arr = m_serialDevice->readAll();
    ui->receiveTextEdit->append(arr);
}