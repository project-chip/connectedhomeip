/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <condition_variable>
#include <mutex>
#include <thread>

#include "TestMdns.h"

int main()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable done;
    int retVal = -1;

    std::thread t([&done, &retVal]() {
        retVal = TestMdns();
        done.notify_all();
    });

    if (done.wait_for(lock, std::chrono::seconds(5)) == std::cv_status::timeout)
    {
        fprintf(stderr, "mDNS test timeout, is avahi daemon running?");
    }
    return retVal;
}
