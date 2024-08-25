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

#include "Privilege.h"
#include "RequestPath.h"
#include "SubjectDescriptor.h"
#include <algorithm>
#include <app-common/zap-generated/cluster-objects.h>
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

class AccessRestrictionProvider
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
     * Defines the interface for a checker for access restriction exceptions.
     */
    class AccessRestrictionExceptionChecker
    {
    public:
        virtual ~AccessRestrictionExceptionChecker() = default;

        /**
         * Check if any restrictions are allowed to be applied on the given endpoint and cluster
         * because of constraints against their use in ARLs.
         *
         * @retval true if ARL checks are allowed to be applied to the cluster on the endpoint, false otherwise
         */
        virtual bool AreRestrictionsAllowed(EndpointId endpoint, ClusterId cluster) = 0;
    };

    /**
     * Define a standard implementation of the AccessRestrictionExceptionChecker interface
     * which is the default implementation used by AccessResrictionProvider.
     */
    class StandardAccessRestrictionExceptionChecker : public AccessRestrictionExceptionChecker
    {
    public:
        StandardAccessRestrictionExceptionChecker()  = default;
        ~StandardAccessRestrictionExceptionChecker() = default;

        bool AreRestrictionsAllowed(EndpointId endpoint, ClusterId cluster) override;
    };

    /**
     * Used to notify of changes in the access restriction list and active reviews.
     */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /**
         * Notifies of a change in the commissioning access restriction list.
         */
        virtual void MarkCommissioningRestrictionListChanged() = 0;

        /**
         * Notifies of a change in the access restriction list.
         *
         * @param [in] fabricIndex  The index of the fabric in which the list has changed.
         */
        virtual void MarkRestrictionListChanged(FabricIndex fabricIndex) = 0;

        /**
         * Notifies of an update to an active review with instructions and an optional redirect URL.
         *
         * @param [in] fabricIndex  The index of the fabric in which the entry has changed.
         * @param [in] token        The token of the review being updated (obtained from ReviewFabricRestrictionsResponse)
         * @param [in] instruction  Optional instructions to be displayed to the user.
         * @param [in] redirectUrl  An optional URL to redirect the user to for more information.
         */
        virtual void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, Optional<CharSpan> instruction,
                                                     Optional<CharSpan> redirectUrl) = 0;

    private:
        Listener * mNext = nullptr;

        friend class AccessRestrictionProvider;
    };

    AccessRestrictionProvider()          = default;
    virtual ~AccessRestrictionProvider() = default;

    AccessRestrictionProvider(const AccessRestrictionProvider &)             = delete;
    AccessRestrictionProvider & operator=(const AccessRestrictionProvider &) = delete;

    /**
     * Set the restriction entries that are to be used during commissioning when there is no accessing fabric.
     *
     * @param [in] entries  The entries to set.
     */
    CHIP_ERROR SetCommissioningEntries(const std::vector<Entry> & entries);

    /**
     * Set the restriction entries for a fabric.
     *
     * @param [in] fabricIndex  The index of the fabric for which to create entries.
     * @param [in] entries  The entries to set for the fabric.
     */
    CHIP_ERROR SetEntries(const FabricIndex, const std::vector<Entry> & entries);

    /**
     * Add a listener to be notified of changes in the access restriction list and active reviews.
     *
     * @param [in] listener  The listener to add.
     */
    void AddListener(Listener & listener);

    /**
     * Remove a listener from being notified of changes in the access restriction list and active reviews.
     *
     * @param [in] listener  The listener to remove.
     */
    void RemoveListener(Listener & listener);

    /**
     * Check whether access by a subject descriptor to a request path should be restricted (denied) for the given action
     * during commissioning by using the CommissioningEntries.
     *
     * These restrictions are are only a part of overall access evaluation.
     *
     * If access is not restricted, CHIP_NO_ERROR will be returned.
     *
     * @retval CHIP_ERROR_ACCESS_DENIED if access is denied.
     * @retval other errors should also be treated as restricted/denied.
     * @retval CHIP_NO_ERROR if access is not restricted/denied.
     */
    CHIP_ERROR CheckForCommissioning(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath);

    /**
     * Check whether access by a subject descriptor to a request path should be restricted (denied) for the given action.
     * These restrictions are are only a part of overall access evaluation.
     *
     * If access is not restricted, CHIP_NO_ERROR will be returned.
     *
     * @retval CHIP_ERROR_ACCESS_DENIED if access is denied.
     * @retval other errors should also be treated as restricted/denied.
     * @retval CHIP_NO_ERROR if access is not restricted/denied.
     */
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath);

    /**
     * Request a review of the access restrictions for a fabric.
     *
     * @param [in]  fabricIndex  The index of the fabric requesting a review.
     * @param [in]  arl          An optinal list of access restriction entries to review.  If null, all entries will be reviewed.
     * @param [out] token        The unique token for the review, which can be matched to a review update event.
     */
    CHIP_ERROR RequestFabricRestrictionReview(FabricIndex fabricIndex, const std::vector<Entry> & arl, uint64_t & token)
    {
        token = mNextToken++;
        return DoRequestFabricRestrictionReview(fabricIndex, token, arl);
    }

    /**
     * Get the commissioning restriction entries.
     *
     * @retval the commissioning restriction entries.
     */
    const std::vector<Entry> & GetCommissioningEntries() const { return mCommissioningEntries; }

    /**
     * Get the restriction entries for a fabric.
     *
     * @param [in]  fabricIndex the index of the fabric for which to get entries.
     * @param [out] entries vector to hold the entries.
     */
    CHIP_ERROR GetEntries(const FabricIndex fabricIndex, std::vector<Entry> & entries) const
    {
        auto it = mFabricEntries.find(fabricIndex);
        if (it == mFabricEntries.end())
        {
            return CHIP_ERROR_NOT_FOUND;
        }

        entries = (it->second);

        return CHIP_NO_ERROR;
    }

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
                                                        const std::vector<Entry> & arl) = 0;

private:
    /**
     * Perform the access restriction check using the given entries.
     */
    CHIP_ERROR DoCheck(const std::vector<Entry> & entries, const SubjectDescriptor & subjectDescriptor,
                       const RequestPath & requestPath);

    uint64_t mNextToken   = 1;
    Listener * mListeners = nullptr;
    StandardAccessRestrictionExceptionChecker mExceptionChecker;
    std::vector<Entry> mCommissioningEntries;
    std::map<FabricIndex, std::vector<Entry>> mFabricEntries;
};

} // namespace Access
} // namespace chip
