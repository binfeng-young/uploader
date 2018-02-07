/**
 * Created by bfyoung on 2018/2/5.
 */

#include "serialdevice.h"
#include <QDebug>

SerialDevice::SerialDevice(QObject *parent)
    : m_serialHandle(nullptr), m_deviceOpened(false) {
}


SerialDevice::SerialDevice(const QString &deviceName) {
    openDevice(deviceName);
}

SerialDevice::~SerialDevice() {
    closeDevice();
}


QSerialPort *SerialDevice::openDevice(const QString &deviceName) {
    closeDevice();
    m_serialHandle = new QSerialPort(deviceName);
    if (m_serialHandle->open(QIODevice::ReadWrite)) {
        if (m_serialHandle->setBaudRate(QSerialPort::Baud115200)
            && m_serialHandle->setDataBits(QSerialPort::Data8)
            && m_serialHandle->setParity(QSerialPort::NoParity)
            && m_serialHandle->setStopBits(QSerialPort::OneStop)
            && m_serialHandle->setFlowControl(QSerialPort::NoFlowControl)) {
            qDebug() << "Serial telemetry running at " << QSerialPort::Baud115200;
            //connect(m_serialHandle, &QSerialPort::readyRead, this, &SerialDevice::ReceivePacket);
            m_deviceOpened = true;
        }
        m_serialHandle->setDataTerminalReady(true);
        return m_serialHandle;
    }
    m_serialHandle->deleteLater();
    m_serialHandle = nullptr;
    return nullptr;
}

void SerialDevice::closeDevice() {
    if (nullptr != m_serialHandle) {
        //disconnect(m_serialHandle, &QSerialPort::readyRead, this, &SerialDevice::ReceivePacket);
        m_serialHandle->deleteLater();
        m_serialHandle = nullptr;
    }

    m_deviceOpened = false;
}

bool SerialDevice::isDeviceOpened() {
    return m_deviceOpened;
}

QSerialPort *SerialDevice::getSerialHandle() {
    return m_serialHandle;
}

QByteArray SerialDevice::readAll() {
    return m_serialHandle->readAll();
}

int SerialDevice::sendData(uint8_t *buf, uint16_t size) {
    emit(test());
    return true;
}

int SerialDevice::read_Packet(void *data) {
}