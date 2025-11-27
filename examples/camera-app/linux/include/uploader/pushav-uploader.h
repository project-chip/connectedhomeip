/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <atomic>
#include <curl/curl.h>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

typedef struct UploadDataInfo
{
    char * mData;
    long mSize;
    long mBytesRead;
} PushAvUploadInfo;

class PushAVUploader
{
public:
    typedef struct CertificatesPathInfo
    {
        std::string mRootCert;
        std::string mDevCert;
        std::string mDevKey;
    } PushAVCertPath;

    typedef struct CertificatesInfo
    {
        std::vector<uint8_t> mRootCertBuffer;
        std::vector<uint8_t> mClientCertBuffer;
        std::vector<uint8_t> mClientKeyBuffer;
        std::vector<std::vector<uint8_t>> mIntermediateCertBuffer;
    } PushAVCertBuffer;

    PushAVUploader();
    ~PushAVUploader();

    void Start();
    void Stop();
    void AddUploadData(std::string & filename, std::string & url);
    size_t GetUploadQueueSize()
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        return mAvData.size();
    }

    void setCertificateBuffer(const PushAVCertBuffer & certBuffer) { mCertBuffer = certBuffer; }
    void setCertificatePath(const PushAVCertPath & certPath) { mCertPath = certPath; }

    void setMPDPath(const std::pair<std::string, std::string> & path) { mMPDPath = path; }
    std::pair<std::string, std::string> getMPDPath() const { return mMPDPath; }

private:
    void ProcessQueue();
    void UploadData(std::pair<std::string, std::string> data);
    PushAVCertPath mCertPath;
    PushAVCertBuffer mCertBuffer;
    std::queue<std::pair<std::string, std::string>> mAvData;
    std::mutex mQueueMutex;
    std::atomic<bool> mIsRunning;
    std::thread mUploaderThread;
    std::pair<std::string, std::string> mMPDPath;
};
