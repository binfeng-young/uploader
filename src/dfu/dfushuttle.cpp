//
// Created by bfyoung on 2/22/18.
//

#include "dfubase.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <thread>
#include <cstring>
#include "serialport.h"

int main(int argc, char *argv[]) {
    std::string filePath = "/home/binfeng/Desktop/fw_hx_v200.opfw";
    std::string portName = "ttyUSB0";
    SerialPort serialPort;
    serialPort.openPort(portName);
    if (!serialPort.isOpened()) {
        std::cout << "Could not open serial port: ttyUSB0" << std::endl;
        return -1;
    }
    uint8_t toBLCmd[] = {0xb5, 0x62, 0x00, 0x03, 0x4c, 0x00, 0x4c};
    if (serialPort.writeBuff(reinterpret_cast<char *>(toBLCmd), 7) <= 0) {
        std::cout << "Could not write serial" << std::endl;
        return -1;
    }
    bool debug = false;

    DFU::DFUBase dfu(true, &serialPort);
    if (!dfu.ready()) {
        return -1;
    }
    std::cout << "ready" << std::endl;
    dfu.AbortOperation();
    if (!dfu.enterDFU(0)) {
        std::cout << "Could not enter DFU mode\n" << std::endl;
        return -1;
    }

    if (!dfu.findDevices() || (dfu.numberOfDevices != 1)) {
        std::cout << "Could not detect a board, aborting!" << std::endl;
        return -1;
    }
    std::cout << "Found " << dfu.numberOfDevices << "\n";
    for (int x = 0; x < dfu.numberOfDevices; ++x) {
        std::cout << "Device #" << x << "\n";
        std::cout << "Device ID=" << dfu.devices[x].ID << "\n";
        std::cout << "Device Readable=" << dfu.devices[x].Readable << "\n";
        std::cout << "Device Writable=" << dfu.devices[x].Writable << "\n";
        std::cout << "Device SizeOfCode=" << dfu.devices[x].SizeOfCode << "\n";
        std::cout << "BL Version=" << (uint32_t)dfu.devices[x].BL_Version << "\n";
        std::cout << "Device SizeOfDesc=" << dfu.devices[x].SizeOfDesc << "\n";
        std::cout << "FW CRC=" << dfu.devices[x].FW_CRC << "\n";

        int size = ((DFU::device)dfu.devices[x]).SizeOfDesc;
        dfu.enterDFU(x);
        std::cout << "Description:" << dfu.DownloadDescription(size) << "\n";
        std::cout << "\n";
    }
    int device = 0;
    if (!dfu.enterDFU(device)) {
        std::cout << "Error:Could not enter DFU mode\n" << std::endl;
        return -1;
    }
    if (!dfu.devices[device].Writable) {
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
    dfu.JumpToApp(false, true);
    std::cout << "Uploading Succeeded!\n" << std::endl;
    return 0;
}
