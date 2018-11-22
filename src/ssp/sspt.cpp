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

        if (!writeArray_.empty() && receivestatus == SSP_TX_IDLE) {
            std::lock_guard<std::mutex> lock(sendbufmutex);
            std::string str = writeArray_.front();
            ssp_SendData(writeArray_.front());
            writeArray_.pop();
        }
        //sendbufmutex.unlock();
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
    sendbufmutex.lock();
    std::string writeBuff(buf, buf + size);
    writeArray_.push(writeBuff);
    sendbufmutex.unlock();
//
//    if (datapending) {
//        return false;
//    }
//
//
//    datapending = true;
//    mbuf  = buf;
//    msize = size;
//    // TODO why do we wait 10 seconds ? why do we then ignore the timeout ?
//    // There is a ssp_SendDataBlock method...
//    //msendwait.lock();
    std::unique_lock<std::mutex> lck(msendwait);
    //sendwait.wait(&msendwait, 1000);
    sendwait.wait_for(lck, std::chrono::seconds(10));

    return true;
}

void sspt::pfCallBack(uint8_t *buf, uint16_t size)
{
    if (debug) {
        std::cout<< "receive callback" << buf[0] << buf[1] << buf[2] << buf[3] << buf[4] << "queue size=" << queue.size() << std::endl;
    }
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
    if (debug) {
        std::cout << "SSP RX " << arr << std::endl;
    }
    mutex.unlock();
    return arr.size();
}

void sspt::end()
{
    endthread = true;
    waitStop();
}
