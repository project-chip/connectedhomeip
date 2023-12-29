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
#include <app/icd/client/ICDRefreshKeyInfo.h>

namespace chip {
namespace app {

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
     * The implementer of this function should generate a new key and store it in a map with peer nodeID as the key and
     * ICDRefreshKeyInfo as the value.
     *
     * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                               node that sent the check-in message. The callee can use the clientInfo to determine the type of key
                               to generate.
     * @param[out] keyData - pointer to the keyData buffer of size keyDataLength. The implementer of this callback should generate a
                             new key of size keyLength and copy it to the keyData buffer
     * @param[in] keyLength - length of the new key to be generated
     */
    virtual void OnRefreshKeyGenerate(const ICDClientInfo & clientInfo, uint8_t * keyData, uint8_t keyLength) = 0;

    /**
     * @brief Callback used to retrieve the refresh key information from the application after establishing a new secure session for
     * re-registration. The application should maintain a map to store the corresponding ICDRefreshKeyInfo for every peer node.
     * Please refer to ICDRefreshKeyInfo.h for details.
     *
     * @param[in] nodeId - node ID of the peer with whom the client needs to re-register with a new key to avoid rollover problems.
     * @param[out] refreshKeyInfo - stored refreshKeyInfo for the corresponding nodeId from the ICDRefreshKeyMap
     */
    virtual CHIP_ERROR OnRefreshKeyRetrieve(const ScopedNodeId & nodeId, ICDRefreshKeyInfo & refreshKeyInfo) = 0;

    /**
 * @brief Callback used to let the application know that the re-registration with the new key was successful and provides the
 * updated ICDClientInfo
 *
 * @param[in] clientInfo - ICDClientInfo object representing the state associated with the
                           node that sent the check-in message. This will have the new key used for registration and the updated icd
                           counter.
 */
    virtual void OnRegistrationComplete(const ICDClientInfo & clientInfo) = 0;
};

} // namespace app
} // namespace chip
