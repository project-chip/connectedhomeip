/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "ICDClientInfo.h"
#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace app {

/*
 *@brief Structure to hold the clientInfo and the new key generated to re-register with a peer
 */
struct ICDRefreshKeyInfo
{
    ICDClientInfo clientInfo;
    uint8_t newKey[chip::Crypto::kAES_CCM128_Key_Length];

    ICDRefreshKeyInfo() {}
    ICDRefreshKeyInfo(const ICDRefreshKeyInfo & other) { *this = other; }

    ICDRefreshKeyInfo & operator=(const ICDRefreshKeyInfo & other)
    {
        clientInfo = other.clientInfo;
        memcpy(newKey, other.newKey, chip::Crypto::kAES_CCM128_Key_Length);
        return *this;
    }
};

} // namespace app
} // namespace chip