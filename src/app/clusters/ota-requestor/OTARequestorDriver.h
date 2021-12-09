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

#include <protocols/bdx/BdxMessages.h>

namespace chip {

/* Commented out until the API is supported
// The set of parameters needed for starting a BDX download.
struct BdxDownloadParameters
{
    uint32_t delayedActionTime;           // Might not be needed
    chip::CharSpan imageURI;
    uint32_t softwareVersion;
    chip::CharSpan softwareVersionString; // Might not be needed
    chip::ByteSpan updateToken;
    bool userConsentNeeded;               // Might not be needed
    chip::ByteSpan metadataForRequestor;  // Might not be needed
};
*/

// Possible values for the UpdateState attribute
enum UpdateStateEnum
{
    Unknown              = 0,
    Idle                 = 1,
    Querying             = 2,
    DelayedOnQuery       = 3,
    Downloading          = 4,
    Applying             = 5,
    DelayedOnApply       = 6,
    RollingBack          = 7,
    DelayedOnUserConsent = 8,
};

// Return type for RequestUserConsent()
enum UserConsentAction
{
    ImmediateYes = 1,
    ImmediateNo  = 2,
    Requested    = 3,
};

// Interface class to abstract the OTA-related business logic. Each application
// must implement this interface. All calls must be non-blocking unless stated otherwise
class OTARequestorDriver
{
public:
    // Mandatory methods, applications are required to implement these

    // A call into the application logic to give it a chance to allow or stop the Requestor
    // from proceeding with actual image download. Returning TRUE will allow the download
    // to proceed, returning FALSE will abort the download process.
    virtual bool CheckImageDownloadAllowed() = 0;

    // Application is directed to complete user consent: either return ImmediateYes/ImmediateNo
    // without blocking or return Requested and call OTARequestor::OnUserConsent() later.
    virtual UserConsentAction RequestUserConsent() = 0;

    // Notify the application that the download is complete and the image can be applied
    virtual void ImageDownloadComplete() = 0;

    // Optional methods, applications may choose to implement these

    /* Commented out until the API is supported
    // This method informs the application of the BDX download parameters. This info can be used
    // later on for diecting the Requestor to resume an interrupted download
    virtual void PostBdxDownloadParameters(const BdxDownloadParameters & bdxParameters){};
    */

    // Return maximum supported download block size
    virtual uint16_t GetMaxDownloadBlockSize() { return 1024; }

    // Get Version of the last downloaded image, return CHIP_ERROR_NOT_FOUND if none exists
    virtual CHIP_ERROR GetLastDownloadedImageVersion(uint32_t & out_version) { return CHIP_ERROR_NOT_FOUND; }

    // Notify application of a change in the UpdateState attribute
    virtual void NotifyUpdateStateChange(chip::UpdateStateEnum state){};

    // Destructor
    virtual ~OTARequestorDriver() = default;
};

} // namespace chip
