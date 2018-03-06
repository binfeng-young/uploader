//
// Created by root on 2/11/18.
//

#ifndef UPLOADER_SERIALPORT_H
#define UPLOADER_SERIALPORT_H
enum {
    UART_DATAT_BIT_7 = 7,
    UART_DATAT_BIT_8
};

enum {
    UART_STOP_BIT_1 = 0,
    UART_STOP_BIT_2
};

#include <string>

class SerialPort {
    enum status { opened, closed, error };
public:
    SerialPort();
    virtual ~SerialPort();

    bool openPort(const std::string & deviceName);
    void closePort();
    int readBuff(char* p_data_buf,int buf_size);
    int writeBuff(char *p_data_buf, int buf_size);
    bool isOpen();
private:
    int m_fd;
    status m_status;
};


#endif //UPLOADER_SERIALPORT_H
