#include "serialport.h"
#include "port.h"
using namespace std;
port::port(const string &name, bool debug) : mstatus(port::closed), debug(debug)
{
    time.start();
    sport = new SerialPort();
    if (sport->openPort(name))  {
        mstatus = port::open;
    } else {
        mstatus = port::error;
    }
}

port::~port()
{
    delete sport;
}

port::portstatus port::status()
{
    return mstatus;
}

int16_t port::pfSerialRead(void)
{
    char c[1];

    if (sport->readBuff(c, 1) > 0) {
//        if (debug) {
//            if (((uint8_t)c[0]) == 0xe1 || rxDebugBuff.count() > 50) {
//                qDebug() << "PORT R " << rxDebugBuff.toHex();
//                rxDebugBuff.clear();
//            }
//            rxDebugBuff.append(c[0]);
//        }
    } else {
        return -1;
    }
    return (uint8_t)c[0];
}

void port::pfSerialWrite(uint8_t c)
{
    char cc[1];

    cc[0] = c;
    sport->writeBuff(cc, 1);
//    if (debug) {
//        if (((uint8_t)cc[0]) == 0xe1 || rxDebugBuff.count() > 50) {
//            qDebug() << "PORT T " << txDebugBuff.toHex();
//            txDebugBuff.clear();
//        }
//        txDebugBuff.append(cc[0]);
//    }
}

uint32_t port::pfGetTime()
{
    return time.elapsed();
}
