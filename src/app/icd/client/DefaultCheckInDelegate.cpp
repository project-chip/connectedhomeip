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

#include <app/icd/client/DefaultCheckInDelegate.h>
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

void DefaultCheckInDelegate::OnCheckInComplete(const ICDClientInfo & clientInfo, bool needRefreshKey)
{
    mpStorage->StoreEntry(clientInfo);
    ChipLogProgress(
        ICD, "Check In Message processing complete: start_counter=%" PRIu32 " offset=%" PRIu32 " nodeid=" ChipLogFormatScopedNodeId,
        clientInfo.start_icd_counter, clientInfo.offset, ChipLogValueScopedNodeId(clientInfo.peer_node));
    if (needRefreshKey)
    {
        // TODO : Refresh key and re-register client
    }
}

} // namespace app
} // namespace chip
