/*
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

/**
 *    @file
 *      This file defines the CHIP message counters.
 *
 */

#include <transport/MessageCounter.h>

#include <lib/support/RandUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {

GlobalUnencryptedMessageCounter::GlobalUnencryptedMessageCounter() : value(GetRandU32()) {}

CHIP_ERROR GlobalEncryptedMessageCounter::Init()
{
    return persisted.Init(CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER, 1000);
}

} // namespace chip
