/**
 * Created by bfyoung on 2018/2/5.
 */

#include "serialportconnection.h"
#include <QDebug>

SerialPortConnection::SerialPortConnection(QObject *parent)
    : m_serialHandle(nullptr), m_deviceOpened(false) {
}


SerialPortConnection::SerialPortConnection(const QString &deviceName) {
}

SerialPortConnection::~SerialPortConnection() {
}

