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
#include <list>
#include "serialport.h"

void usage()
{
    std::cout << "stm firmware upgrade" << std::endl;
    std::cout << "Usage: dfu_shuttle [Options]" << std::endl;
    std::cout << "\tfile\t\tFirmware file path" << std::endl;
    std::cout << "\t\t\t\tIf no file is specified - /mnt/UDISK/fw_hx_x500.hxfw" << std::endl;
    std::cout << "\tserial_port" << std::endl;
    std::cout << "\t\t\t\tIf no serial port is specified - ttyS3" << std::endl;
    std::cout << "\t-h or --help\t\tThis help" << std::endl;
    std::cout << "\t--verify  \t\tVerify the correctness of the upgrade" << std::endl;
    std::cout << "\t-d         \t\tDebug log" << std::endl;
    std::cout << "\t-f         \t\tForced upgrade, ignore fw check" <<std::endl;
}
bool removeAll(std::vector<std::string> &list, std::string var) {
    bool hasVar = false;
    for (auto it = list.begin(); it != list.end();) {
        if (*it == var) {
            it = list.erase(it);
            hasVar = true;
        } else {
            it++;
        }
    }
    return hasVar;
}
int main(int argc, char *argv[])
{
    std::vector<std::string> arguments_list;
    for (int i = 1; i < argc; i ++) {
        arguments_list.emplace_back(argv[i]);
    }
    if (removeAll(arguments_list, "-h")  || removeAll(arguments_list ,"--help")) {
        usage();
        return 0;
    }
    bool verify = removeAll(arguments_list, "--verify");
    bool debug = removeAll(arguments_list, "-d");
    bool forced = removeAll(arguments_list, "-f");
    for(int i = 0; i < 3 ; i++) {
        if (system("killall bvrobot")) {
            break;
        }
    }
    std::string filePath;
    std::string portName;
    if (arguments_list.size() >= 2) {
        filePath = argv[1];

        char serial_port[6] = "ttyS";
        strcat(serial_port, argv[2]);
        portName = serial_port;
    }
    else {
        usage();

        filePath = "/mnt/UDISK/fw_hx_x500.hxfw";
        portName = "ttyS3";
    }
    std::cout << "file Path : " << filePath << std::endl;
    std::cout << "serial port : " << portName << std::endl;

    std::ifstream fwfile(filePath, std::ios::binary | std::ios::in);
    if (!fwfile.is_open())
    {
        std::cout << "Cannot open file " << filePath << std::endl;
        return -1;
    }
    std::stringstream streambuf;
    streambuf << fwfile.rdbuf();
    std::string loadedFW(streambuf.str());
    fwfile.close();
    SerialPort serialPort(debug);
    serialPort.openPort(portName);
    if (!serialPort.isOpen()) {
        std::cout << "Could not open serial port:"<< portName << std::endl;
        return -1;
    }
    uint8_t toBLCmd[] = {0xb5, 0x62, 0x00, 0x03, 0x4c, 0x00, 0x4c};
    if (serialPort.writeBuff(reinterpret_cast<char *>(toBLCmd), 7) <= 0) {
        std::cout << "Could not write serial" << std::endl;
        return -1;
    }

    uint8_t toBLCmd2[] = {0xb5, 0x62, 0x00, 0x04, 0x01, 0x78, 0x01, 0x78};
    if (serialPort.writeBuff(reinterpret_cast<char *>(toBLCmd2), 8) <= 0) {
        std::cout << "Could not write serial" << std::endl;
        return -1;
    }

    DFU::DFUBase dfu(debug, &serialPort);
    if (!dfu.ready()) {
        return -1;
    }
    dfu.AbortOperation();
    if (!dfu.enterDFU(0)) {
        std::cout << "Could not enter DFU mode\n" << std::endl;
        return -1;
    }

    if (!dfu.findDevices() || (dfu.numberOfDevices != 1)) {
        std::cout << "Could not detect a board, aborting!" << std::endl;
        return -1;
    }
    std::cout << "Found " << dfu.numberOfDevices << "device\n";
    for (int x = 0; x < dfu.numberOfDevices; ++x) {
        std::cout << "Device #" << x << "\n";
        std::cout << "Device ID=" << std::hex << dfu.devices[x].ID << "\n" << std::oct;
        std::cout << "Device Readable=" << dfu.devices[x].Readable << "\n";
        std::cout << "Device Writable=" << dfu.devices[x].Writable << "\n";
        std::cout << "Device SizeOfCode=" << dfu.devices[x].SizeOfCode << "\n";
        std::cout << "Device SizeOfDesc=" << dfu.devices[x].SizeOfDesc << "\n";
        std::cout << "BL Version=" << (uint32_t)dfu.devices[x].BL_Version << "\n";
        std::cout << "FW CRC=" << dfu.devices[x].FW_CRC << "\n";

        //int size = ((DFU::device)dfu.devices[x]).SizeOfDesc;
        //dfu.enterDFU(x);
        //std::cout << "Description:" << dfu.DownloadDescription(size) << "\n";
        //std::cout << "\n";
    }
    int device = 0;
    if (!dfu.enterDFU(device)) {
        std::cout << "Error:Could not enter DFU mode\n" << std::endl;
        return -1;
    }
    if (!dfu.devices[device].Writable) {
        std::cout << "ERROR device not Writable\n" << std::endl;
        return -1;
    }
    if (dfu.devices[device].SizeOfCode < loadedFW.size()) {
        std::cout << "ERROR file too big for device\n" << std::endl;
        return -1;
    }
    if (!forced) {
        std::string desc = loadedFW.substr(loadedFW.size() - 100);
        std::string head = "HxFw";
        if (desc.substr(0, head.size()) == head) {
            // Now do sanity checking:
            // - Check whether board type matches firmware:
            int board = dfu.devices[device].ID;
            int firmwareBoard = ((uint16_t) (uint8_t) desc.at(12) << 8) + (uint16_t) (uint8_t) desc.at(13);
            if (firmwareBoard != board) {
                std::cout << "Error: Device ID: firmware 0x" << std::hex << firmwareBoard << " does not match board 0x"
                          << std::hex << board << std::endl;
                return -1;
            }
        } else {
            // The firmware is not packaged, just upload the text in the description field
            // if it is there.
            std::cout << "Error: firmware is not packaged(.hxfw)" << std::endl;
            return -1;
        }
    }
    dfu.AbortOperation();

    // this call is asynchronous so the only false status it will report
    // is when it is already running...
    DFU::Status retstatus = dfu.UploadFirmware(filePath, verify, device);

    if (retstatus != DFU::Last_operation_Success) {
        std::cout << "Upload failed with code:" << retstatus << std::endl;
        return -1;
    }
    // TODO check if upload went well...
//    if (filePath.endsWith("opfw")) {
/*    std::ifstream fwfile(filePath);
    if (!fwfile.is_open()) {
        std::cout << "Cannot open file " << filePath << std::endl;
        return -1;
    }*/
/*    std::stringstream streambuf;
    streambuf << fwfile.rdbuf();
    std::string firmware(streambuf.str());
    //std::string strBuff(std::istreambuf_iterator<char>(fwfile), std::istreambuf_iterator<char>());
    //std::cout << strBuff << " xxxxxxx" << std::endl;
    std::string desc = firmware.substr(firmware.length() - 100, 100);
    DFU::Status status = dfu.UploadDescription(desc);
    if (status != DFU::Last_operation_Success) {
        std::cout << "Upload failed with code:" << status << std::endl;
        return -1;
    }*/
    // }// else if (!description.isEmpty()) {
//        DFU::Status status = qdfu.UploadDescription(description);
//        if (status != DFU::Last_operation_Success) {
//            std::cout << "Upload failed with code:" << status << std::endl;
//            return -1;
//        }
    //   }
    std::cout << "boot system.." << std::endl;
    dfu.JumpToApp(false, true);
    DFU::Status status = dfu.StatusRequest();
    if (status != DFU::abort) {
        std::cout << "boot failed with code:" << status << std::endl;
        return -1;
    }
    std::cout << "Uploading Succeeded!\n" << std::endl;
    return 0;
}
