#include <cstring>
#include "sspt.h"
using namespace std;
sspt::sspt(port *info, bool debug) : ssp(info, debug), endthread(false), datapending(false), debug(debug)
{}

sspt::~sspt()
{
}

void sspt::run()
{
    // TODO the loop is non blocking and will spin as fast as the CPU allows
    while (!endthread) {
        receivestatus = ssp_ReceiveProcess();
        sendstatus = ssp_SendProcess();
        sendbufmutex.lock();
        if (datapending && receivestatus == SSP_TX_IDLE) {
            ssp_SendData(mbuf, msize);
            datapending = false;
        }
        sendbufmutex.unlock();
        if (sendstatus == SSP_TX_ACKED) {
            sendwait.notify_all();
        }
    }
}

bool sspt::sendData(uint8_t *buf, uint16_t size)
{
//    if (debug) {
//        QByteArray data((const char *)buf, size);
//        qDebug() << "SSP TX " << data.toHex();
//    }
    if (datapending) {
        return false;
    }
    sendbufmutex.lock();
    datapending = true;
    mbuf  = buf;
    msize = size;
    sendbufmutex.unlock();
    // TODO why do we wait 10 seconds ? why do we then ignore the timeout ?
    // There is a ssp_SendDataBlock method...
    //msendwait.lock();
    std::unique_lock<std::mutex> lck(msendwait);
    //sendwait.wait(&msendwait, 1000);
    sendwait.wait_for(lck, std::chrono::seconds(1));

    return true;
}

void sspt::pfCallBack(uint8_t *buf, uint16_t size)
{
//    if (debug) {
//        qDebug() << "receive callback" << buf[0] << buf[1] << buf[2] << buf[3] << buf[4] << "queue size=" << queue.count();
//    }
    std::string data(buf, buf + size);
    mutex.lock();
    queue.push(data);
    mutex.unlock();
}

int sspt::packets_Available()
{
    return queue.size();
}

int sspt::read_Packet(void *data)
{
    mutex.lock();
    if (queue.size() == 0) {
        mutex.unlock();
        return -1;
    }
    std::string arr = queue.front();
    queue.pop();
    std::memcpy(data, (uint8_t *)arr.data(), arr.size());
//    if (debug) {
//        qDebug() << "SSP RX " << arr.toHex();
//    }
    mutex.unlock();
    return arr.size();
}

void sspt::end()
{
    endthread = true;
    waitStop();
}
