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
#include <app/icd/client/ICDClientStorage.h>

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
     * The implementer of this function should create a new ICDRefreshKeyInfo object and a CommandSender delegate object. They
       should also generate a new key and store it in the ICDRefreshKeyInfo object. This object will be tied to the specific key
       refresh process and will not be used by the caller after that particular key refresh process has ended, regardless of success
       or failure. On creating both the objects successfully, the callee should use the EstablishSession() API in ICDRefreshKeyInfo
       to start a CASE session for re-registration.

     * If the callee is unable to provide either the ICDRefreshKeyInfo or the CommandSender Delegate objects, that indicates key
     * refresh is not possible until the callee is able to provide the required resources.
     *
     * @param[in] clientInfo - pointer to ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. The callee can use the clientInfo to determine the type of key
                               to generate.
     * @param[in] clientStorage - ICDClientStorage object stores the updated ICDClientInfo after re-registration into
                                  persistent storage.
     */
    virtual void OnKeyRefreshNeeded(ICDClientInfo * clientInfo, ICDClientStorage * clientStorage) = 0;

    /**
     * @brief Callback used to let the application know that the re-registration with the new key was successful and provides the
     * updated ICDClientInfo
     *
     * @param[in] clientInfo - pointer to ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. This will have the new key used for registration and the updated
                               icd counter.
     * @param[in] ICDRefreshKeyInfo - a pointer to the ICDRefreshKeyInfo object comprising the newly
                                      generated key, the input ICDClientInfo, pointer to the CheckInDelegate and CommandSender
                                      object for re-registering with the new key. The caller will NOT use icdRefreshKeyInfo passed
                                      here anymore.
     */
    virtual void OnRegistrationUpdateComplete(const ICDClientInfo * clientInfo, ICDRefreshKeyInfo * icdRefreshKeyInfo) = 0;

    /**
     * @brief Callback used to let the application know that the re-registration with the new key has failed. The caller will NOT
     *        use icdRefreshKeyInfo passed here anymore.
     * @param[in] ICDRefreshKeyInfo - a pointer to the ICDRefreshKeyInfo object comprising the newly generated key, the input
                                      ICDClientInfo, pointer to the CheckInDelegate and CommandSender object for re-registering with
                                      the new key. The caller will NOT use icdRefreshKeyInfo passed
                                      here anymore.
     * @param[in] failureReason - reason for regitration failure. The callee can use this informarion to take necessary action to
                                  resolve the failure.
     */
    virtual void OnRegistrationUpdateFailure(ICDRefreshKeyInfo * icdRefreshKeyInfo, CHIP_ERROR failureReason) = 0;
};

} // namespace app
} // namespace chip
