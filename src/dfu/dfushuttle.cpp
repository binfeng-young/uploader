//
// Created by bfyoung on 2/22/18.
//

#include "dfubase.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <thread>

int main(int argc, char *argv[]) {
    bool debug = false;
    std::string filePath = "/home/binfeng/Desktop/fw_hx_v200.opfw";
    DFU::DFUBase dfu(true, true, "ttyUSB0");
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
        std::cout << dfu.StatusToString(ret) << std::endl;
    }
    if (!dfu.findDevices() || (dfu.numberOfDevices != 1)) {
        std::cout << "Could not detect a board, aborting!" << std::endl;
        return -1;
    }
//    std::cout << "Found " << qdfu.numberOfDevices << "\n";
//    for (int x = 0; x < qdfu.numberOfDevices; ++x) {
//        std::cout << "Device #" << x << "\n";
//        std::cout << "Device ID=" << qdfu.devices[x].ID << "\n";
//        std::cout << "Device Readable=" << qdfu.devices[x].Readable << "\n";
//        std::cout << "Device Writable=" << qdfu.devices[x].Writable << "\n";
//        std::cout << "Device SizeOfCode=" << qdfu.devices[x].SizeOfCode << "\n";
//        std::cout << "BL Version=" << qdfu.devices[x].BL_Version << "\n";
//        std::cout << "Device SizeOfDesc=" << qdfu.devices[x].SizeOfDesc << "\n";
//        std::cout << "FW CRC=" << qdfu.devices[x].FW_CRC << "\n";
//
//        int size = ((DFU::device)qdfu.devices[x]).SizeOfDesc;
//        qdfu.enterDFU(x);
//        std::cout << "Description:" << qdfu.DownloadDescription(size).toLatin1().data() << "\n";
//        std::cout << "\n";
//    }
    int device = 0;
    if (!dfu.enterDFU(device)) {
        std::cout << "Error:Could not enter DFU mode\n" << std::endl;
        return -1;
    }
    if (((DFU::device) dfu.devices[device]).Writable == false) {
        std::cout << "ERROR device not Writable\n" << std::endl;
        return false;
    }
    std::cout << "Uploading..." << std::endl;

    // this call is asynchronous so the only false status it will report
    // is when it is already running...
    bool retstatus = dfu.UploadFirmware(filePath, false, device);

    if (!retstatus) {
        std::cout << "Upload failed with code:" << retstatus << std::endl;
        return -1;
    }
    while (dfu.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    dfu.waitStop();
    // TODO check if upload went well...
//    if (filePath.endsWith("opfw")) {
    std::ifstream fwfile(filePath);
    if (!fwfile.is_open()) {
        std::cout << "Cannot open file " << filePath << std::endl;
        return -1;
    }
    std::stringstream streambuf;
    streambuf << fwfile.rdbuf();
    std::string firmware(streambuf.str());
    //std::string strBuff(std::istreambuf_iterator<char>(fwfile), std::istreambuf_iterator<char>());
    //std::cout << strBuff << " xxxxxxx" << std::endl;
    std::string desc = firmware.substr(firmware.length() - 100, 100);
    DFU::Status status = dfu.UploadDescription(desc);
    if (status != DFU::Last_operation_Success) {
        std::cout << "Upload failed with code:" << retstatus << std::endl;
        return -1;
    }
    // }// else if (!description.isEmpty()) {
//        DFU::Status status = qdfu.UploadDescription(description);
//        if (status != DFU::Last_operation_Success) {
//            std::cout << "Upload failed with code:" << status << std::endl;
//            return -1;
//        }
    //   }
    while (dfu.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Uploading Succeeded!\n" << std::endl;
    return 0;
}
