//
// Created by root on 2/23/18.
//

#include "dfubase.h"
#include "port.h"
#include "sspt.h"
#include <fstream>
#include <sstream>

// #include <ophid/inc/ophid_hidapi.h>
// #include <ophid/inc/ophid_usbmon.h>
// #include <ophid/inc/ophid_usbsignal.h>

using namespace std;
using namespace DFU;

DFUBase::DFUBase(bool _debug, SerialPort* serialPort) :
        debug(_debug), mready(true)
{
    use_serial = true;
    numberOfDevices = 0;
    serialhandle    = nullptr;
    port *info = nullptr;
    info = new port(serialPort, false);
    initPort(info);
}

DFUBase::DFUBase(bool _debug, bool _use_serial,const std::string& portname) :
        debug(_debug), use_serial(_use_serial), mready(true)
{
    numberOfDevices = 0;
    serialhandle    = nullptr;
    port *info = nullptr;

    if (use_serial) {
        info = new port(portname, _debug);
        initPort(info);
    } else {
/*
        hidHandle = new opHID_hidapi();

        mready    = false;
        QEventLoop m_eventloop;
        QTimer::singleShot(200, &m_eventloop, SLOT(quit()));
        m_eventloop.exec();
        QList<USBPortInfo> devices;
        devices = USBMonitor::instance()->availableDevices(0x20a0, -1, -1, USBMonitor::Bootloader);
        if (devices.length() == 1) {
            if (hidHandle->open(1, devices.first().vendorID, devices.first().productID, 0, 0) == 1) {
                mready = true;
                QTimer::singleShot(200, &m_eventloop, SLOT(quit()));
                m_eventloop.exec();
            } else {
                hidHandle->close(0);
            }
        } else {
            // Wait for the board to appear on the USB bus:
            USBSignalFilter filter(0x20a0, -1, -1, USBMonitor::Bootloader);
            connect(&filter, SIGNAL(deviceDiscovered()), &m_eventloop, SLOT(quit()));
            for (int x = 0; x < 4; ++x) {
                qDebug() << "DFU trying to detect bootloader:" << x;

                if (x == 0) {
                    QTimer::singleShot(10000, &m_eventloop, SLOT(quit()));
                } else {
                    QTimer::singleShot(2000, &m_eventloop, SLOT(quit()));
                }
                m_eventloop.exec();
                devices = USBMonitor::instance()->availableDevices(0x20a0, -1, -1, USBMonitor::Bootloader);
                qDebug() << "Devices length: " << devices.length();
                if (devices.length() == 1) {
                    qDebug() << "Opening device";
                    if (hidHandle->open(1, devices.first().vendorID, devices.first().productID, 0, 0) == 1) {
                        QTimer::singleShot(200, &m_eventloop, SLOT(quit()));
                        m_eventloop.exec();
                        qDebug() << "DFU detected after delay";
                        mready = true;
                        qDebug() << "Detected";
                        break;
                    } else {
                        hidHandle->close(0);
                    }
                } else {
                    qDebug() << devices.length() << " device(s) detected, don't know what to do!";
                    mready = false;
                }
            }
        }
 */
    }
}
void DFUBase::initPort(port*& info)
{
    if (info->status() != port::open) {
        delete info;
        info = nullptr;
        cout << "Could not open serial port\n";
        mready = false;
        return;
    }
    info->rxBuf      = sspRxBuf;
    info->rxBufSize  = MAX_PACKET_DATA_LEN;
    info->txBuf      = sspTxBuf;
    info->txBufSize  = MAX_PACKET_DATA_LEN;
    info->max_retry  = 10;
    info->timeoutLen = 1000;

    serialhandle = new sspt(info, debug /*debug*/);
    int count = 0;
    std::cout << "sync...." << std::endl;
    while (!serialhandle->ssp_Synchronise() && (++count < 10)) {
        if (debug) {
            std::cout << "SYNC failed, resending..." << std::endl;
        }
    }
    if (count == 10) {
        mready = false;
        std::cout << "SYNC failed" << std::endl;
        return;
    }
    std::cout << "SYNC success" << std::endl;
    // transfer ownership of port to serialhandle thread
    // start the serialhandle thread
    serialhandle->start();
}
DFUBase::~DFUBase()
{
    if (use_serial) {
        if(nullptr != serialhandle) {
            serialhandle->end();
            delete serialhandle;
            serialhandle = nullptr;
        }

    } else {
/*
        if (hidHandle) {
            hidHandle->close(0);
            delete hidHandle;
        }
 */
    }
}

