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

class RefreshKeySender;

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
     * The implementer of this function should create a new RefreshKeySender object. This object will be tied to the specific key
     * refresh process and will not be used by the caller after that particular key refresh process has ended, regardless of success
     * or failure.
     *
     * The caller guarantees that if a non-null RefreshKeySender pointer is returned, it will call OnKeyRefreshDone
     * at some point, and pass it the returned pointer.
     *
     * If the callee is unable to provide the RefreshKeySender object, that indicates key
     * refresh is not possible until the callee is able to provide the required resources.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
     *                         node that sent the check-in message. The callee can use the clientInfo to determine the type of key
     *                         to generate.
     * @param[in] clientStorage - ICDClientStorage object stores the updated ICDClientInfo after re-registration into
     *                            persistent storage.
     * @return RefreshKeySender - pointer to RefreshKeySender object
     */
    virtual RefreshKeySender * OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage) = 0;

    /**
     * @brief Callback used to let the application know that the re-registration process is done. This callback will be called for
     * both success and failure cases. On failure, the callee should take appropriate corrective action based on the error.
     *
     * @param[in] refreshKeySender - pointer to the RefreshKeySender object that was used for the key refresh process. It will NOT
     *                               be null regardless the key refresh status. The caller will NOT use this pointer any more.
     * @param[in] error - CHIP_NO_ERROR indicates successful re-registration using the new key
     *                     Other errors indicate the failure reason.
     */
    virtual void OnKeyRefreshDone(RefreshKeySender * refreshKeySender, CHIP_ERROR error) = 0;
};

} // namespace app
} // namespace chip
