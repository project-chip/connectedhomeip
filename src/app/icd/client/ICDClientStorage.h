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
    using ICDClientInfoIterator = CommonIterator<ICDClientInfo>;

    virtual ~ICDClientStorage() = default;

    /**
     * Iterate through persisted ICD Client Info
     *
     * @return A valid iterator on success. Use CommonIterator accessor to retrieve ICDClientInfo
     */
    virtual ICDClientInfoIterator * IterateICDClientInfo() = 0;

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
     * Remove ICD key from clientInfo when ICD registration fails
     *
     * @param[inout] clientInfo the updated ICD Client Info.
     */
    virtual void RemoveKey(ICDClientInfo & clientInfo) = 0;

    /**
     * Get ICD ClientInfo from storage
     * One user case is to retrieve UserActiveModeTriggerHint and inform how user how to wake up sleepy device.
     * @param[in] peerNode scoped node with peer node id and fabric index
     * @param[out] clientInfo the ICD Client Info.
     */
    virtual CHIP_ERROR GetEntry(const ScopedNodeId & peerNode, ICDClientInfo & clientInfo) = 0;

    /**
     * Delete ICD Client persistent information associated with the specified scoped node Id.
     * when ICD device is unpaired/removed, the corresponding entry in ICD storage is removed.
     * @param peerNode scoped node with peer node id and fabric index
     */
    virtual CHIP_ERROR DeleteEntry(const ScopedNodeId & peerNode) = 0;

    /**
     * Remove all ICDClient persistent information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     * When the whole fabric is removed, all entries from persistent storage in current fabric index are removed.
     *
     * @param[in] fabricIndex the index of the fabric for which to remove ICDClient persistent information
     */
    virtual CHIP_ERROR DeleteAllEntries(FabricIndex fabricIndex) = 0;

    /**
     * Process received ICD Check-in message payload.  The implementation needs to parse the payload,
     * look for a key that allows successfully decrypting the payload, verify that the counter in the payload is valid,
     * and populate the clientInfo with the stored information corresponding to the key.
     * @param[in] payload received checkIn Message payload
     * @param[out] clientInfo retrieved matched clientInfo from storage
     */
    virtual CHIP_ERROR ProcessCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo) = 0;
};
} // namespace app
} // namespace chip
