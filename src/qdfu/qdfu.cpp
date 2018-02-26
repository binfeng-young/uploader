#include "qdfu.h"

#include <QTimer>
#include <QDebug>
#include <QtCore/QTime>

using namespace std;
using namespace DFU;

DFUObject::DFUObject(bool _debug, bool _use_serial, QString portname)
{
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
    return m_dfuBase->UploadDescription(desc.toByteArray().toStdString());
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
    return m_dfuBase->findDevices();
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
quint32 DFUObject::CRC32WideFast(quint32 Crc, quint32 Size, quint32 *Buffer)
{
    // Size = Size >> 2; // /4  Size passed in as a byte count, assumed to be a multiple of 4

    while (Size--) {
        // Nibble lookup table for 0x04C11DB7 polynomial
        static const quint32 CrcTable[16] = {
            0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
            0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD
        };

        Crc     = Crc ^ *((quint32 *)Buffer); // Apply all 32-bits

        Buffer += 1;

        // Process 32-bits, 4 at a time, or 8 rounds

        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // 0x04C11DB7 Polynomial used in STM32
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
    }

    return Crc;
}

/**
   Utility function
 */
quint32 DFUObject::CRCFromQBArray(QByteArray array, quint32 Size)
{
    quint32 pad = Size - array.length();

    array.append(QByteArray(pad, (char)255));
    int num_words = Size / 4;
    quint32 *t    = (quint32 *)malloc(Size);
    for (int x = 0; x < num_words; x++) {
        quint32 aux = 0;
        aux  = (char)array[x * 4 + 3] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 2] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 1] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 0] & 0xFF;
        t[x] = aux;
    }
    quint32 ret = DFUObject::CRC32WideFast(0xFFFFFFFF, num_words, t);
    free(t);

    return ret;
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