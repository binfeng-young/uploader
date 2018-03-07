#ifndef UPLOADERGADGETWIDGET_H
#define UPLOADERGADGETWIDGET_H
#include "enums.h"

#include <QEventLoop>
#include <QProgressDialog>
#include <QTimer>
#include "qdfu.h"

using namespace DFU;
using namespace uploader;
namespace Ui {
    class UploaderWidget;
}
class FlightStatus;
class UAVObject;

class TimedDialog : public QProgressDialog {
    Q_OBJECT

public:
    TimedDialog(const QString &title, const QString &labelText, int timeout, QWidget *parent = 0, Qt::WindowFlags flags = 0);

private slots:
    void perform();

private:
    QProgressBar *bar;
};

// A helper class to wait for board connection and disconnection events
// until a the desired number of connected boards is found
// or until a timeout is reached
class ConnectionWaiter : public QObject {
    Q_OBJECT

public:
    ConnectionWaiter(int targetDeviceCount, int timeout, QWidget *parent = 0);

    enum ResultCode { Ok, Canceled, TimedOut };

public slots:
    int exec();
    void cancel();
    void quit();

    static int openDialog(const QString &title, const QString &labelText, int targetDeviceCount, int timeout, QWidget *parent = 0, Qt::WindowFlags flags = 0);

signals:
    void timeChanged(int elapsed);

private slots:
    void perform();
    void deviceEvent();

private:
    QEventLoop eventLoop;
    QTimer timer;
    // timeout in ms
    int timeout;
    // elapsed time in seconds
    int elapsed;
    int targetDeviceCount;
    int result;
};

class ResultEventLoop : public QEventLoop {
    Q_OBJECT
public:
    int run(int millisTimout);

public slots:
    void success();
    void fail();

private:
    QTimer m_timer;
};

class UploaderWidget : public QWidget {
    Q_OBJECT

public:
    UploaderWidget(QWidget *parent = 0);
    ~UploaderWidget();

    static const int BOARD_EVENT_TIMEOUT;
    static const int AUTOUPDATE_CLOSE_TIMEOUT;
    static const int REBOOT_TIMEOUT;
    static const int ERASE_TIMEOUT;
    static const int BOOTLOADER_TIMEOUT;

    void log(QString str);

public slots:
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void populate();
    void autoUpdateDisconnectProgress(int);
    void autoUpdateConnectProgress(int);
    void autoUpdateFlashProgress(int);

signals:
    void progressUpdate(uploader::ProgressStep, QVariant);
    void bootloaderFailed();
    void bootloaderSuccess();
    void bootFailed();
    void bootSuccess();
    void autoUpdateFailed();
    void autoUpdateSuccess();

private:
    Ui::UploaderWidget *m_config;
    DFUObject *m_dfu;
    IAPStep m_currentIAPStep;
    void clearLog();
    QString getPortDevice(const QString &friendName);
    void connectSignalSlot(QWidget *widget);
    void bootButtonsSetEnable(bool enabled);

private slots:
    void onPhysicalHWConnect();
    void systemBoot();
    void systemSafeBoot();
    void systemEraseBoot();
    void rebootWithDialog();
    void commonSystemBoot(bool safeboot = false, bool erase = false);
    void systemRescue();
    void getSerialPorts();
    void uploadStarted();
    void uploadEnded(bool succeed);
    void downloadStarted();
    void downloadEnded(bool succeed);
};

#endif // UPLOADERGADGETWIDGET_H