bool DFUBase::SaveByteArrayToFile(std::string const & sfile, const std::string &array)
{
    ofstream file(sfile);

    if (!file.is_open()) {
        if (debug) {
            std::cout << "Can't open file" << sfile << std::endl;
        }
        return false;
    }
    file.write(array.c_str(), sizeof(char)*array.length());
    file.close();
    return true;
}

/**
   Tells the mainboard to enter DFU Mode.
 */
bool DFUBase::enterDFU(int const &devNumber)
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::EnterDFU; // DFU Command
    buf[2] = 0; // DFU Count
    buf[3] = 0; // DFU Count
    buf[4] = 0; // DFU Count
    buf[5] = 0; // DFU Count
    buf[6] = devNumber; // DFU Data0
    buf[7] = 1; // DFU Data1
    buf[8] = 1; // DFU Data2
    buf[9] = 1; // DFU Data3

    int result = sendData(buf, BUF_LEN);
    if (result < 1) {
        return false;
    }
    if (debug) {
        //qDebug() << "EnterDFU: " << result << " bytes sent";
    }
    return true;
}

/**
   Tells the board to get ready for an upload. It will in particular
   erase the memory to make room for the data. You will have to query
   its status to wait until erase is done before doing the actual upload.
 */
bool DFUBase::StartUpload(int32_t const & numberOfBytes, TransferTypes const & type, uint32_t crc)
{
    int lastPacketCount;
    int32_t numberOfPackets = numberOfBytes / 4 / 14;
    int pad = (numberOfBytes - numberOfPackets * 4 * 14) / 4;

    if (pad == 0) {
        lastPacketCount = 14;
    } else {
        ++numberOfPackets;
        lastPacketCount = pad;
    }
    char buf[BUF_LEN];
    buf[0]  = 0x02; // reportID
    buf[1]  = setStartBit(DFU::Upload); // DFU Command
    buf[2]  = numberOfPackets >> 24; // DFU Count
    buf[3]  = numberOfPackets >> 16; // DFU Count
    buf[4]  = numberOfPackets >> 8; // DFU Count
    buf[5]  = numberOfPackets; // DFU Count
    buf[6]  = (int)type; // DFU Data0
    buf[7]  = lastPacketCount; // DFU Data1
    buf[8]  = crc >> 24;
    buf[9]  = crc >> 16;
    buf[10] = crc >> 8;
    buf[11] = crc;
    if (debug) {
        //qDebug() << "Number of packets:" << numberOfPackets << " Size of last packet:" << lastPacketCount;
    }

    int result = sendData(buf, BUF_LEN);
    this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (debug) {
        //qDebug() << result << " bytes sent";
    }
    if (result > 0) {
        return true;
    }
    return false;
}

/**
   Does the actual data upload to the board. Needs to be called once the
   board is ready to accept data following a StartUpload command, and it is erased.
 */
