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

#include "pushav-uploader.h"
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

PushAVUploader::PushAVUploader(PushAVCertPath certPath) : mCertPath(certPath), mIsRunning(false) {}

PushAVUploader::~PushAVUploader()
{
    Stop();
}

void PushAVUploader::ProcessQueue()
{
    while (mIsRunning)
    {
        std::pair<std::string, std::string> uploadJob;
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            if (!mAvData.empty())
            {
                uploadJob = std::move(mAvData.front());
                mAvData.pop();
            }
        }
        if (!uploadJob.first.empty() && !uploadJob.second.empty())
        {
            UploadData(uploadJob);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void PushAVUploader::Start()
{
    if (!mIsRunning)
    {
        mIsRunning      = true;
        mUploaderThread = std::thread(&PushAVUploader::ProcessQueue, this);
    }
}

void PushAVUploader::Stop()
{
    if (mIsRunning)
    {
        mIsRunning = false;
        if (mUploaderThread.joinable())
        {
            mUploaderThread.join();
        }
    }
}

void PushAVUploader::AddUploadData(std::string & filename, std::string & url)
{
    ChipLogProgress(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(mQueueMutex);
    auto data = make_pair(filename, url);
    mAvData.push(data);
}

size_t PushAvUploadCb(void * ptr, size_t size, size_t nmemb, void * stream)
{
    int bufferSize            = (int) (size * nmemb);
    PushAvUploadInfo * upload = (PushAvUploadInfo *) stream;
    if (ptr == NULL)
    {
        ChipLogError(Camera, "Invalid destination pointer");
        return 0;
    }
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
    {
        ChipLogError(Camera, "Zero buffer size = %ld nmemb = %ld %ld\n", size, nmemb, size * nmemb);
        return 0;
    }
    long remaining          = upload->mSize - upload->mBytesRead;
    unsigned long copyChunk = (unsigned long) ((remaining < bufferSize) ? remaining : bufferSize);

    if (copyChunk)
    {
        memcpy(ptr, upload->mData + upload->mBytesRead, copyChunk);
        upload->mBytesRead += copyChunk;
    }
    return (size_t) copyChunk;
}

void PushAVUploader::UploadData(std::pair<std::string, std::string> data)
{
    CURL * curl = curl_easy_init();
    if (!curl)
    {
        ChipLogError(Camera, "Failed to initialize CURL");
        return;
    }

    std::ifstream file(data.first.c_str(), std::ios::binary);
    if (!file)
    {
        ChipLogError(Camera, "Failed to open file %s", data.first.c_str());
        return;
    }
    file.seekg(0, std::ios::end);
    unsigned long size = (unsigned long) file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), static_cast<std::streamsize>(size)))
    {
        ChipLogError(Camera, "Failed to read file into buffer");
        return;
    }
    file.close();
    PushAvUploadInfo upload;
    upload.mData = (char *) std::malloc(size);
    memcpy(upload.mData, buffer.data(), size);
    upload.mSize                = static_cast<long>(size);
    upload.mBytesRead           = 0;
    struct curl_slist * headers = nullptr;
    headers                     = curl_slist_append(headers, "Content-Type: application/*");

    // Extract just the filename from the full path
    std::string fullPath = data.first;
    std::string filename = fullPath.substr(fullPath.find_last_of("/\\") + 1);

    // Construct the URL with just the filename
    std::string baseUrl = data.second;
    if (baseUrl.back() != '/')
    {
        baseUrl += "/";
    }
    std::string fullUrl = baseUrl + filename;

    ChipLogProgress(Camera, "Uploading file: %s to URL: %s", filename.c_str(), fullUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));
    curl_easy_setopt(curl, CURLOPT_CAINFO, mCertPath.mRootCert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, mCertPath.mDevCert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, mCertPath.mDevKey.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, PushAvUploadCb);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        ChipLogError(Camera, "CURL upload  failed [%s] %s", data.first.c_str(), curl_easy_strerror(res));
    }
    else
    {
        ChipLogError(Camera, "CURL uploaded file  %s size: %ld", data.first.c_str(), size);
    }
    if (upload.mData)
    {
        std::free(upload.mData);
        upload.mData = nullptr;
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}
