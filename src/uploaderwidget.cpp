#include "uploaderwidget.h"

#include "ui_uploaderwidget.h"

//#include <ophid/inc/ophid_usbmon.h>

#include "devicewidget.h"

//#include "rebootdialog.h"
#include "qdfu.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QProgressBar>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

#define DFU_DEBUG true

const int UploaderWidget::BOARD_EVENT_TIMEOUT = 20000;
const int UploaderWidget::AUTOUPDATE_CLOSE_TIMEOUT = 7000;
const int UploaderWidget::REBOOT_TIMEOUT     = 20000;
const int UploaderWidget::ERASE_TIMEOUT      = 20000;
const int UploaderWidget::BOOTLOADER_TIMEOUT = 20000;

TimedDialog::TimedDialog(const QString &title, const QString &labelText, int timeout, QWidget *parent, Qt::WindowFlags flags) :
    QProgressDialog(labelText, tr("Cancel"), 0, timeout, parent, flags), bar(new QProgressBar(this))
{
    setWindowTitle(title);
    setAutoReset(false);
    // open immediately...
    setMinimumDuration(0);
    // setup progress bar
    bar->setRange(0, timeout);
    bar->setFormat(tr("Timing out in %1 seconds").arg(timeout));
    bar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setBar(bar);
}

void TimedDialog::perform()
{
    setValue(value() + 1);
    int remaining = bar->maximum() - bar->value();
    if (remaining > 0) {
        bar->setFormat(tr("Timing out in %1 seconds").arg(remaining));
    } else {
        bar->setFormat(tr("Timed out after %1 seconds").arg(bar->maximum()));
    }
}

ConnectionWaiter::ConnectionWaiter(int targetDeviceCount, int timeout, QWidget *parent) : QObject(parent), eventLoop(this), timer(this), timeout(timeout), elapsed(0), targetDeviceCount(targetDeviceCount), result(ConnectionWaiter::Ok)
{}

int ConnectionWaiter::exec()
{
//    connect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(deviceEvent()));
//    connect(USBMonitor::instance(), SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(deviceEvent()));

    connect(&timer, SIGNAL(timeout()), this, SLOT(perform()));
    timer.start(1000);

    emit timeChanged(0);
    eventLoop.exec();

    return result;
}

void ConnectionWaiter::cancel()
{
    quit();
    result = ConnectionWaiter::Canceled;
}

void ConnectionWaiter::quit()
{
//    disconnect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(deviceEvent()));
//    disconnect(USBMonitor::instance(), SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(deviceEvent()));
    timer.stop();
    eventLoop.exit();
}

void ConnectionWaiter::perform()
{
    ++elapsed;
    emit timeChanged(elapsed);
    int remaining = timeout - elapsed * 1000;
    if (remaining <= 0) {
        result = ConnectionWaiter::TimedOut;
        quit();
    }
}

void ConnectionWaiter::deviceEvent()
{
//    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() == targetDeviceCount) {
//        quit();
//    }
}

int ConnectionWaiter::openDialog(const QString &title, const QString &labelText, int targetDeviceCount, int timeout, QWidget *parent, Qt::WindowFlags flags)
{
    TimedDialog dlg(title, labelText, timeout / 1000, parent, flags);
    ConnectionWaiter waiter(targetDeviceCount, timeout, parent);

    connect(&dlg, SIGNAL(canceled()), &waiter, SLOT(cancel()));
    connect(&waiter, SIGNAL(timeChanged(int)), &dlg, SLOT(perform()));
    return waiter.exec();
}

