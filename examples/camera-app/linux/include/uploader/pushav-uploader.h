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
    typedef struct CertificatesInfo
    {
        std::string mRootCert;
        std::string mDevCert;
        std::string mDevKey;
    } PushAVCertPath;

    PushAVUploader(PushAVCertPath certPath);
    ~PushAVUploader();

    void Start();
    void Stop();
    void AddUploadData(std::string & filename, std::string & url);
    size_t GetUploadQueueSize()
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        return mAvData.size();
    }

private:
    void ProcessQueue();
    void UploadData(std::pair<std::string, std::string> data);
    PushAVCertPath mCertPath;
    std::queue<std::pair<std::string, std::string>> mAvData;
    std::mutex mQueueMutex;
    std::atomic<bool> mIsRunning;
    std::thread mUploaderThread;
};
