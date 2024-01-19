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
     * The implementer of this function should dynamically create a new ICDRefreshKeyInfo object. They should also generate a new
       key and store it in the ICDRefreshKeyInfo object.
       The implementer of this function should use CreateICDRefreshKeyInfo() to create the object
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. The callee can use the clientInfo to determine the type of key
                               to generate.
     * @param[in] clientStorage - ICDClientStorage object stores the updated ICDClientInfo after re-registration into
                                  persistent storage.
     * @return ICDRefreshKeyInfo - pointer to the ICDRefreshKeyInfo object comprising the newly generated key, the input
                                  ICDClientInfo, pointer to the CheckInDelegate and CommandSender object for re-registering with the
                                  new key.
    */
    virtual ICDRefreshKeyInfo * OnKeyRefreshNeeded(const ICDClientInfo & clientInfo, const ICDClientStorage * clientStorage) = 0;

    /**
     * @brief Callback used to let the application know that the re-registration with the new key was successful and provides the
     * updated ICDClientInfo
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. This will have the new key used for registration and the updated
                               icd counter. The application should clean up the ICDRefreshKeyInfo object using
                               ReleaseICDRefreshKeyInfo() on receipt.
     */
    virtual void OnRegistrationUpdateComplete(const ICDClientInfo & clientInfo, ICDRefreshKeyInfo * icdRefreshKeyInfo) = 0;

    /**
     * @brief Callback used to let the application know that the re-registration with the new key has failed. The application should
              clean up the ICDRefreshKeyInfo object using ReleaseICDRefreshKeyInfo() on receipt.
     */
    virtual void OnRegistrationUpdateFailure(ICDRefreshKeyInfo * icdRefreshKeyInfo) = 0;

    /**
     * @brief Function used to dynamically create an ICDRefreshKeyInfo object.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message.
     * @param[in] clientStorage - ICDClientStorage object used to store the updated ICDClientInfo(with the new key and start ICD
                                  counter) to persistent storage.
     * @return ICDRefreshKeyInfo - pointer to the new ICDRefreshKeyInfo object created
     */
    virtual ICDRefreshKeyInfo * CreateICDRefreshKeyInfo(const ICDClientInfo & clientInfo,
                                                        const ICDClientStorage * clientStorage) = 0;

    /**
     * @brief Function used to destroy an ICDRefreshKeyInfo object at the end of a key refresh process. This function should be
              called for both success and failure cases of key refresh to avoid memory leak.
     *
     * @param[in] icdRefreshKeyInfo - pointer to the ICDRefreshKeyInfo object created for the key refresh process
     */
    virtual void ReleaseICDRefreshKeyInfo(ICDRefreshKeyInfo * icdRefreshKeyInfo) = 0;
};

} // namespace app
} // namespace chip
