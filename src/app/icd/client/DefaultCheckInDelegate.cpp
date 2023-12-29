/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CheckInHandler.h"
#include <app/icd/client/DefaultCheckInDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>

namespace chip {
namespace app {

CHIP_ERROR DefaultCheckInDelegate::Init(ICDClientStorage * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpStorage = storage;
    return CHIP_NO_ERROR;
}

void DefaultCheckInDelegate::OnCheckInComplete(const ICDClientInfo & clientInfo)
{
    ChipLogProgress(
        ICD, "Check In Message processing complete: start_counter=%" PRIu32 " offset=%" PRIu32 " nodeid=" ChipLogFormatScopedNodeId,
        clientInfo.start_icd_counter, clientInfo.offset, ChipLogValueScopedNodeId(clientInfo.peer_node));
}

void DefaultCheckInDelegate::OnRefreshKeyGenerate(const ICDClientInfo & clientInfo, uint8_t * keyData, uint8_t keyLength)
{
    chip::Crypto::DRBG_get_bytes(keyData, keyLength);
    ICDRefreshKeyInfo refreshKeyInfo;
    refreshKeyInfo.clientInfo = clientInfo;
    memcpy(&refreshKeyInfo.newKey, keyData, keyLength);
    icdRefreshKeyMap.insert(make_pair(clientInfo.peer_node, refreshKeyInfo));
}

CHIP_ERROR DefaultCheckInDelegate::OnRefreshKeyRetrieve(const ScopedNodeId & nodeId, ICDRefreshKeyInfo & refreshKeyInfo)
{
    // Todo : Check if CHIP_ERROR_KEY_NOT_FOUND can be used here
    VerifyOrReturnError(icdRefreshKeyMap.find(nodeId) != icdRefreshKeyMap.end(), CHIP_ERROR_KEY_NOT_FOUND);
    refreshKeyInfo = icdRefreshKeyMap.at(nodeId);
    return CHIP_NO_ERROR;
}

void DefaultCheckInDelegate::OnRegistrationComplete(const ICDClientInfo & clientInfo) {}

} // namespace app
} // namespace chip
