/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ICDClientInfo.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/CommonIterator.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <stddef.h>

namespace chip {
namespace app {

/**
 * The ICDClientStorage class is an abstract interface that defines the operations
 * for storing, retrieving and deleting ICD client information in persistent storage.
 */
class ICDClientStorage
{
public:
    virtual ~ICDClientStorage() = default;

    /**
     * Called during ICD device registration in commissioning, commissioner/controller
     * provides raw key data, the shared aes key handle and hmac key handle in clientInfo are updated based upon raw key data
     *
     * @param[inout] clientInfo the ICD Client information to be updated with keyData and be saved
     * @param[in] aKeyData raw key data provided by application
     */
    virtual CHIP_ERROR SetKey(ICDClientInfo & clientInfo, const ByteSpan keyData) = 0;

    /**
     * Store updated ICD ClientInfo to storage when ICD registration completes or check-in message
     * comes.
     *
     * @param[in] clientInfo the updated ICD Client Info.
     */
    virtual CHIP_ERROR StoreEntry(const ICDClientInfo & clientInfo) = 0;

    /**
     * This function removes the ICD key from the provided clientInfo object in the event
     *  of a failed LIT ICD device registration attempt. If the key handle is not found within
     *  the Keystore, the function will not perform any operation.
     * @param[inout] clientInfo The ICD Client Info to update with uninitialized key handle if key is removed successfully.
     */
    virtual void RemoveKey(ICDClientInfo & clientInfo) = 0;

    /**
     * Delete ICD Client persistent information associated with the specified scoped node Id.
     * when ICD device is unpaired/removed, the corresponding entry in ICD storage is removed.
     * @param peerNode scoped node with peer node id and fabric index
     */
    virtual CHIP_ERROR DeleteEntry(const ScopedNodeId & peerNode) = 0;

    /**
     * Process received ICD check-in message payload.  The implementation needs to parse the payload,
     * look for a key that allows successfully decrypting the payload, verify that the counter in the payload is valid,
     * and populate the clientInfo with the stored information corresponding to the key.
     * @param[in] payload received check-in Message payload
     * @param[out] clientInfo retrieved matched clientInfo from storage
     * @param[out] counter counter value received in the check-in message
     */
    virtual CHIP_ERROR ProcessCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo,
                                             Protocols::SecureChannel::CounterType & counter) = 0;

    // 4 bytes for counter + 2 bytes for ActiveModeThreshold
    static inline constexpr uint8_t kAppDataLength = 6;
};
} // namespace app
} // namespace chip
