/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <pthread.h>

/** A scoped lock/unlock around a mutex. */
struct StackLockGuard
{
public:
    StackLockGuard(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_lock(mMutex); }
    ~StackLockGuard() { pthread_mutex_unlock(mMutex); }

private:
    pthread_mutex_t * mMutex;
};

/**
 * Use StackUnlockGuard to temporarily unlock the CHIP BLE stack, e.g. when calling application
 * or Android BLE code as a result of a BLE event.
 */
struct StackUnlockGuard
{
public:
    StackUnlockGuard(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_unlock(mMutex); }
    ~StackUnlockGuard() { pthread_mutex_lock(mMutex); }

private:
    pthread_mutex_t * mMutex;
};
