/**
 * Created by bfyoung on 2018/3/7.
 */

#ifndef UPLOADER_SERIALPORTWIDGET_H
#define UPLOADER_SERIALPORTWIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
    class SerialPortWidget;
}
class QSerialPort;

class SerialPortWidget : public QWidget
{
    Q_OBJECT
    enum class PortStatus {
        open, closed, error
    };
public:
    explicit SerialPortWidget(QWidget *parent = 0);
    ~SerialPortWidget();

public:
    void connectDevice();
    void disconnectDevice();

signals:
    void deviceConnected();
public slots:
    void getSerialPorts();
    void onConnect();
    void onSend();
    void onRead();
private:
    Ui::SerialPortWidget *ui;
    QSerialPort *m_serialDevice;
    PortStatus m_portStatus;
};
#endif //UPLOADER_SERIALPORTWIDGET_H