bool DFUBase::UploadData(int32_t const & numberOfBytes, const std::string & data)
{
    int lastPacketCount;
    int32_t numberOfPackets = numberOfBytes / 4 / 14;
    int pad = (numberOfBytes - numberOfPackets * 4 * 14) / 4;

    if (pad == 0) {
        lastPacketCount = 14;
    } else {
        ++numberOfPackets;
        lastPacketCount = pad;
    }
    if (debug) {
        //qDebug() << "Start Uploading:" << numberOfPackets << "4Bytes";
    }
    char buf[BUF_LEN];
    buf[0] = 0x02; // reportID
    buf[1] = DFU::Upload; // DFU Command
    int packetsize;
    float percentage;
    int laspercentage = 0;
    for (int32_t packetcount = 0; packetcount < numberOfPackets; ++packetcount) {
        percentage = (float)(packetcount + 1) / numberOfPackets * 100;
        if (laspercentage != (int)percentage) {
            printProgBar((int)percentage, "UPLOADING");
        }
        laspercentage = (int)percentage;
        if (packetcount == numberOfPackets) {
            packetsize = lastPacketCount;
        } else {
            packetsize = 14;
        }
        // qDebug()<<packetcount;
        buf[2]  = packetcount >> 24; // DFU Count
        buf[3]  = packetcount >> 16; // DFU Count
        buf[4]  = packetcount >> 8; // DFU Count
        buf[5]  = packetcount; // DFU Count
        char *pointer = const_cast<char *>(data.data());
        pointer = pointer + 4 * 14 * packetcount;
        // if (debug) {
        // qDebug() << "Packet Number=" << packetcount << "Data0=" << (int)data[0] << " Data1=" << (int)data[1] << " Data0=" << (int)data[2] << " Data0=" << (int)data[3] << " buf6=" << (int)buf[6] << " buf7=" << (int)buf[7] << " buf8=" << (int)buf[8] << " buf9=" << (int)buf[9];
        // }
        CopyWords(pointer, buf + 6, packetsize * 4);
        // for (int y=0;y<packetsize*4;++y) {
        // qDebug()<<y<<":"<<(int)data[packetcount*14*4+y]<<"---"<<(int)buf[6+y];
        // }
        // qDebug()<<" Data0="<<(int)data[0]<<" Data0="<<(int)data[1]<<" Data0="<<(int)data[2]<<" Data0="<<(int)data[3]<<" buf6="<<(int)buf[6]<<" buf7="<<(int)buf[7]<<" buf8="<<(int)buf[8]<<" buf9="<<(int)buf[9];
        // QThread::msleep(send_delay);

        // if (StatusRequest() != DFU::uploading) {
        // return false;
        // }
        int result = sendData(buf, BUF_LEN);
        // if (debug) {
        // qDebug() << "sent:" << result;
        // }
        if (result < 1) {
            return false;
        }
        // if (debug) {
        // qDebug() << "UPLOAD:" << "Data=" << (int)buf[6] << (int)buf[7] << (int)buf[8] << (int)buf[9] << ";" << result << " bytes sent";
        // }
    }
    return true;
}

/**
   Sends the firmware description to the device
 */
DFU::Status DFUBase::UploadDescription(const std::string& desc)
{
    cout << "Starting uploading description\n";

    if (!StartUpload(desc.length(), DFU::Descript, 0)) {
        return DFU::abort;
    }
    if (!UploadData(desc.length(), desc)) {
        return DFU::abort;
    }
    if (!EndOperation()) {
        return DFU::abort;
    }
    DFU::Status ret = StatusRequest();

    if (debug) {
        //qDebug() << "Upload description Status=" << StatusToString(ret);
    }
    return ret;
}


/**
   Downloads the description string for the current device.
   You have to call enterDFU before calling this function.
 */
std::string DFUBase::DownloadDescription(int const & numberOfChars)
{
    std::string arr;

    StartDownloadT(arr, numberOfChars, DFU::Descript);
    std::cout << "***** " << arr << std::endl;
    long index = arr.find((char)255, 0);
    return std::string((index == std::string::npos) ? arr : arr.substr(0, index));
}

std::string DFUBase::DownloadDescriptionAsBA(int const & numberOfChars)
{
    std::string arr;

    StartDownloadT(arr, numberOfChars, DFU::Descript);
    std::cout << "***** " << arr << std::endl;
    return arr;
}

/**
   Starts a firmware download
   @param firmwareArray: pointer to the location where we should store the firmware
   @package device: the device to use for the download
 */
std::string DFUBase::DownloadFirmware(int sizeOfCode)
{
    std::string arr;
    StartDownloadT(arr, sizeOfCode, DFU::FW);
    return arr;
}

/**
   Downloads a certain number of bytes from a certain location, and stores in an array whose
   pointer is passed as an argument
 */