UploaderWidget::UploaderWidget(QWidget *parent) :
    QWidget(parent), m_config(new Ui::UploaderWidget())
{
    m_config->setupUi(this);
    m_currentIAPStep = IAP_STATE_READY;
    m_dfu = nullptr;
    connect(m_config->bootButton, SIGNAL(clicked()), this, SLOT(systemBoot()));
    connect(m_config->safeBootButton, SIGNAL(clicked()), this, SLOT(systemSafeBoot()));
    connect(m_config->eraseBootButton, SIGNAL(clicked()), this, SLOT(systemEraseBoot()));
    connect(m_config->rescueButton, SIGNAL(clicked()), this, SLOT(systemRescue()));

    getSerialPorts();

    m_config->refreshPorts->setIcon(QIcon(":uploader/images/view-refresh.svg"));

    bootButtonsSetEnable(false);

    connect(m_config->refreshPorts, SIGNAL(clicked()), this, SLOT(getSerialPorts()));
}

bool sortPorts(const QSerialPortInfo &s1, const QSerialPortInfo &s2)
{
    return s1.portName() < s2.portName();
}

/**
   Gets the list of serial ports
 */
void UploaderWidget::getSerialPorts()
{
    QStringList list;

    // Populate the telemetry combo box:
    m_config->telemetryLink->clear();

    //list.append(QString("USB"));
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // sort the list by port number (nice idea from PT_Dreamer :))
    qSort(ports.begin(), ports.end(), sortPorts);
    foreach(QSerialPortInfo port, ports) {
        list.append(port.portName());
    }

    m_config->telemetryLink->addItems(list);
}


QString UploaderWidget::getPortDevice(const QString &friendName)
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo port, ports) {
        if (port.portName() == friendName) {
            return port.portName();
        }
    }
    return "";
}

void UploaderWidget::connectSignalSlot(QWidget *widget)
{
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(uploadStarted()), this, SLOT(uploadStarted()));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(uploadEnded(bool)), this, SLOT(uploadEnded(bool)));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(downloadStarted()), this, SLOT(downloadStarted()));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(downloadEnded(bool)), this, SLOT(downloadEnded(bool)));
}

void UploaderWidget::bootButtonsSetEnable(bool enabled)
{
    m_config->bootButton->setEnabled(enabled);
    m_config->safeBootButton->setEnabled(enabled);

    // this feature is supported only on BL revision >= 4
    bool isBL4 = ((m_dfu != NULL) && (m_dfu->devices[0].BL_Version >= 4));
    m_config->eraseBootButton->setEnabled(isBL4 && enabled);
}

void UploaderWidget::onPhysicalHWConnect()
{
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);
}

/**
   Enables widget buttons if autopilot connected
 */
void UploaderWidget::onAutopilotConnect()
{
    QTimer::singleShot(1000, this, SLOT(populate()));
}

void UploaderWidget::populate()
{
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);

    // Add a very simple widget with Board model & serial number
    // Delete all previous tabs:
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
    }
}

/**
   Enables widget buttons if autopilot disconnected
 */
void UploaderWidget::onAutopilotDisconnect()
{
    bootButtonsSetEnable(true);
    if (m_currentIAPStep == IAP_STATE_BOOTLOADER) {
        m_config->rescueButton->setEnabled(false);
        m_config->telemetryLink->setEnabled(false);
    } else {
        m_config->rescueButton->setEnabled(true);
        m_config->telemetryLink->setEnabled(true);
    }
}

static void sleep(int ms)
{
    QEventLoop eventLoop;
    QTimer::singleShot(ms, &eventLoop, SLOT(quit()));

    eventLoop.exec();
}

void UploaderWidget::systemBoot()
{
    commonSystemBoot(false, false);
}

void UploaderWidget::systemSafeBoot()
{
    commonSystemBoot(true, false);
}

void UploaderWidget::systemEraseBoot()
{
//    switch (confirmEraseSettingsMessageBox()) {
//    case QMessageBox::Ok:
//        commonSystemBoot(true, true);
//        break;
//    case QMessageBox::Help:
//        QDesktopServices::openUrl(QUrl(QString(WIKI_URL_ROOT) + QString("Erase+board+settings"),
//                                       QUrl::StrictMode));
//        break;
//    }
}

void UploaderWidget::rebootWithDialog()
{
    //RebootDialog dialog(this);

    //dialog.exec();
}

