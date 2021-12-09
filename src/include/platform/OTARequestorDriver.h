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

// Interface class to abstract the OTA-related business logic. Each application
// must implement this interface. All calls must be non-blocking unless stated otherwise
class OTARequestorDriver
{
public:
    // A call into the application logic to give it a chance to allow or stop the Requestor
    // from proceeding with actual image download. Returning TRUE will allow the download
    // to proceed, returning FALSE will abort the download process.
    virtual bool CheckImageDownloadAllowed() = 0;

    // Notify the application that the download is complete and the image can be applied
    virtual void ImageDownloadComplete() = 0;

    // Destructor
    virtual ~OTARequestorDriver() = default;
};
