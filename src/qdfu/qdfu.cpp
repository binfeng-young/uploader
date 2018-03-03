#include "qdfu.h"

#include <QTimer>
#include <QDebug>
#include <QtCore/QTime>

using namespace std;
using namespace DFU;

DFUObject::DFUObject(bool _debug, bool _use_serial, QString portname)
{
    qRegisterMetaType<DFU::Status>("Status");
    m_dfuBase = new DFUBase(_debug, _use_serial, portname.toStdString());
}

DFUObject::~DFUObject()
{
    delete m_dfuBase;
}

bool DFUObject::ready()
{
    return m_dfuBase->ready();
}

bool DFUObject::SaveByteArrayToFile(QString const & sfile, const QByteArray &array)
{
    return m_dfuBase->SaveByteArrayToFile(sfile.toStdString(), array.toStdString());
}

bool DFUObject::enterDFU(int const &devNumber)
{
    return m_dfuBase->enterDFU(devNumber);
}

DFU::Status DFUObject::UploadDescription(QVariant desc)
{
    cout << "Starting uploading description\n";

    emit operationProgress("Uploading description");

    std::string array;
    if (desc.type() == QVariant::String) {
        QString description = desc.toString();
        if (description.length() % 4 != 0) {
            int pad = description.length() / 4;
            pad = (pad + 1) * 4;
            pad = pad - description.length();
            QString padding;
            padding.fill(' ', pad);
            description.append(padding);
        }
        array = description.toLatin1().toStdString();
    } else if (desc.type() == QVariant::ByteArray) {
        array = desc.toByteArray().toStdString();
    }


    return m_dfuBase->UploadDescription(array);
}

QString DFUObject::DownloadDescription(int const & numberOfChars)
{
    return QString::fromStdString(m_dfuBase->DownloadDescription(numberOfChars));
}

QByteArray DFUObject::DownloadDescriptionAsBA(int const & numberOfChars)
{
    return QByteArray::fromStdString(m_dfuBase->DownloadDescriptionAsBA(numberOfChars));
}

bool DFUObject::DownloadFirmware(QByteArray *firmwareArray, int device)
{
    if (isRunning()) {
        return false;
    }
    requestedOperation  = DFU::Download;
    requestSize = devices[device].SizeOfCode;
    requestStorage = firmwareArray;
    start();
    return true;
}

bool DFUObject::UploadFirmware(const QString &sfile, const bool &verify, int device)
{
    if (isRunning()) {
        return false;
    }
    requestedOperation = DFU::Upload;
    requestFilename    = sfile;
    requestDevice = device;
    requestVerify = verify;
    start();
    return true;
}

void DFUObject::run()
{
    switch (requestedOperation) {
    case DFU::Download:
        *requestStorage = QByteArray::fromStdString(m_dfuBase->DownloadFirmware(requestSize));
        emit(downloadFinished());
        break;
    case DFU::Upload:
    {
        DFU::Status ret = m_dfuBase->UploadFirmware(requestFilename.toStdString(), requestVerify, requestDevice);
        emit(uploadFinished(ret));
        break;
    }
    default:
        break;
    }
}

int DFUObject::ResetDevice(void)
{
    return m_dfuBase->ResetDevice();
}

int DFUObject::AbortOperation(void)
{
    return m_dfuBase->AbortOperation();
}

int DFUObject::JumpToApp(bool safeboot, bool erase)
{
    return m_dfuBase->JumpToApp(safeboot, erase);
}

DFU::Status DFUObject::StatusRequest()
{
    return m_dfuBase->StatusRequest();
}

bool DFUObject::findDevices()
{
    bool ret = m_dfuBase->findDevices();
    numberOfDevices = m_dfuBase->numberOfDevices;
    devices = m_dfuBase->devices;
    return ret;
}

bool DFUObject::EndOperation()
{
    return m_dfuBase->EndOperation();
}

DFU::Status DFUObject::CompareFirmware(const QString &sfile, const CompareType &type, int device)
{
    return m_dfuBase->CompareFirmware(sfile.toStdString(), type, device);
}

QString DFUObject::StatusToString(DFU::Status const & status)
{
   return QString::fromStdString(m_dfuBase->StatusToString(status));
}

/**
   Prints a progress bar with percentage & label during an operation.

   Also outputs to stdout if we are in debug mode.
 */
void DFUObject::printProgBar(int const & percent, QString const & label)
{
    emit(progressUpdated(percent));
    //m_dfuBase->printProgBar(percent, label.toStdString());
}

/**
   Utility function
 */
quint32 DFUObject::CRCFromQBArray(QByteArray array, quint32 Size)
{
    return DFUBase::CRCFromQBArray(array.toStdString(), Size);
}
#define BOARD_ID_MB      1
#define BOARD_ID_INS     2
#define BOARD_ID_PIP     3
#define BOARD_ID_CC      4
#define BOARD_ID_REVO    9
#define BOARD_ID_SPARKY2 0x92

/**
   Gets the type of board connected
 */
DFU::eBoardType DFUObject::GetBoardType(int boardNum)
{
    DFU::eBoardType brdType = eBoardUnkwn;

    // First of all, check what Board type we are talking to
    int board = devices[boardNum].ID;

    qDebug() << "Board model: " << board;
    switch (board >> 8) {
    case BOARD_ID_MB: // Mainboard family
        brdType = eBoardMainbrd;
        break;
    case BOARD_ID_INS: // Inertial Nav
        brdType = eBoardINS;
        break;
    case BOARD_ID_PIP: // PIP RF Modem
        brdType = eBoardPip;
        break;
    case BOARD_ID_CC: // CopterControl family
        brdType = eBoardCC;
        break;
    case BOARD_ID_REVO: // Revo board
        brdType = eBoardRevo;
        break;
    case BOARD_ID_SPARKY2: // Sparky2 board
        brdType = eBoardSparky2;
        break;
    }
    return brdType;
}