/**
 * Tells the system to boot (from Bootloader state)
 * @param[in] safeboot Indicates whether the firmware should use the stock HWSettings
 */
void UploaderWidget::commonSystemBoot(bool safeboot, bool erase)
{
    clearLog();
    bootButtonsSetEnable(false);

    QString devName = m_config->telemetryLink->currentText();
    log("Attempting to boot the system through " + devName + ".");
    repaint();

    if (nullptr == m_dfu) {
        if (devName == "USB") {
            m_dfu = new DFUObject(DFU_DEBUG, false, QString());
        } else {
            m_dfu = new DFUObject(DFU_DEBUG, true, getPortDevice(devName));
        }
    }
    m_dfu->AbortOperation();
    if (!m_dfu->enterDFU(0)) {
        log("Could not enter DFU mode.");
        delete m_dfu;
        m_dfu = NULL;
        bootButtonsSetEnable(true);
        m_config->rescueButton->setEnabled(true); // Boot not possible, maybe Rescue OK?
        emit bootFailed();
        return;
    }
    log("Booting system...");
    m_dfu->JumpToApp(safeboot, erase);
    // Restart the polling thread
    // cm->resumePolling();
    m_config->rescueButton->setEnabled(true);
    m_config->telemetryLink->setEnabled(true);
    if (m_currentIAPStep == IAP_STATE_BOOTLOADER) {
        // Freeze the tabs, they are not useful anymore and their buttons
        // will cause segfaults or weird stuff if we use them.
        for (int i = 0; i < m_config->systemElements->count(); i++) {
            // OP-682 arriving here too "early" (before the devices are refreshed) was leading to a crash
            // OP-682 the crash was due to an unchecked cast in the line below that would cast a RunningDeviceGadget to a DeviceGadget
            DeviceWidget *qw = dynamic_cast<DeviceWidget *>(m_config->systemElements->widget(i));
            if (qw) {
                // OP-682 fixed a second crash by disabling *all* buttons in the device widget
                // disabling the buttons is only half of the solution as even if the buttons are enabled
                // the app should not crash
                qw->freeze();
            }
        }
    }
    m_currentIAPStep = IAP_STATE_READY;
    log("You can now reconnect telemetry...");
    delete m_dfu; // Frees up the USB/Serial port too
    m_dfu = nullptr;
    emit bootSuccess();
}

void UploaderWidget::autoUpdateDisconnectProgress(int value)
{
    emit progressUpdate(WAITING_DISCONNECT, value);
}

void UploaderWidget::autoUpdateConnectProgress(int value)
{
    emit progressUpdate(WAITING_CONNECT, value);
}

void UploaderWidget::autoUpdateFlashProgress(int value)
{
    emit progressUpdate(UPLOADING_FW, value);
}

/**
   Attempt a guided procedure to put both boards in BL mode when
   the system is not bootable
 */
