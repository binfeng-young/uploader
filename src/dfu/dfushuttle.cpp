//
// Created by bfyoung on 2/22/18.
//

#include "dfu.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

int main(int argc, char *argv[])
{
    bool debug = false;
    QString filePath = "/home/binfeng/Desktop/fw_hx_v200.opfw";
    DFU::DFUObject dfu(true, true, "ttyUSB0");
    if (!dfu.ready()) {
        return -1;
    }
    dfu.AbortOperation();
    if (!dfu.enterDFU(0)) {
        std::cout << "Could not enter DFU mode\n" << std::endl;
        return -1;
    }
    if (debug) {
        DFU::Status ret = dfu.StatusRequest();
        std::cout << dfu.StatusToString(ret).toStdString() << std::endl;
    }
    if (!dfu.findDevices() || (dfu.numberOfDevices != 1)) {
        std::cout << "Could not detect a board, aborting!" << std::endl;
        return -1;
    }
//    std::cout << "Found " << dfu.numberOfDevices << "\n";
//    for (int x = 0; x < dfu.numberOfDevices; ++x) {
//        std::cout << "Device #" << x << "\n";
//        std::cout << "Device ID=" << dfu.devices[x].ID << "\n";
//        std::cout << "Device Readable=" << dfu.devices[x].Readable << "\n";
//        std::cout << "Device Writable=" << dfu.devices[x].Writable << "\n";
//        std::cout << "Device SizeOfCode=" << dfu.devices[x].SizeOfCode << "\n";
//        std::cout << "BL Version=" << dfu.devices[x].BL_Version << "\n";
//        std::cout << "Device SizeOfDesc=" << dfu.devices[x].SizeOfDesc << "\n";
//        std::cout << "FW CRC=" << dfu.devices[x].FW_CRC << "\n";
//
//        int size = ((DFU::device)dfu.devices[x]).SizeOfDesc;
//        dfu.enterDFU(x);
//        std::cout << "Description:" << dfu.DownloadDescription(size).toLatin1().data() << "\n";
//        std::cout << "\n";
//    }
    int device = 0;
    if (!dfu.enterDFU(device)) {
        std::cout << "Error:Could not enter DFU mode\n" << std::endl;
        return -1;
    }
    if (((DFU::device)dfu.devices[device]).Writable == false) {
        std::cout << "ERROR device not Writable\n" << std::endl;
        return false;
    }
    std::cout << "Uploading..." << std::endl;

    // this call is asynchronous so the only false status it will report
    // is when it is already running...
    bool retstatus = dfu.UploadFirmware(filePath.toLatin1(), false, device);

    if (!retstatus) {
        std::cout << "Upload failed with code:" << retstatus << std::endl;
        return -1;
    }
    while (!dfu.isFinished()) {
        QThread::msleep(500);
    }
    return 0;
    // TODO check if upload went well...
    if (filePath.endsWith("opfw")) {
        std::ifstream fwfile(filePath.toStdString());
        if (!fwfile.is_open()) {
            std::cout << "Cannot open file " << filePath.toStdString() << std::endl;
            return -1;
        }
        std::stringstream streambuf;
        streambuf << fwfile.rdbuf();
        std::string strBuff(streambuf.str());
        //std::string strBuff(std::istreambuf_iterator<char>(fwfile), std::istreambuf_iterator<char>());
        //std::cout << strBuff << " xxxxxxx" << std::endl;
        QByteArray firmware = QString::fromStdString(strBuff).toLatin1();
        QByteArray desc     = firmware.right(100);
        DFU::Status status  = dfu.UploadDescription(desc);
        if (status != DFU::Last_operation_Success) {
            std::cout << "Upload failed with code:" << retstatus << std::endl;
            return -1;
        }
   // }// else if (!description.isEmpty()) {
//        DFU::Status status = dfu.UploadDescription(description);
//        if (status != DFU::Last_operation_Success) {
//            std::cout << "Upload failed with code:" << status << std::endl;
//            return -1;
//        }
    }
    while (!dfu.isFinished()) {
        QThread::msleep(500);
    }
    std::cout << "Uploading Succeeded!\n" << std::endl;
    return 0;
}
