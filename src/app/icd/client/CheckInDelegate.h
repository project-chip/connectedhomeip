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
/**
 * @brief The application implementing an ICD client should inherit the CheckInDelegate and implement the listed callbacks
 * OnCheckInComplete will be called on successful processing of a received checkIn message from the server
 * OnRefreshKey will be called when the key needs to be refreshed to avoid check in counter roll over problems. On receiving
 * OnRefreshKey callback, the application should generate a new key.
 */
class DLL_EXPORT CheckInDelegate
{
public:
    virtual ~CheckInDelegate() {}

    /**
     * @brief Callback used to let the application know that a checkin message was received and validated.
     *
     * @param[in] clientInfo - ClientInfo object of the peer node
     */
    virtual void OnCheckInComplete(ICDClientInfo & clientInfo) = 0;

    /**
     * @brief Callback used to let the application know that a checkin message was received and validated and a key refresh is
     * needed to avoid counter roolover problems.
     *
     * The implementer of this function should generate a new key
     *
     * @param[out] keyData - new key generated
     */
    virtual void OnRefreshKey(ByteSpan & keyData) = 0;
};

} // namespace app
} // namespace chip