bool DFUBase::StartDownloadT(std::string &fw, int32_t const & numberOfBytes, TransferTypes const & type)
{
    int lastPacketCount;

    // First of all, work out the number of DFU packets we should ask for:
    int32_t numberOfPackets = numberOfBytes / 4 / 14;
    int pad = (numberOfBytes - numberOfPackets * 4 * 14) / 4;

    if (pad == 0) {
        lastPacketCount = 14;
    } else {
        ++numberOfPackets;
        lastPacketCount = pad;
    }

    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::Download_Req; // DFU Command
    buf[2] = numberOfPackets >> 24; // DFU Count
    buf[3] = numberOfPackets >> 16; // DFU Count
    buf[4] = numberOfPackets >> 8; // DFU Count
    buf[5] = numberOfPackets; // DFU Count
    buf[6] = (int)type; // DFU Data0
    buf[7] = lastPacketCount; // DFU Data1
    buf[8] = 1; // DFU Data2
    buf[9] = 1; // DFU Data3

    int result = sendData(buf, BUF_LEN);
    if (debug) {
        //qDebug() << "StartDownload:" << numberOfPackets << "packets" << " Last Packet Size=" << lastPacketCount << " " << result << " bytes sent";
    }
    float percentage;
    int laspercentage = 0;

    // Now get those packets:
    for (int32_t x = 0; x < numberOfPackets; ++x) {
        int size;
        percentage = (float)(x + 1) / numberOfPackets * 100;
        if (laspercentage != (int)percentage) {
            printProgBar((int)percentage, "DOWNLOADING");
        }
        laspercentage = (int)percentage;

        result = receiveData(buf, BUF_LEN);
        if (debug) {
            //qDebug() << result << " bytes received" << " Count=" << x << "-" << (int)buf[2] << ";" << (int)buf[3] << ";" << (int)buf[4] << ";" << (int)buf[5] << " Data=" << (int)buf[6] << ";" << (int)buf[7] << ";" << (int)buf[8] << ";" << (int)buf[9];
        }
        if (x == numberOfPackets - 1) {
            size = lastPacketCount * 4;
        } else {
            size = 14 * 4;
        }
        fw.append(buf + 6, size);
    }

    StatusRequest();
    return true;
}

/**
   Resets the device
 */
int DFUBase::ResetDevice(void)
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::Reset; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    return sendData(buf, BUF_LEN);
    // return hidHandle->send(0,buf, BUF_LEN, 500);
}

int DFUBase::AbortOperation(void)
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::Abort_Operation; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    return sendData(buf, BUF_LEN);
}

/**
   Starts the firmware (leaves bootloader and boots the main software)
 */
int DFUBase::JumpToApp(bool safeboot, bool erase)
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::JumpFW; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    if (safeboot) {
        /* force system to safe boot mode (hwsettings == defaults) */
        buf[8] = 0x5A;
        buf[9] = 0xFE;
    } else {
        buf[8] = 0;
        buf[9] = 0;
    }
    if (erase) {
        // force data flash clear
        buf[10] = 0x00;
        buf[11] = 0x00;
        buf[12] = 0xFA;
        buf[13] = 0x5F;

        buf[14] = 0x00;
        buf[15] = 0x00;
        buf[16] = 0x00;
        buf[17] = 0x01;

        buf[18] = 0x00;
        buf[19] = 0x00;
        buf[20] = 0x00;
        buf[21] = 0x00;
    } else {
        buf[10] = 0x00;
        buf[11] = 0x00;
        buf[12] = 0x00;
        buf[13] = 0x00;

        buf[14] = 0x00;
        buf[15] = 0x00;
        buf[16] = 0x00;
        buf[17] = 0x00;

        buf[18] = 0x00;
        buf[19] = 0x00;
        buf[20] = 0x00;
        buf[21] = 0x00;
    }

    return sendData(buf, BUF_LEN);
}

DFU::Status DFUBase::StatusRequest()
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::Status_Request; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result    = sendData(buf, BUF_LEN);
    int retry_cnt = 0;
    const int MaxSendRetry = 10, SendRetryIntervalMS = 1000;
    while (result < 0 && retry_cnt < MaxSendRetry) {
        retry_cnt++;
        //qWarning() << "StatusRequest failed, sleeping" << SendRetryIntervalMS << "ms";
        this_thread::sleep_for(std::chrono::milliseconds(SendRetryIntervalMS));
        //qWarning() << "StatusRequest retry attempt" << retry_cnt;
        result = sendData(buf, BUF_LEN);
    }
    if (retry_cnt >= MaxSendRetry) {
        //qWarning() << "StatusRequest failed too many times, aborting";
        return DFU::abort;
    }
    if (debug) {
        //qDebug() << "StatusRequest: " << result << " bytes sent";
    }
    result = receiveData(buf, BUF_LEN);
    if (debug) {
        //qDebug() << "StatusRequest: " << result << " bytes received";
    }
    if (buf[1] == DFU::Status_Rep) {
        return (DFU::Status)buf[6];
    } else {
        return DFU::abort;
    }
}

