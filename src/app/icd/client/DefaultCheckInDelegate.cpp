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

#include <app/InteractionModelEngine.h>
#include <app/icd/client/DefaultCheckInDelegate.h>
#include <app/icd/client/RefreshKeySender.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

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
#if CHIP_CONFIG_ENABLE_READ_CLIENT
    InteractionModelEngine::GetInstance()->OnActiveModeNotification(clientInfo.peer_node);
#endif
}

RefreshKeySender * DefaultCheckInDelegate::OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RefreshKeySender::RefreshKeyBuffer newKey;

    err = Crypto::DRBG_get_bytes(newKey.Bytes(), newKey.Capacity());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ICD, "Generation of new key failed: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    auto refreshKeySender = Platform::New<RefreshKeySender>(this, clientInfo, clientStorage, newKey);
    if (refreshKeySender == nullptr)
    {
        return nullptr;
    }
    return refreshKeySender;
}

void DefaultCheckInDelegate::OnKeyRefreshDone(RefreshKeySender * refreshKeySender, CHIP_ERROR error)
{
    if (error == CHIP_NO_ERROR)
    {
        ChipLogProgress(ICD, "Re-registration with new key completed successfully");
    }
    else
    {
        ChipLogError(ICD, "Re-registration with new key failed with error : %" CHIP_ERROR_FORMAT, error.Format());
        // The callee can take corrective action  based on the error received.
    }
    if (refreshKeySender != nullptr)
    {
        Platform::Delete(refreshKeySender);
        refreshKeySender = nullptr;
    }
}
} // namespace app
} // namespace chip
