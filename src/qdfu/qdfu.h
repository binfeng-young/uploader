/**
 ******************************************************************************
 *
 * @file       dfu.h
 * @author     The LibrePilot Project, http://www.librepilot.org Copyright (C) 2017.
 *             The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Uploader Plugin
 * @{
 * @brief The uploader plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef DFU_H
#define DFU_H

#include <QByteArray>
#include <QThread>
#include <QMutex>
#include <QList>
#include <QVariant>
#include "dfubase.h"

namespace DFU {
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
Q_NAMESPACE
#endif
Q_ENUMS(Status)

class DFUObject : public QThread {
    Q_OBJECT

public:
    static quint32 CRCFromQBArray(QByteArray array, quint32 Size);

    DFUObject(bool debug, bool use_serial, QString port);

    virtual ~DFUObject();

    // Service commands:
    bool enterDFU(int const &devNumber);
    bool findDevices();
    int JumpToApp(bool safeboot, bool erase);
    int ResetDevice(void);
    DFU::Status StatusRequest();
    bool EndOperation();
    int AbortOperation(void);
    bool ready();

    // Upload (send to device) commands
    DFU::Status UploadDescription(QVariant description);
    bool UploadFirmware(const QString &sfile, const bool &verify, int device);

    // Download (get from device) commands:
    // DownloadDescription is synchronous
    QString DownloadDescription(int const & numberOfChars);
    QByteArray DownloadDescriptionAsBA(int const & numberOfChars);
    // Asynchronous firmware download: initiates fw download,
    // and a downloadFinished signal is emitted when download
    // if finished:
    bool DownloadFirmware(QByteArray *byteArray, int device);

    // Comparison functions (is this needed?)
    DFU::Status CompareFirmware(const QString &sfile, const CompareType &type, int device);

    bool SaveByteArrayToFile(QString const & file, QByteArray const &array);

    // Variables:
    std::vector<device> devices;
    int numberOfDevices;
    int send_delay;
    bool use_delay;

    // Helper functions:
    QString StatusToString(DFU::Status const & status);
    DFU::eBoardType GetBoardType(int boardNum);

signals:
    void progressUpdated(int);
    void downloadFinished();
    void uploadFinished(DFU::Status);
    void operationProgress(QString status);

private:
    DFUBase *m_dfuBase;
//  void CopyWords(char *source, char *destination, int count);
    void printProgBar(int const & percent, QString const & label);
    DFU::Commands requestedOperation;
    qint32 requestSize;
    DFU::TransferTypes requestTransferType;
    QByteArray *requestStorage;
    QString requestFilename;
    bool requestVerify;
    int requestDevice;

protected:
    void run(); // Executes the upload or download operations
};
}

Q_DECLARE_METATYPE(DFU::Status)


#endif // DFU_H
