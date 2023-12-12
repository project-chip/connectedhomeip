/*
 *
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

#pragma once

#include <app/icd/client/ICDClientInfo.h>

namespace chip {
namespace app {

/// Callbacks for check in protocol
class DLL_EXPORT CheckInDelegate
{
public:
    virtual ~CheckInDelegate() {}

    /**
     * @brief Callback used to let the application know that a checkin message was received and validated.
     * When needRefreshKey is true,  it indicates that the ICD registration needs to be updated with a new key to avoid counter
     * roll-over problems.
     *
     * The implementor of this function should generate a new key and send it to CheckInHandler using CHIP_ERROR
     * CheckInMessageHandler::SetNewKey(ICDClientInfo & clientInfo, const ByteSpan keyData)
     *
     * @param[in] clientInfo - ClientInfo object of the peer node
     * @param[in] needRefreshKey - Indicates whether the application should refresh the existing key
     */
    virtual void OnCheckInComplete(ICDClientInfo & clientInfo, bool needRefreshKey) = 0;
};

} // namespace app
} // namespace chip
