//
// Created by bfyoung on 2/22/18.
//

#ifndef UPLOADER_TIME_H
#define UPLOADER_TIME_H

#include <chrono>

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


#endif //UPLOADER_TIME_H
