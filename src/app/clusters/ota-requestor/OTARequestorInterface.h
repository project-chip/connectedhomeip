/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* This file contains the declaration for the OTA Requestor interface.
 * Any implementation of the OTA Requestor must implement this interface.
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <lib/core/ClusterEnums.h>

#pragma once

namespace chip {

namespace app {
class CommandHandler;
struct ConcreteCommandPath;
} // namespace app

/**
 * A class to represent a list of the provider locations
 */
class ProviderLocationList
{
public:
    /**
     * A class to iterate over an instance of ProviderLocationList
     */
    class Iterator
    {
    public:
        /**
         * Initialize iterator values, must be called before calling Next()/GetValue()
         */
        Iterator(const Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> * const list,
                 size_t size)
        {
            mList         = list;
            mListSize     = size;
            mCurrentIndex = 0;
            mNextIndex    = 0;
        }

        /**
         * Search for the next provider location found in the list
         */
        bool Next()
        {
            for (size_t i = mNextIndex; i < mListSize; i++)
            {
                if (mList[i].HasValue())
                {
                    mCurrentIndex = i;
                    mNextIndex    = mCurrentIndex + 1;
                    return true;
                }
            }

            return false;
        }

        /**
         * Retrieves a reference to the provider location found on a previous call to Next()
         */
        const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & GetValue() const
        {
            return mList[mCurrentIndex].Value();
        }

    private:
        const Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> * mList;
        size_t mListSize;
        size_t mCurrentIndex;
        size_t mNextIndex;
    };

    /**
     * Retrieve an iterator to the list
     */
    Iterator Begin() const { return Iterator(mList, CHIP_CONFIG_MAX_FABRICS); }

    /**
     * Add a provider location to the list if there is space available
     */
    CHIP_ERROR Add(const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & providerLocation)
    {
        for (size_t i = 0; i < mMaxSize; i++)
        {
            if (!mList[i].HasValue())
            {
                mList[i].SetValue(providerLocation);
                mListSize++;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_NO_MEMORY;
    }

    /**
     * Delete a provider location for the given fabric index.
     */
    CHIP_ERROR Delete(FabricIndex fabricIndex)
    {
        for (size_t i = 0; i < mMaxSize; i++)
        {
            if (mList[i].HasValue() && mList[i].Value().GetFabricIndex() == fabricIndex)
            {
                mList[i].ClearValue();
                mListSize--;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_NOT_FOUND;
    }

private:
    Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> mList[CHIP_CONFIG_MAX_FABRICS];
    size_t mListSize = 0;
    size_t mMaxSize  = CHIP_CONFIG_MAX_FABRICS;
};

// Interface class to connect the OTA Software Update Requestor cluster command processing
// with the core OTA Requestor logic
class OTARequestorInterface
{
public:
    using OTAUpdateStateEnum   = chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum;
    using ProviderLocationType = app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type;

    // Reset any relevant states
    virtual void Reset(void) = 0;

    /**
     * Called to handle an AnnounceOTAProvider command and is responsible for sending the status. The caller is responsible for
     * validating fields in the command.
     */
    virtual void HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) = 0;

    // Destructor
    virtual ~OTARequestorInterface() = default;

    // Application API to send the QueryImage command and start the image update process.
    // The `fabricIndex` optional argument can be used to explicitly select the OTA provider.
    virtual CHIP_ERROR TriggerImmediateQuery(FabricIndex fabricIndex = kUndefinedFabricIndex) = 0;

    // Internal API meant for use by OTARequestorDriver to send the QueryImage command and start the image update process
    // with the preset provider
    virtual void TriggerImmediateQueryInternal() = 0;

    // Download image
    virtual void DownloadUpdate() = 0;

    // Image download delayed on user consent
    virtual void DownloadUpdateDelayedOnUserConsent() = 0;

    // Initiate the session to send ApplyUpdateRequest command
    virtual void ApplyUpdate() = 0;

    // Initiate the session to send NotifyUpdateApplied command
    virtual void NotifyUpdateApplied() = 0;

    // Get the value of the UpdateStateProgress attribute (in percentage) of the OTA Software Update Requestor Cluster on the given
    // endpoint
    virtual CHIP_ERROR GetUpdateStateProgressAttribute(EndpointId endpointId,
                                                       chip::app::DataModel::Nullable<uint8_t> & progress) = 0;

    // Get the value of the UpdateState attribute of the OTA Software Update Requestor Cluster on the given endpoint
    virtual CHIP_ERROR GetUpdateStateAttribute(EndpointId endpointId, OTAUpdateStateEnum & state) = 0;

    // Get the current state of the OTA update
    virtual OTAUpdateStateEnum GetCurrentUpdateState() = 0;

    // Get the target version of the OTA update
    virtual uint32_t GetTargetVersion() = 0;

    // Application directs the Requestor to cancel image update in progress. All the Requestor state is
    // cleared, UpdateState is reset to Idle
    virtual void CancelImageUpdate() = 0;

    // Clear all entries with the specified fabric index in the default OTA provider list
    virtual CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) = 0;

    // Set the provider location to be used in the next query and OTA update process
    virtual void SetCurrentProviderLocation(ProviderLocationType providerLocation) = 0;

    // Set the metadata value for the provider to be used in the next query and OTA update process
    virtual void SetMetadataForProvider(chip::ByteSpan metadataForProvider) = 0;

    // If there is an OTA update in progress, returns the provider location for the current OTA update, otherwise, returns the
    // provider location that was last used
    virtual void GetProviderLocation(Optional<ProviderLocationType> & providerLocation) = 0;

    // Add a default OTA provider to the cached list
    virtual CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation) = 0;

    // Retrieve an iterator to the cached default OTA provider list
    virtual ProviderLocationList::Iterator GetDefaultOTAProviderListIterator(void) = 0;
};

// The instance of the class implementing OTARequestorInterface must be managed through
// the following global getter and setter functions.

// Set the object implementing OTARequestorInterface
void SetRequestorInstance(OTARequestorInterface * instance);

// Get the object implementing OTARequestorInterface
OTARequestorInterface * GetRequestorInstance();

} // namespace chip