/**
   Ask the bootloader for the list of devices available
 */
bool DFUBase::findDevices()
{
    devices.clear();
    char buf[BUF_LEN];
    buf[0] = 0x02; // reportID
    buf[1] = DFU::Req_Capabilities; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    if (result < 1) {
        return false;
    }

    result = receiveData(buf, BUF_LEN);
    if (result < 1) {
        return false;
    }

    numberOfDevices = buf[7];
    RWFlags = buf[8];
    RWFlags = RWFlags << 8 | buf[9];

    if (buf[1] == DFU::Rep_Capabilities) {
        for (int x = 0; x < numberOfDevices; ++x) {
            device dev;
            dev.Readable = (bool)(RWFlags >> (x * 2) & 1);
            dev.Writable = (bool)(RWFlags >> (x * 2 + 1) & 1);
            devices.push_back(dev);
            buf[0] = 0x02; // reportID
            buf[1] = DFU::Req_Capabilities; // DFU Command
            buf[2] = 0;
            buf[3] = 0;
            buf[4] = 0;
            buf[5] = 0;
            buf[6] = x + 1;
            buf[7] = 0;
            buf[8] = 0;
            buf[9] = 0;
            sendData(buf, BUF_LEN);
            receiveData(buf, BUF_LEN);
            devices[x].ID = buf[14];
            devices[x].ID = devices[x].ID << 8 | (uint8_t)buf[15];
            devices[x].BL_Version = buf[7];
            devices[x].SizeOfDesc = buf[8];

            uint32_t aux;
            aux = (uint8_t)buf[10];
            aux = aux << 8 | (uint8_t)buf[11];
            aux = aux << 8 | (uint8_t)buf[12];
            aux = aux << 8 | (uint8_t)buf[13];

            devices[x].FW_CRC = aux;


            aux = (uint8_t)buf[2];
            aux = aux << 8 | (uint8_t)buf[3];
            aux = aux << 8 | (uint8_t)buf[4];
            aux = aux << 8 | (uint8_t)buf[5];
            devices[x].SizeOfCode = aux;
        }
//        if (debug) {
//            qDebug() << "Found " << numberOfDevices << " devices";
//            for (int x = 0; x < numberOfDevices; ++x) {
//                qDebug() << "Device #" << x + 1;
//                qDebug() << "Device ID=" << devices[x].ID;
//                qDebug() << "Device Readable=" << devices[x].Readable;
//                qDebug() << "Device Writable=" << devices[x].Writable;
//                qDebug() << "Device SizeOfCode=" << devices[x].SizeOfCode;
//                qDebug() << "Device SizeOfDesc=" << devices[x].SizeOfDesc;
//                qDebug() << "BL Version=" << devices[x].BL_Version;
//                qDebug() << "FW CRC=" << devices[x].FW_CRC;
//            }
//        }
    }
    return true;
}

bool DFUBase::EndOperation()
{
    char buf[BUF_LEN];

    buf[0] = 0x02; // reportID
    buf[1] = DFU::END; // DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    if (debug) {
        //qDebug() << result << " bytes sent";
    }
    if (result > 0) {
        return true;
    }
    return false;
}

/**
   Starts a firmware upload (asynchronous)
 */

