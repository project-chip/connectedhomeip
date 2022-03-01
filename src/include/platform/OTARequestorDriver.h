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
    CharSpan imageURI;
    uint32_t softwareVersion;
    CharSpan softwareVersionStr;
    ByteSpan updateToken;
    bool userConsentNeeded;
    ByteSpan metadataForRequestor;
};

enum class UpdateFailureState
{
    kQuerying,
    kDownloading,
    kApplying,
    kNotifying,
    kAwaitingNextAction,
};

enum class UpdateNotFoundReason
{
    Busy,
    NotAvailable,
    UpToDate
};

// Interface class to abstract the OTA-related business logic. Each application
// must implement this interface. All calls must be non-blocking unless stated otherwise
class OTARequestorDriver
{
public:
    virtual ~OTARequestorDriver() = default;

    /// Return if the device provides UI for asking a user for consent before downloading a software image
    virtual bool CanConsent() = 0;

    /// Return maximum supported download block size
    virtual uint16_t GetMaxDownloadBlockSize() { return 1024; }

    /// Called when an error occurs at any OTA requestor operation
    virtual void HandleError(UpdateFailureState state, CHIP_ERROR error) = 0;

    /// Called when the latest query found a software update
    virtual void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) = 0;

    /// Called when the latest query did not find any software update
    virtual void UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay) = 0;

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
};

} // namespace chip
