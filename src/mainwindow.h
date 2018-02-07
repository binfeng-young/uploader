#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPortInfo>
class SerialDevice;

namespace Ui {
    class MainWindow;
}
enum class PortStatus {
    open, closed, error
};

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    QList<QSerialPortInfo> availablePorts();

    void openDevice();

    void closeDevice();
public slots:
    void onOpen();
    void onSend();
    void onRefresh();
    void onReadBuf();
    void onRescue();

private:
    Ui::MainWindow *ui;
    SerialDevice *m_serialDevice;
    PortStatus m_portStatus;

};

#endif // MAINWINDOW_H
