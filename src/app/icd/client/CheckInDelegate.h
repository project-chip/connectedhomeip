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

class ICDRefreshKeyInfo;

/// Callbacks for check in protocol
/**
 * @brief The application implementing an ICD client should inherit the CheckInDelegate and implement the listed callbacks
 */
class DLL_EXPORT CheckInDelegate
{
public:
    virtual ~CheckInDelegate() {}

    /**
     * @brief Callback used to let the application know that a check-in message was received and validated.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message.
     */
    virtual void OnCheckInComplete(const ICDClientInfo & clientInfo) = 0;

    /**
     * @brief Callback used to let the application know that a key refresh is
     * needed to avoid counter rollover problems.
     *
     * The implementer of this function should generate a new key and store it in a map with ScopedNodeID as the key and
     * ICDRefreshKeyInfo as the value.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. The callee can use the clientInfo to determine the type of key
                               to generate.
     * @return ICDRefreshKeyInfo - pointer to the ICDRefreshKeyInfo object comprising the newly generated key, the input
                                  ICDClientInfo, pointer to the CheckInDelegate and CommandSender object for re-registering with the
                                  new key. The implementer of this function should allocate the ICDRefreshKeyInfo object on
                                  receiving this callback and de-allocate the object once the re-registration is complete and
                                  receives OnRegistrationUpdateComplete.

    */
    virtual ICDRefreshKeyInfo * OnKeyRefreshNeeded(const ICDClientInfo & clientInfo) = 0;
};

} // namespace app
} // namespace chip
