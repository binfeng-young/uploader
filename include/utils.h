//
// Created by bfyoung on 2/22/18.
//

#ifndef UPLOADER_TIME_H
#define UPLOADER_TIME_H

#include <chrono>
#include <thread>
#include <future>
#include <iostream>
class Time {
public:
    void start() { m_start = std::chrono::high_resolution_clock::now(); }

    unsigned int elapsed() {
        return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now() - m_start).count());
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
};

class Thread {
public:
    Thread() : m_thread(nullptr), m_task(nullptr) {

    }
    virtual ~Thread() {
        if (nullptr != m_thread) {
           // stop();
            delete m_thread;
            m_thread = nullptr;
        }
        if (nullptr != m_task) {
            delete m_task;
            m_task = nullptr;
        }
    };
    void start() {
        if (isRunning()) {
            return;
        }
        m_task = new std::packaged_task<void()>(std::bind(&Thread::run, this));
        m_future = m_task->get_future();
        m_thread = new std::thread(std::move(*m_task));
    };
//    void stop() {
//        if (nullptr != m_thread) {
//            if (m_thread->joinable()) {
//                m_thread->join();
//            }
//            delete m_thread;
//            m_thread = nullptr;
//        }
//    };
    void waitStop() {
        if (m_thread->joinable()) {
            m_thread->join();
        }
    }
    bool isRunning() {
        if (nullptr == m_thread) {
            return false;
        }
        return m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready;
    }
protected:
    virtual void run() {}
private:
    std::thread *m_thread;
    std::packaged_task<void()> *m_task;
    std::future<void> m_future;
};


#endif //UPLOADER_TIME_H
