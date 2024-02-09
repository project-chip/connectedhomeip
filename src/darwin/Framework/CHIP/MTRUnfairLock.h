/**
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 * RAII wrapper around os_unfair_lock.
 */

#import <os/lock.h>

#include <mutex>

template <>
class std::lock_guard<os_unfair_lock>
{
public:
    explicit lock_guard(os_unfair_lock & lock) : mLock(lock) { os_unfair_lock_lock(&mLock); }
    ~lock_guard() { os_unfair_lock_unlock(&mLock); }

    lock_guard(const lock_guard &)     = delete;
    void operator=(const lock_guard &) = delete;

private:
    os_unfair_lock & mLock;
};
