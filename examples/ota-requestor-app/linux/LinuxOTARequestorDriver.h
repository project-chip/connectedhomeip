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

/* This file contains the decalarions for the Linux implementation of the
 * the OTARequestorDriver interface class
 */
#include <platform/OTARequestorDriver.h>

namespace chip {

class LinuxOTARequestorDriver : public OTARequestorDriver
{

    // Virtual functions from OTARequestorDriver -- start

    // A call into the application logic to give it a chance to allow or stop the Requestor
    // from proceeding with actual image download. Returning TRUE will allow the download
    // to proceed, returning FALSE will abort the download process.
    bool CheckImageDownloadAllowed();

    // Notify the application that the download is complete and the image can be applied
    void ImageDownloadComplete();

    // Application is directed to complete user consent: either return ImmediateYes/ImmediateNo
    // without blocking or return Requested and call OTARequestor::OnUserConsent() later.
    virtual UserConsentAction RequestUserConsent();

    // Virtual functions from OTARequestorDriver -- end
};

} // namespace chip