DFU::Status DFUBase::UploadFirmware(const std::string &sfile, const bool &verify, int device)
{
    DFU::Status ret;

    if (debug) {
       // qDebug() << "Starting Firmware Uploading...";
    }
    std::ifstream file(sfile, ios::binary);
    if (!file.is_open()) {
        return DFU::abort;
    }
    std::stringstream streambuf;
    streambuf << file.rdbuf();
    std::string arr(streambuf.str());

    if (debug) {
        std::cout << "Bytes Loaded=" << arr.length()<< std::endl;
    }
    if (arr.size() % 4 != 0) {
        int pad = arr.size() / 4;
        ++pad;
        pad = pad * 4;
        pad = pad - arr.size();
        arr.append(std::string(pad, (char)255));
    }
    if (devices[device].SizeOfCode < (uint32_t)arr.size()) {
        if (debug) {
            std::cout << "ERROR file to big for device" << std::endl;
        }
        return DFU::abort;;
    }

    uint32_t crc = DFUBase::CRCFromQBArray(arr, devices[device].SizeOfCode);
    if (debug) {
        std::cout << "NEW FIRMWARE CRC=" << crc << std::endl;
    }

    if (!StartUpload(arr.size(), DFU::FW, crc)) {
        ret = StatusRequest();
        if (debug) {
            //std::cout << "StartUpload failed" << std::endl;
            //qDebug() << "StartUpload returned:" << StatusToString(ret);
        }
        return ret;
    }

    //emit operationProgress("Erasing, please wait...");

    if (debug) {
        std::cout << "Erasing memory..." << std::endl;
    }
    if (StatusRequest() == DFU::abort) {
        return DFU::abort;
    }

    // TODO: why is there a loop there? The "if" statement
    // will cause a break or return anyway!!
    for (int x = 0; x < 3; ++x) {
        ret = StatusRequest();
        if (debug) {
            std::cout << "Erase returned: " << StatusToString(ret) << std::endl;
        }
        if (ret == DFU::uploading) {
            break;
        } else {
            return ret;
        }
    }

    if (!UploadData(arr.size(), arr)) {
        ret = StatusRequest();
        if (debug) {
            std::cout << "Upload failed (upload data)"<< std::endl;
            std::cout << "UploadData returned:" << StatusToString(ret)<< std::endl;
        }
        return ret;
    }
    if (!EndOperation()) {
        ret = StatusRequest();
        if (debug) {
            std::cout << "Upload failed (end operation)" << std::endl;
            std::cout << "EndOperation returned:" << StatusToString(ret) << std::endl;
        }
        return ret;
    }
    ret = StatusRequest();
    if (ret != DFU::Last_operation_Success) {
        return ret;
    }

    if (verify) {
        //emit operationProgress("Verifying firmware");
        cout << "Starting code verification\n";
        std::string arr2;
        StartDownloadT(arr2, arr.size(), DFU::FW);
        if (arr != arr2) {
            cout << "Verify:FAILED\n";
            return DFU::abort;
        }
    }

    if (debug) {
        std::cout << "Status=" << ret << std::endl;
    }
    cout << "Firmware Uploading succeeded\n";
    return ret;
}

DFU::Status DFUBase::CompareFirmware(const std::string &sfile, const CompareType &type, int device)
{
    cout << "Starting Firmware Compare...\n";
    ifstream file(sfile);
    if (!file.is_open()) {
        if (debug) {
            std::cout << "Cant open file" << std::endl;
        }
        return DFU::abort;
    }
    std::stringstream streambuf;
    streambuf << file.rdbuf();
    std::string arr(streambuf.str());
    if (debug) {
        std::cout  << "Bytes Loaded=" << arr.length() << std::endl;
    }
    if (arr.length() % 4 != 0) {
        int pad = arr.length() / 4;
        ++pad;
        pad = pad * 4;
        pad = pad - arr.length();
        arr.append(std::string(pad, (char)255));
    }
    if (type == DFU::crccompare) {
        uint32_t crc = DFUBase::CRCFromQBArray(arr, devices[device].SizeOfCode);
        if (crc == devices[device].FW_CRC) {
            cout << "Compare Successfull CRC MATCH!\n";
        } else {
            cout << "Compare failed CRC DONT MATCH!\n";
        }
        return StatusRequest();
    } else {
        std::string arr2;
        StartDownloadT(arr2, arr.length(), DFU::FW);
        if (arr == arr2) {
            cout << "Compare Successfull ALL Bytes MATCH!\n";
        } else {
            cout << "Compare failed Bytes DONT MATCH!\n";
        }
        return StatusRequest();
    }
}

void DFUBase::CopyWords(char *source, char *destination, int count)
{
    for (int x = 0; x < count; x = x + 4) {
        *(destination + x)     = source[x + 3];
        *(destination + x + 1) = source[x + 2];
        *(destination + x + 2) = source[x + 1];
        *(destination + x + 3) = source[x + 0];
    }
}

