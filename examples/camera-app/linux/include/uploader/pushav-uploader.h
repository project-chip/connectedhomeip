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

typedef struct upload_data_info
{
    char * data;
    int size;
    int bytes_read;
} PushAvUploadInfo;

typedef struct cerficates_info
{
    std::string root_cert;
    std::string dev_cert;
    std::string dev_key;
} PushAVCertPath;

class PushAVUploader
{
public:
    PushAVUploader();
    ~PushAVUploader();

    void start();
    void stop();
    void add_uploadData(std::string& filename, std::string& url);

private:
    void process_queue();
    void upload_data(std::pair<std::string, std::string> data, PushAVCertPath * path = nullptr);

    std::queue<std::pair <std::string, std::string>> av_data;
    std::mutex queue_mutex;
    std::atomic<bool> running;
    std::thread uploader_thread;
};

