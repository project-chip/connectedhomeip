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

class MTRAutoUnfairLock;

class MTRUnfairLock
{
public:
    MTRUnfairLock() { mOSLock = OS_UNFAIR_LOCK_INIT; }

    void AssertOwner() { os_unfair_lock_assert_owner(&mOSLock); }

private:
    friend class MTRAutoUnfairLock;
    os_unfair_lock mOSLock;
};

class MTRAutoUnfairLock
{
public:
    MTRAutoUnfairLock(MTRUnfairLock & aLock) : mLock(aLock) { os_unfair_lock_lock(&mLock.mOSLock); }

    ~MTRAutoUnfairLock() { os_unfair_lock_unlock(&mLock.mOSLock); }

private:
    MTRUnfairLock & mLock;
};