void UploaderWidget::systemRescue()
{

    // Delete all previous tabs:
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
    }

    // Existing DFU objects will have a handle over USB and will
    // disturb everything for the rescue process:
    if (nullptr != m_dfu) {
        delete m_dfu;
        m_dfu = nullptr;
    }

    // Avoid users pressing Rescue twice.
    //this->setEnabled(false);

    // Now we're good to go
    clearLog();
    log("**********************************************************");
    log("** Follow those instructions to attempt a system rescue **");
    log("**********************************************************");
    log("You will be prompted to first connect USB, then system power");
    QString devName = m_config->telemetryLink->currentText();
    // Check if boards are connected and, if yes, prompt user to disconnect them all
    if(devName != "USB") {
        m_dfu = new DFUObject(DFU_DEBUG, true, devName);
        if (!m_dfu->ready()){
            this->setEnabled(true);
            log("not ready");
            delete m_dfu;
            m_dfu = nullptr;
            return;
        } else {
            log("ready");
        }
    } else {
//    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() > 0) {
//        QString labelText = QString("<p align=\"left\">%1</p>").arg(tr("Please disconnect your board."));
//        int result = ConnectionWaiter::openDialog(tr("System Rescue"), labelText, 0, BOARD_EVENT_TIMEOUT, this);
//        switch (result) {
//        case ConnectionWaiter::Canceled:
//            m_config->rescueButton->setEnabled(true);
//            return;
//
//        case ConnectionWaiter::TimedOut:
//            QMessageBox::warning(this, tr("System Rescue"), tr("Timed out while waiting for all boards to be disconnected!"));
//            m_config->rescueButton->setEnabled(true);
//            return;
//        }
//    }

        // Now prompt user to connect board
        QString labelText = QString("<p align=\"left\">%1</p>").arg(tr("Please connect your board."));
        int result = ConnectionWaiter::openDialog(tr("System Rescue"), labelText, 1, BOARD_EVENT_TIMEOUT, this);
        switch (result) {
            case ConnectionWaiter::Canceled:
                this->setEnabled(true);
                return;
            case ConnectionWaiter::TimedOut:
                QMessageBox::warning(this, tr("System Rescue"),
                                     tr("Timed out while waiting for a board to be connected!"));
                this->setEnabled(true);
                return;
            default:
                break;
        }
        log("Detecting first board...");
        m_dfu = new DFUObject(DFU_DEBUG, false, QString());
    }
    this->setEnabled(true);
    m_dfu->AbortOperation();
    if (!m_dfu->enterDFU(0)) {
        log("Could not enter DFU mode.");
        delete m_dfu;
        m_dfu = nullptr;
        //cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    log("enterDFU");
    if (!m_dfu->findDevices() || (m_dfu->numberOfDevices != 1)) {
        log("Could not detect a board, aborting!");
        delete m_dfu;
        m_dfu = nullptr;
        //cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    log(QString("Found %1 device(s).").arg(m_dfu->numberOfDevices));

    if (m_dfu->numberOfDevices > 5) {
        log("Inconsistent number of devices, aborting!");
        delete m_dfu;
        m_dfu = nullptr;
        //cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    for (int i = 0; i < m_dfu->numberOfDevices; i++) {
        DeviceWidget *dw = new DeviceWidget(this);
        connectSignalSlot(dw);
        dw->setDeviceID(i);
        dw->setDfu(m_dfu);
        dw->populate();
        m_config->systemElements->addTab(dw, tr("Device") + QString::number(i));
    }
    bootButtonsSetEnable(true);
    m_config->rescueButton->setEnabled(false);

    // So that we can boot from the GUI afterwards.
    m_currentIAPStep = IAP_STATE_BOOTLOADER;
}

void UploaderWidget::uploadStarted()
{
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
}

void UploaderWidget::uploadEnded(bool succeed)
{
    Q_UNUSED(succeed);
    // device is halted so no halt
    bootButtonsSetEnable(true);
    // device is halted so no reset
    m_config->rescueButton->setEnabled(true);
}

void UploaderWidget::downloadStarted()
{
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
}

void UploaderWidget::downloadEnded(bool succeed)
{
    Q_UNUSED(succeed);
    // device is halted so no halt
    bootButtonsSetEnable(true);
    // device is halted so no reset
    m_config->rescueButton->setEnabled(true);
}

/**
   Update log entry
 */
void UploaderWidget::log(QString str)
{
    qDebug() << "UploaderGadgetWidget -" << str;
    m_config->textBrowser->append(str);
}

void UploaderWidget::clearLog()
{
    m_config->textBrowser->clear();
}

/**
 * Remove all the device widgets...
 */
UploaderWidget::~UploaderWidget()
{
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
        qw = 0;
    }
}


int ResultEventLoop::run(int millisTimout)
{
    m_timer.singleShot(millisTimout, this, SLOT(fail()));
    return exec();
}

void ResultEventLoop::success()
{
    m_timer.stop();
    exit(0);
}

void ResultEventLoop::fail()
{
    m_timer.stop();
    exit(-1);
}
