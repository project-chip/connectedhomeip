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

static size_t pushAVCurlCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  return 0;
}

PushAVUploader::PushAVUploader() : running(false) {}

PushAVUploader::~PushAVUploader() {
    stop();
}

void PushAVUploader::process_queue() {
    while (running) {
	std::pair<std::string, std::string> data;
        {
           std::lock_guard<std::mutex> lock(queue_mutex);
           if (!av_data.empty()) {
                data = av_data.front();
                av_data.pop();
            }
        }
        if (!av_data.empty()) {
            upload_data(data);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void PushAVUploader::start() {
    if (!running) {
        running = true;
        uploader_thread = std::thread(&PushAVUploader::process_queue, this);
    }
}

void PushAVUploader::stop() {
    if (running) {
        running = false;
        if (uploader_thread.joinable()) {
            uploader_thread.join();
        }
    }
}

void PushAVUploader::add_uploadData(std::string& filename, std::string& url) {
    ChipLogError(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(queue_mutex);
    auto data = make_pair(filename, url);
    av_data.push(data);
}

void PushAVUploader::upload_data(std::pair<std::string, std::string> data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        ChipLogError(Camera, "Failed to initialize CURL");
        return;
    }
    curl_mime *form;
    curl_mimepart *field;
    struct curl_slist *header;
    std::string response;

    form = curl_mime_init(curl);
    field = curl_mime_addpart(form);
    curl_mime_name(field, "file");
    curl_mime_filedata(field, data.first.c_str());
    header = curl_slist_append(header, "Content-Type: multipart/form-data");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_URL, data.second.c_str());
    //TODO:Enable below code for saving the response header
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pushAVCurlCallback);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);    
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);    

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        ChipLogError(Camera, "CURL upload failed %s", curl_easy_strerror(res));
    } else {
        ChipLogError(Camera, "CURL uploaded file  %s", data.first.c_str());
    }

    curl_mime_free(form);
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
}