std::string DFUBase::StatusToString(DFU::Status const & status)
{
    switch (status) {
        case DFUidle:
            return "DFUidle";

        case uploading:
            return "";

        case wrong_packet_received:
            return "wrong_packet_received";

        case too_many_packets:
            return "too_many_packets";

        case too_few_packets:
            return "too_few_packets";

        case Last_operation_Success:
            return "Last_operation_Success";

        case downloading:
            return "downloading";

        case idle:
            return "idle";

        case Last_operation_failed:
            return "Last_operation_failed";

        case outsideDevCapabilities:
            return "outsideDevCapabilities";

        case CRC_Fail:
            return "CRC check FAILED";

        case failed_jump:
            return "Jmp to user FW failed";

        case abort:
            return "abort";

        case uploadingStarting:
            return "Uploading Starting";

        default:
            return "unknown";
    }
}

/**
   Prints a progress bar with percentage & label during an operation.

   Also outputs to stdout if we are in debug mode.
 */
void DFUBase::printProgBar(int const & percent, std::string const & label)
{
    std::string bar;
    for (int i = 0; i < 50; i++) {
        if (i < (percent / 2)) {
            bar.replace(i, 1, "=");
        } else if (i == (percent / 2)) {
            bar.replace(i, 1, ">");
        } else {
            bar.replace(i, 1, " ");
        }
    }

    std::cout << "\r" << label << "[" << bar << "] ";
    std::cout.width(3);
    std::cout << percent << "%     " << std::flush;
    if (nullptr != printCb) {
        printCb(percent);
    }
}

/**
   Utility function
 */
uint32_t DFUBase::CRC32WideFast(uint32_t Crc, uint32_t Size, uint32_t *Buffer)
{
    // Size = Size >> 2; // /4  Size passed in as a byte count, assumed to be a multiple of 4

    while (Size--) {
        // Nibble lookup table for 0x04C11DB7 polynomial
        static const uint32_t CrcTable[16] = {
                0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
                0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD
        };

        Crc     = Crc ^ *((uint32_t *)Buffer); // Apply all 32-bits

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
uint32_t DFUBase::CRCFromQBArray(std::string array, uint32_t Size)
{
    uint32_t pad = Size - array.length();

    array.append(std::string(pad, (char)255));
    int num_words = Size / 4;
    uint32_t *t    = (uint32_t *)malloc(Size);
    for (int x = 0; x < num_words; x++) {
        uint32_t aux = 0;
        aux  = (char)array[x * 4 + 3] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 2] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 1] & 0xFF;
        aux  = aux << 8;
        aux += (char)array[x * 4 + 0] & 0xFF;
        t[x] = aux;
    }
    uint32_t ret = DFUBase::CRC32WideFast(0xFFFFFFFF, num_words, t);
    free(t);

    return ret;
}

/**
   Send data to the bootloader, either through the serial port
   of through the HID handle, depending on the mode we're using
 */
int DFUBase::sendData(void *data, int size)
{
/*
    if (!use_serial) {
        return hidHandle->send(0, data, size, 5000);
    }
 */

    // Serial Mode:
    if (serialhandle->sendData((uint8_t *)data + 1, size - 1)) {
        if (debug) {
            std::cout  << "packet sent" << "data0" << (int)(((uint8_t *)data + 1)[0]) << std::endl;
        }
        return size;
    }
    if (debug) {
        std::cout << "Serial send OVERRUN" << std::endl;
    }
    return -1;
}

/**
   Receive data from the bootloader, either through the serial port
   of through the HID handle, depending on the mode we're using
 */
int DFUBase::receiveData(void *data, int size)
{
/*
    if (!use_serial) {
        return hidHandle->receive(0, data, size, 10000);
    }
 */

    // Serial Mode:
    int x;
    Time time;

    time.start();
    while (true) {
        if ((x = serialhandle->read_Packet(((char *)data) + 1) != -1) || time.elapsed() > 10000) {
            // QThread::msleep(10);
            if (time.elapsed() > 10000) {
                std::cout << "____timeout";
            }
            if (x > size - 1) {
                //qDebug() << "Error buffer overrun";
                //Q_ASSERT(false);
            }
            return x;
        }
    }
}
