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

PushAVUploader::PushAVUploader() : running(false) {}

PushAVUploader::~PushAVUploader() {
    Stop();
}

void PushAVUploader::ProcessUploadQueue() {
    while (running) {
	std::pair<std::string, std::string> data;
        {
           std::lock_guard<std::mutex> lock(queue_mutex);
           if (!av_data.empty()) {
                data = av_data.front();
                av_data.pop();
            }
        }
        if (!data.first.empty())
            UploadData(data);
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void PushAVUploader::Start() {
    if (!running) {
        running = true;
        uploader_thread = std::thread(&PushAVUploader::ProcessUploadQueue, this);
    }
}

void PushAVUploader::Stop() {
    if (running) {
        running = false;
        if (uploader_thread.joinable()) {
            uploader_thread.join();
        }
    }
}

void PushAVUploader::AddFileToUpload(std::string& filename, std::string& url) {
    ChipLogError(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(queue_mutex);
    auto data = make_pair(filename, url);
    av_data.push(data);
}

size_t PushAvUploadCb(void * ptr, size_t size, size_t nmemb, void * stream)
{
    int len                   = (int) (size * nmemb);
    PushAvUploadInfo * upload = (PushAvUploadInfo *) stream;
    if (ptr == NULL)
    {
        ChipLogError(Camera, "ptr is null");
        return 0;
    }
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
    {
        ChipLogError(Camera, "_upload_file_read_callback size = %d nmemb = %d %d\n", size, nmemb, size * nmemb);
        return 0;
    }
    int buffer_size = size * nmemb;
    int remaining   = upload->size - upload->bytes_read;
    int to_copy     = (remaining < buffer_size) ? remaining : buffer_size;

    if (to_copy)
    {
        memcpy(ptr, upload->data + upload->bytes_read, to_copy);
        upload->bytes_read += to_copy;
    }
    return to_copy;
}
void PushAVUploader::UploadData(std::pair<std::string, std::string> data)
{
    CURL * curl = curl_easy_init();
    if (!curl)
    {
        ChipLogError(Camera, "Failed to initialize CURL");
        return;
    }

    //TODO: Remove these values and get details from trasport/recoder
    PushAVCertPath path;
    path.root_cert = "/root/.pavstest/certs/server/root.pem";
    path.dev_cert  = "/root/.pavstest/certs/device/dev.pem";
    path.dev_key   = "/root/.pavstest/certs/device/dev.key";

    std::ifstream file(data.first.c_str(), std::ios::binary);
    if (!file)
    {
        ChipLogError(Camera, "Failed to open file");
        return;
    }
    int size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        ChipLogError(Camera, "Failed to read file into buffer");
        return;
    }
    file.close();
    PushAvUploadInfo upload;
    upload.data                 = buffer.data();
    upload.size                 = size;
    upload.bytes_read           = 0;
    struct curl_slist * headers = nullptr;
    headers                     = curl_slist_append(headers, "Content-Type: application/*");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    std::string fullUrl = data.second + data.first;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));
    curl_easy_setopt(curl, CURLOPT_CAINFO, path.root_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, path.dev_cert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, path.dev_key.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, PushAvUploadCb);
    curl_easy_setopt(curl, CURLOPT_READDATA, upload);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        ChipLogError(Camera, "CURL upload failed %s", curl_easy_strerror(res));
    } else {
        ChipLogError(Camera, "CURL uploaded file  %s", data.first.c_str());
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}

