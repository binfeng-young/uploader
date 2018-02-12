#ifndef SSPT_H
#define SSPT_H

#include "ssp.h"
#include <condition_variable>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>

class sspt : public ssp {
public:
    sspt(port *info, bool debug = false);
    ~sspt();

    void run();
    void end();
    int packets_Available();
    int read_Packet(void *);
    bool sendData(uint8_t *buf, uint16_t size);

private:
    uint8_t *mbuf;
    uint16_t msize;
    std::queue<std::vector<unsigned char>> queue;
    std::mutex mutex;
    std::mutex sendbufmutex;
    bool endthread;
    bool datapending;
    uint16_t sendstatus;
    uint16_t receivestatus;
    std::condition_variable sendwait;
    std::mutex msendwait;
    bool debug;
    std::thread *m_thread;
    void pfCallBack(uint8_t *, uint16_t) override;
};

#endif // SSPT_H
