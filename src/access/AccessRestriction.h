/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "RequestPath.h"
#include "SubjectDescriptor.h"
#include <algorithm>
#include <cstdint>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>
#include <map>
#include <memory>
#include <protocols/interaction_model/Constants.h>
#include <vector>

namespace chip {
namespace Access {

using namespace chip::Protocols::InteractionModel;
using namespace chip::Platform;

class AccessRestriction
{
public:
    static constexpr size_t kNumberOfFabrics      = CHIP_CONFIG_MAX_FABRICS;
    static constexpr size_t kEntriesPerFabric     = CHIP_CONFIG_ACCESS_RESTRICTION_MAX_ENTRIES_PER_FABRIC;
    static constexpr size_t kRestrictionsPerEntry = CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY;

    /**
     * Defines the type of access restriction, which is used to determine the meaning of the restriction's id.
     */
    enum class Type : uint8_t
    {
        kAttributeAccessForbidden = 0,
        kAttributeWriteForbidden  = 1,
        kCommandForbidden         = 2,
        kEventForbidden           = 3
    };

    /**
     * Defines a single restriction on an attribute, command, or event.
     *
     * If id is not set, the restriction applies to all attributes, commands, or events of the given type (wildcard).
     */
    struct Restriction
    {
        Type restrictionType;
        Optional<uint32_t> id;
    };

    /**
     * Defines a single entry in the access restriction list, which contains a list of restrictions
     * for a cluster on an endpoint.
     */
    struct Entry
    {
        FabricIndex fabricIndex;
        EndpointId endpointNumber;
        ClusterId clusterId;
        std::vector<Restriction> restrictions;
    };

    /**
     * Used to notify of changes in the access restriction list and active reviews.
     */
    class EntryListener
    {
    public:
        enum class ChangeType
        {
            kAdded   = 1,
            kRemoved = 2,
            kUpdated = 3
        };

        virtual ~EntryListener() = default;

        /**
         * Notifies of a change in the access restriction list.
         *
         * @param [in] fabricIndex  The index of the fabric in which the entry has changed.
         * @param [in] index        Index of entry to which has changed (relative to fabric).
         * @param [in] entry        The latest value of the entry which changed.
         * @param [in] changeType   The type of change that occurred.
         */
        virtual void OnEntryChanged(FabricIndex fabricIndex, size_t index, SharedPtr<Entry> entry, ChangeType changeType) = 0;

        /**
         * Notifies of an update to an active review with instructions and an optional redirect URL.
         *
         * @param [in] fabricIndex  The index of the fabric in which the entry has changed.
         * @param [in] token        The token of the review being updated (obtained from ReviewFabricRestrictionsResponse)
         * @param [in] instruction  The instructions to be displayed to the user.
         * @param [in] redirectUrl  An optional URL to redirect the user to for more information.  May be null.
         */
        virtual void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, const char * instruction,
                                                     const char * redirectUrl) = 0;

    private:
        EntryListener * mNext = nullptr;

        friend class AccessRestriction;
    };

    AccessRestriction()          = default;
    virtual ~AccessRestriction() = default;

    AccessRestriction(const AccessRestriction &) = delete;
    AccessRestriction & operator=(const AccessRestriction &) = delete;

    /**
     * Add a listener to be notified of changes in the access restriction list and active reviews.
     *
     * @param [in] listener  The listener to add.
     */
    void AddListener(EntryListener & listener);

    /**
     * Remove a listener from being notified of changes in the access restriction list and active reviews.
     *
     * @param [in] listener  The listener to remove.
     */
    void RemoveListener(EntryListener & listener);

