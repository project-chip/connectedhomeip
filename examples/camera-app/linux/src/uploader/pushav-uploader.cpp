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
#include <lib/support/logging/CHIPLogging.h>

PushAVUploader::PushAVUploader() : mIsRunning(false) {}

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

void PushAVUploader::AddUploadData(const std::string & filename, const std::string & url)
{
    ChipLogError(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(mQueueMutex);
    auto data = make_pair(filename, url);
    mAvData.push(data);
}

void PushAVUploader::UploadData(std::pair<std::string, std::string> data)
{
    CURL * curl = curl_easy_init();
    if (!curl)
    {
        ChipLogError(Camera, "Failed to initialize CURL");
        return;
    }
    curl_mime * form;
    curl_mimepart * field;
    struct curl_slist * header;
    std::string response;

    form  = curl_mime_init(curl);
    field = curl_mime_addpart(form);
    curl_mime_name(field, "file");
    curl_mime_filedata(field, data.first.c_str());
    header = curl_slist_append(header, "Content-Type: multipart/form-data");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_URL, data.second.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        ChipLogError(Camera, "CURL upload failed %s", curl_easy_strerror(res));
    }
    else
    {
        ChipLogError(Camera, "CURL uploaded file  %s", data.first.c_str());
    }

    curl_mime_free(form);
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
}
