#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialdevice.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(onOpen()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(onSend()));
    connect(ui->rescueButton, SIGNAL(clicked()), this, SLOT(onRescue()));
    connect(ui->refreshPorts, SIGNAL(clicked()), this, SLOT(onRefresh()));

    m_serialDevice = new SerialDevice();
    m_portStatus = PortStatus::closed;
    ui->openButton->setText(tr("connect"));
    ui->sendButton->setDisabled(true);
    ui->rescueButton->setDisabled(true);
    onRefresh();
}

MainWindow::~MainWindow() {
    delete ui;
    if (m_serialDevice != nullptr) {
        closeDevice();
        delete m_serialDevice;
        m_serialDevice = nullptr;
    }
}

void MainWindow::onOpen() {
    switch (m_portStatus) {
        case PortStatus::closed:
        case PortStatus::error:
            openDevice();
            break;
        case PortStatus::open:
            closeDevice();
            break;
    }
}

void MainWindow::onSend() {
    QString sendStr = ui->sendTextEdit->toPlainText();
    if (!sendStr.isEmpty()) {
        m_serialDevice->getSerialHandle()->write(sendStr.toUtf8());
        std::cout << "send" << std::endl;
    }
}

void MainWindow::onReadBuf() {
    // m_serialDevice->ReceivePacket();
    std::cout << "read" << std::endl;
}

void MainWindow::onRefresh() {
    closeDevice();
    ui->portsList->clear();
    QList<QSerialPortInfo> ports = availablePorts();
    qSort(ports.begin(), ports.end(), [](const QSerialPortInfo &s1, const QSerialPortInfo &s2) {
        return s1.portName() < s2.portName();
    });
    for (const QSerialPortInfo &port : ports) {
        ui->portsList->addItem(port.portName());
    }
}

void MainWindow::onRescue() {
}

void MainWindow::openDevice() {
    QString deviceName = ui->portsList->currentText();
    m_serialDevice->openDevice(deviceName);
    if (m_serialDevice->isDeviceOpened()) {
        m_portStatus = PortStatus::open;
        ui->openButton->setText(tr("Disconnect"));
        ui->sendButton->setDisabled(false);
        ui->rescueButton->setDisabled(false);
        std::cout << "open" << std::endl;

    } else {
        m_portStatus = PortStatus::error;
        std::cout << "error" << std::endl;
        ui->sendButton->setDisabled(true);
        ui->rescueButton->setDisabled(true);
    }
}

void MainWindow::closeDevice() {
    if (m_serialDevice->isDeviceOpened()) {
        m_serialDevice->closeDevice();
        m_portStatus = PortStatus::closed;
        ui->openButton->setText(tr("connect"));
        std::cout << "closed" << std::endl;
        ui->sendButton->setDisabled(true);
        ui->rescueButton->setDisabled(true);
    }
}

QList<QSerialPortInfo> MainWindow::availablePorts() {
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
#if QT_VERSION == 0x050301 && defined(Q_OS_WIN)
    // workaround to QTBUG-39748 (https://bugreports.qt-project.org/browse/QTBUG-39748)
    // Qt 5.3.1 reports spurious ports with an empty description...
    QMutableListIterator<QSerialPortInfo> i(ports);
    while (i.hasNext()) {
        if (i.next().description().isEmpty()) {
            i.remove();
        }
    }
#endif
    return ports;
}