//
// Created by root on 2/11/18.
//

#include "serialport.h"
#include <cstring>
extern "C" {
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
}

SerialPort::SerialPort() : m_fd(-1), m_status(SerialPort::closed)
{

}

SerialPort::~SerialPort() {
    closePort();
}


int set_opt(int fd,int wSpeed, int wBits, char cEvent, int wStop)
{
    struct termios newtio, oldtio;

    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(fd,&oldtio) != 0) {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));

    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL|CREAD;     //CLOCAL:忽略modem控制线  CREAD：打开接受者
    newtio.c_cflag &= ~CSIZE;           //字符长度掩码。取值为：CS5，CS6，CS7或CS8

    /*设置停止位*/
    switch(wBits)
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    /*设置奇偶校验位*/
    switch( cEvent )
    {
        case 'O'://奇数
            newtio.c_cflag |= PARENB; //允许输出产生奇偶信息以及输入到奇偶校验
            newtio.c_cflag |= PARODD;  //输入和输出是奇及校验
            newtio.c_iflag |= (INPCK|ISTRIP); // INPACK:启用输入奇偶检测；ISTRIP：去掉第八位
            break;
        case 'E'://偶数
            newtio.c_iflag |= (INPCK|ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N'://无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;
    }

    /*设置波特率*/
    switch( wSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        case 1500000:
            cfsetispeed(&newtio, B1500000);
            cfsetospeed(&newtio, B1500000);
            break;
        default:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
    }

    /*设置停止位*/

    if (wStop == 1)
        newtio.c_cflag &= ~CSTOPB; //CSTOPB:设置两个停止位，而不是一个
    else if (wStop == 2)
        newtio.c_cflag |= CSTOPB;

    /*设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0; //VTIME:非cannoical模式读时的延时，以十分之一秒位单位
    newtio.c_cc[VMIN] = 0; //VMIN:非canonical模式读到最小字符数

    /*处理未接收字符*/
    tcflush(fd,TCIFLUSH); // 改变在所有写入 fd 引用的对象的输出都被传输后生效，所有已接受但未读入的输入都在改变发生前丢弃。
    if ((tcsetattr(fd,TCSANOW,&newtio))!=0) //TCSANOW:改变立即发生
    {
        //bv_logw("com set error");
        return -1;
    }
    //bv_logi("set done!\n\r");
    return 0;
}

bool SerialPort::openPort(const std::string & deviceName)
{
    char dev[20];
    sprintf(dev, "/dev/%s",deviceName.c_str());
    printf("dev :%s\n",dev);
    int wRlt = 0;

    m_fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (m_fd < 0)
    {
        printf("uart %s open err!",dev);
        m_status = SerialPort::error;
        return false;
    }

    //判断串口的状态是否为阻塞状态
    if (fcntl(m_fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        m_status = SerialPort::error;
        return false;
    }
    else
    {
        printf("fcntl=%d\n",fcntl(m_fd, F_SETFL,0));
    }

    if (isatty(STDIN_FILENO)==0)
    {
        printf("standard input is not a terminal device.\n");
        m_status = SerialPort::error;
        return false;
    }
    else
    {
        printf("isatty success!\n");
    }

    printf("fd-open=%d\n",m_fd);
    wRlt = set_opt(m_fd, 115200, UART_DATAT_BIT_8, 'N', UART_STOP_BIT_2);
    if (wRlt < 0)
    {
        printf("set_opt  err!\n");
        m_status = SerialPort::error;
        return false;
    }
    printf("opened\n");
    m_status = SerialPort::opened;
    return true;
}

void SerialPort::closePort()
{
    if (m_fd > 0) {
        close(m_fd);
    }
    m_status = SerialPort::closed;
}

int SerialPort::readBuff(char *p_data_buf, int buf_size) {
    int recvlen = 0;
    fd_set fs_read;
    timeval time {0, 10};
    char* data_buf = p_data_buf;
    FD_ZERO(&fs_read);
    FD_SET(m_fd, &fs_read);

    /*使用select实现串口的多路通信*/
    int fs_sel = select(m_fd + 1, &fs_read, NULL, NULL, &time);
    if (fs_sel) {
        while(1) {
            int len = read(m_fd, data_buf, buf_size);
            if (len > 0) {
                recvlen += len;
                data_buf += len;
            }
            else {
                break;
            }
            if (len >= buf_size) break;
        }
        //printf("%d\n", recvlen);
    }
    else
    {
        //printf("uart read time out\n.");
        recvlen = -1;
    }

    return recvlen;
}

int SerialPort::writeBuff(char *p_data_buf, int buf_size) {
    int len = write(m_fd, p_data_buf, buf_size);
    if (len == buf_size) {
        return len;
    }
    else {
        tcflush(m_fd,TCOFLUSH);
        printf("uart write err.\n");
        return -1;
    }
}

bool SerialPort::isOpen() {
    return m_status == SerialPort::opened;
}
