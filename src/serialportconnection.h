/**
 * Created by bfyoung on 2018/2/5.
 */

#ifndef SERIALPORT_SERIALCONNECTION_H
#define SERIALPORT_SERIALCONNECTION_H

#include <QThread>

class SerialPortConnection :public QThread{
    Q_OBJECT
public:
    SerialPortConnection(QObject *parent = 0);

    SerialPortConnection(const QString &deviceName);

    virtual ~SerialPortConnection();

    bool isDeviceOpened() { return m_deviceOpened; };

    int read_Packet(void *data);

public slots:

signals:
    void test();
private:
    bool m_deviceOpened;
protected:


};


#endif //SERIALPORT_SERIALCONNECTION_H