    /**
     * Check whether access by a subject descriptor to a request path should be restricted (denied) for the given action.
     * These restrictions are are only a part of overall access evaluation.
     *
     * If access is not restricted, CHIP_NO_ERROR will be returned.
     *
     * @retval #CHIP_ERROR_ACCESS_DENIED if access is denied.
     * @retval other errors should also be treated as restricted/denied.
     * @retval #CHIP_NO_ERROR if access is not restricted/denied.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, MsgType action);

    /**
     * Request a review of the access restrictions for a fabric.
     *
     * @param [in]  fabricIndex  The index of the fabric requesting a review.
     * @param [in]  arl          An optinal list of access restriction entries to review.  If null, all entries will be reviewed.
     * @param [out] token        The unique token for the review, which can be matched to a review update event.
     */
    CHIP_ERROR RequestFabricRestrictionReview(FabricIndex fabricIndex, const std::vector<Entry> * arl, uint64_t & token)
    {
        token = ++mNextToken;
        return DoRequestFabricRestrictionReview(fabricIndex, token, arl);
    }

    using EntryIterator = std::vector<SharedPtr<Entry>>::iterator;

    /**
     * Get iterator over entries in the access restriction list by fabric.
     *
     * @param [in]  fabricIndex Iteration is confined to fabric
     * @param [out] begin iterator pointing to the beginning of the entries
     * @param [out] end iterator pointing to the end of the entries
     */
    CHIP_ERROR Entries(const FabricIndex fabricIndex, EntryIterator & begin, EntryIterator & end)
    {
        if (mFabricEntries.find(fabricIndex) == mFabricEntries.end())
        {
            return CHIP_ERROR_NOT_FOUND;
        }

        begin = mFabricEntries[fabricIndex].begin();
        end   = mFabricEntries[fabricIndex].end();

        return CHIP_NO_ERROR;
    }

    /**
     * Add a restriction entry to the access restriction list and notify any listeners. This request will fail if there is already
     * an entry with the same cluster and endpoint.
     *
     * @param [out] index       (If not nullptr) index of created entry (relative to fabric).
     * @param [in]  entry       The entry to add to the access restriction list.
     * @param [in]  fabricIndex The index of the fabric in which the entry should be added.
     * @return                  CHIP_NO_ERROR if the entry was successfully created, or an error code.
     */
    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, const FabricIndex fabricIndex);

    /**
     * Delete a restriction entry from the access restriction list and notify any listeners.
     *
     * @param [in] index           Index of entry to delete.  May be relative to fabric.
     * @param [in] fabricIndex     Fabric to which entry `index` is relative.
     * @return                     CHIP_NO_ERROR if the entry was successfully deleted, or an error code.
     */
    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex fabricIndex);

    /**
     * Update a restriction entry in the access restriction list and notify any listeners.
     *
     * @param [in] index           Index of entry to delete.  May be relative to fabric.
     * @param [in] entry        The updated entry to replace the existing entry.
     * @param [in] fabricIndex  The index of the fabric in which the entry should be updated.
     * @return                  CHIP_NO_ERROR if the entry was successfully updated, or an error code.
     */
    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex fabricIndex);

protected:
    /**
     * Initiate a review of the access restrictions for a fabric. This method should be implemented by the platform and be
     * non-blocking.
     *
     * @param [in] fabricIndex  The index of the fabric requesting a review.
     * @param [in] token        The unique token for the review, which can be matched to a review update event.
     * @param [in] arl          An optinal list of access restriction entries to review.  If null, all entries will be reviewed.
     * @return                  CHIP_NO_ERROR if the review was successfully requested, or an error code if the request failed.
     */
    virtual CHIP_ERROR DoRequestFabricRestrictionReview(const FabricIndex fabricIndex, uint64_t token,
                                                        const std::vector<Entry> * arl) = 0;

private:
    SharedPtr<Entry> GetEntry(FabricIndex fabricIndex, size_t index);

    uint64_t mNextToken        = 1;
    EntryListener * mListeners = nullptr;
    std::map<FabricIndex, std::vector<SharedPtr<Entry>>> mFabricEntries;
};

} // namespace Access
} // namespace chip
