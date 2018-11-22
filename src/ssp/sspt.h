#ifndef SSPT_H
#define SSPT_H

#include "ssp.h"
#include <condition_variable>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include "utils.h"

class sspt : public ssp, public Thread {
public:
    sspt(port *info, bool debug = false);
    ~sspt();

    void end();
    int packets_Available();
    int read_Packet(void *);
    bool sendData(uint8_t *buf, uint16_t size);

private:
    uint8_t *mbuf;
    uint16_t msize;
    std::queue<std::string> queue;
    std::mutex mutex;
    std::mutex sendbufmutex;
    bool endthread;
    bool datapending;
    uint16_t sendstatus;
    uint16_t receivestatus;
    std::condition_variable sendwait;
    std::mutex msendwait;
    std::queue<std::string> writeArray_;
    bool debug;
    void pfCallBack(uint8_t *, uint16_t) override;

protected:
    void run() override;
};

#endif // SSPT_H
