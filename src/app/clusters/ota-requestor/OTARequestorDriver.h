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

/* This file contains the declaration for the OTARequestorDriver class,  an interface
 * that abstracts the OTA-related business logic out of the Requestor functionality in
 * the Matter SDK. Applications implementing the OTA Requestor functionality must include
 * this file.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/bdx/BdxMessages.h>
#include <system/SystemClock.h>

namespace chip {

// The set of parameters needed for starting a BDX download.
struct UpdateDescription
{
    NodeId nodeId;
    CharSpan fileDesignator;
    uint32_t softwareVersion;
    CharSpan softwareVersionStr;
    ByteSpan updateToken;
    bool userConsentNeeded;
    ByteSpan metadataForRequestor;
};

enum class UpdateNotFoundReason
{
    kBusy,
    kNotAvailable,
    kUpToDate,
};

// The reasons for why the OTA Requestor has entered idle state
enum class IdleStateReason
{
    kUnknown,
    kIdle,
    kInvalidSession,
};

// The current selected OTA Requestor timer to be running
enum class SelectedTimer
{
    kPeriodicQueryTimer,
    kWatchdogTimer,
};

// Interface class to abstract the OTA-related business logic. Each application
// must implement this interface. All calls must be non-blocking unless stated otherwise
class OTARequestorDriver
{
public:
    using ProviderLocationType = app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type;

    virtual ~OTARequestorDriver() = default;

    /// Return true if the device has the ability to ask the user for consent before downloading a software image
    virtual bool CanConsent() = 0;

    /// Return maximum supported download block size
    virtual uint16_t GetMaxDownloadBlockSize() { return 1024; }

    /// Set maximum supported download block size
    virtual void SetMaxDownloadBlockSize(uint16_t maxDownloadBlockSize) = 0;

    /// Called when OTA Requestor has exited the Idle state for which the driver may need to take various actions
    virtual void HandleIdleStateExit() = 0;

    // Called when the OTA Requestor has entered the Idle state for which the driver may need to take various actions
    virtual void HandleIdleStateEnter(IdleStateReason reason) = 0;

    /// Called when the latest query found a software update
    virtual void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) = 0;

    /// Called when the latest query did not find any software update
    virtual CHIP_ERROR UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay) = 0;

    /// Called when the download of a new software image has finished
    virtual void UpdateDownloaded() = 0;

    /// Called when the current software update can be applied
    virtual void UpdateConfirmed(System::Clock::Seconds32 delay) = 0;

    /// Called when the requestor shall ask again before applying the current software update
    virtual void UpdateSuspended(System::Clock::Seconds32 delay) = 0;

    /// Called when the current software update should be discontinued
    virtual void UpdateDiscontinued() = 0;

    /// Called when the current software update has been cancelled by the local application
    virtual void UpdateCancelled() = 0;

    /// Inform the driver that the device commissioning has completed
    virtual void OTACommissioningCallback() = 0;

    /// Driver portion of the logic for processing the AnnounceOTAProviders command
    virtual void
    ProcessAnnounceOTAProviders(const ProviderLocationType & providerLocation,
                                app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason announcementReason) = 0;

    /// Direct the driver to trigger the QueryImage command. The driver may choose to execute some internal
    /// logic and will then call an OTARequestorInterface API to actually send the command. The purpose of this
    /// function is to allow implementation-specific logic (such as possibly cancelling an ongoing update)
    /// to be executed before triggering the image update process
    virtual void SendQueryImage() = 0;

    // Driver picks the OTA Provider that should be used for the next query and update. The Provider is picked according to
    // the driver's internal logic such as, for example, traversing the default providers list.
    // Returns true if there is a Provider available for the next query, returns false otherwise.
    // @param[out] listExhausted - set to TRUE if the list of providers has been traversed until the end and has looped
    // back to the beginning.
    virtual bool GetNextProviderLocation(ProviderLocationType & providerLocation, bool & listExhausted) = 0;
};

} // namespace chip
