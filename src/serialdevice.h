/**
 * Created by bfyoung on 2018/2/5.
 */

#ifndef SERIALPORT_SERIALCONNECTION_H
#define SERIALPORT_SERIALCONNECTION_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class SerialDevice {
public:
    SerialDevice(QObject *parent = 0);

    SerialDevice(const QString &deviceName);

    virtual ~SerialDevice();

    QSerialPort *openDevice(const QString &deviceName);

    void closeDevice();

    bool isDeviceOpened();

    int sendData(uint8_t *data, uint16_t size);

    QByteArray readAll();

    QSerialPort *getSerialHandle();

    int read_Packet(void *data);

public slots:

    void ReceivePacket();

private:
    bool m_deviceOpened;
private:
    QSerialPort *m_serialHandle;

};


#endif //SERIALPORT_SERIALCONNECTION_H
