#include "port.h"

#ifdef USE_QSERIALPORT
#include <QSerialPort>
#include <QString>
#else
#include "serialport.h"
#endif
using namespace std;
port::port(const string &name, bool debug) : mstatus(port::closed), debug(debug)
{
    time.start();
#ifdef USE_QSERIALPORT
    sport = new QSerialPort(QString::fromStdString(name));
    if (sport->open(QIODevice::ReadWrite)) {
        if (sport->setBaudRate(QSerialPort::Baud115200)
            && sport->setDataBits(QSerialPort::Data8)
            && sport->setParity(QSerialPort::NoParity)
            && sport->setStopBits(QSerialPort::OneStop)
            && sport->setFlowControl(QSerialPort::NoFlowControl)) {
            mstatus = port::open;
        }
    }
#else
    sport = new SerialPort();
    if (sport->openPort(name))  {
        mstatus = port::open;
    }
#endif
    else {
        mstatus = port::error;
    }
}

port::port(SerialPort* serialPort, bool debug) : mstatus(port::closed), debug(debug){
    time.start();
    sport = serialPort;
    if (sport->isOpen()) {
        mstatus = port::open;
    } else {
        mstatus = port::error;
    }
}

port::~port()
{
    sport->closePort();
}

port::portstatus port::status()
{
    return mstatus;
}

int16_t port::pfSerialRead(void)
{
    char c[1];
#ifdef USE_QSERIALPORT
    // TODO why the wait ? (gcs uploader dfu does not have it)
    sport->waitForBytesWritten(1);
    if (sport->bytesAvailable() || sport->waitForReadyRead(0)) {
        sport->read(c, 1);
    }
#else
    if (sport->readBuff(c, 1) > 0) {}
#endif
    else {
        return -1;
    }
    return (uint8_t)c[0];
}

void port::pfSerialWrite(uint8_t c)
{
    char cc[1];

    cc[0] = c;
#ifdef USE_QSERIALPORT
    sport->write(cc, 1);
    sport->waitForBytesWritten(1);
#else
    sport->writeBuff(cc, 1);
#endif

}

uint32_t port::pfGetTime()
{
    return time.elapsed();
}
