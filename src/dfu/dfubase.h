//
// Created by bfyoung on 2/23/18.
//

#ifndef UPLOADER_DFUBASE_H
#define UPLOADER_DFUBASE_H

#include <vector>
#include <string>
#include <mutex>
#include "../utils.h"

#define MAX_PACKET_DATA_LEN 255
#define MAX_PACKET_BUF_SIZE (1 + 1 + MAX_PACKET_DATA_LEN + 2)

#define BUF_LEN             64

// serial
class sspt;
class port;
class SerialPort;

// usb
//class opHID_hidapi;

namespace DFU {
    enum TransferTypes {
        FW,
        Descript
    };

    enum CompareType {
        crccompare,
        bytetobytecompare
    };
    enum Status {
        DFUidle, // 0
        uploading, // 1
        wrong_packet_received, // 2
        too_many_packets, // 3
        too_few_packets, // 4
        Last_operation_Success, // 5
        downloading, // 6
        idle, // 7
        Last_operation_failed, // 8
        uploadingStarting, // 9
        outsideDevCapabilities, // 10
        CRC_Fail, // 11
        failed_jump, // 12
        abort // 13
    };

    enum Actions {
        actionNone,
        actionProgram,
        actionProgramAndVerify,
        actionDownload,
        actionCompareAll,
        actionCompareCrc,
        actionListDevs,
        actionStatusReq,
        actionReset,
        actionJump
    };

    enum Commands {
        Reserved, // 0
        Req_Capabilities, // 1
        Rep_Capabilities, // 2
        EnterDFU, // 3
        JumpFW, // 4
        Reset, // 5
        Abort_Operation, // 6
        Upload, // 7
        END, // 8
        Download_Req, // 9
        Download, // 10
        Status_Request, // 11
        Status_Rep, // 12
    };

    enum eBoardType {
        eBoardUnkwn   = 0,
        eBoardMainbrd = 1,
        eBoardINS,
        eBoardPip     = 3,
        eBoardCC = 4,
        eBoardRevo    = 9,
        eBoardSparky2 = 0x92,
    };

    struct device {
        uint16_t ID;
        uint32_t FW_CRC;
        uint8_t  BL_Version;
        int     SizeOfDesc;
        uint32_t SizeOfCode;
        bool    Readable;
        bool    Writable;
    };

    class DFUBase {
    public:
        static uint32_t CRCFromQBArray(std::string array, uint32_t Size);

        DFUBase(bool debug, bool use_serial,const std::string& port);
        DFUBase(bool debug, SerialPort* serialPort);

        virtual ~DFUBase();

        // Service commands:
        bool enterDFU(int const &devNumber);
        bool findDevices();
        int JumpToApp(bool safeboot, bool erase);
        int ResetDevice(void);
        DFU::Status StatusRequest();
        bool EndOperation();
        int AbortOperation(void);
        bool ready()
        {
            return mready;
        }

        // Upload (send to device) commands
        DFU::Status UploadDescription(const std::string& desc);
        DFU::Status UploadFirmware(const std::string &sfile, const bool &verify, int device);

        // Download (get from device) commands:
        // DownloadDescription is synchronous
        std::string DownloadDescription(int const & numberOfChars);
        std::string DownloadDescriptionAsBA(int const & numberOfChars);
        // Asynchronous firmware download: initiates fw download,
        // and a downloadFinished signal is emitted when download
        // if finished:
        std::string DownloadFirmware(int sizeOfCode);

        // Comparison functions (is this needed?)
        DFU::Status CompareFirmware(const std::string &sfile, const CompareType &type, int device);

        bool SaveByteArrayToFile(std::string const & file, std::string const &array);

        // Variables:
        std::vector<device> devices;
        int numberOfDevices;
        int send_delay;
        bool use_delay;

        // Helper functions:
        std::string StatusToString(DFU::Status const & status);
        static uint32_t CRC32WideFast(uint32_t Crc, uint32_t Size, uint32_t *Buffer);
        DFU::eBoardType GetBoardType(int boardNum);
        void printProgBar(int const & percent, std::string const & label);
    private:
        // Generic variables:
        bool debug;
        bool use_serial;
        bool mready;
        int RWFlags;

        // Serial
        sspt *serialhandle;

        // USB
        // opHID_hidapi *hidHandle;

        int sendData(void *, int);
        int receiveData(void *data, int size);
        uint8_t sspTxBuf[MAX_PACKET_BUF_SIZE];
        uint8_t sspRxBuf[MAX_PACKET_BUF_SIZE];

        int setStartBit(int command)
        {
            return command | 0x20;
        }

        void CopyWords(char *source, char *destination, int count);
        bool StartUpload(int32_t const &numberOfBytes, TransferTypes const & type, uint32_t crc);
        bool UploadData(int32_t const & numberOfPackets, const std::string & data);

        // Thread management:
        // Same as startDownload except that we store in an external array:
        bool StartDownloadT(std::string &fw, int32_t const & numberOfBytes, TransferTypes const & type);
        void initPort(port*& info);
    };
}


#endif //UPLOADER_DFUBASE_H
