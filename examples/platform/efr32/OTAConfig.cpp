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

#include "OTAConfig.h"

#include "platform/bootloader/api/application_properties.h"
#include <app/server/Server.h>

// Header used for building the image GBL file
#define APP_PROPERTIES_VERSION 1
#define APP_PROPERTIES_ID                                                                                                          \
    {                                                                                                                              \
        0                                                                                                                          \
    }

__attribute__((used)) ApplicationProperties_t sl_app_properties = {
    /// @brief Magic value indicating that this is an ApplicationProperties_t
    /// Must equal @ref APPLICATION_PROPERTIES_MAGIC
    .magic = APPLICATION_PROPERTIES_MAGIC,

    /// Version number of this struct
    .structVersion = APPLICATION_PROPERTIES_VERSION,

    /// Type of signature this application is signed with
    .signatureType = APPLICATION_SIGNATURE_NONE,

    /// Location of the signature. Typically a pointer to the end of application
    .signatureLocation = 0,

    /// Information about the application
    .app = {

      /// Bitfield representing type of application
      /// e.g. @ref APPLICATION_TYPE_BLUETOOTH_APP
      .type = APPLICATION_TYPE_THREAD,

      /// Version number for this application
      .version = APP_PROPERTIES_VERSION,

      /// Capabilities of this application
      .capabilities = 0,

      /// Unique ID (e.g. UUID/GUID) for the product this application is built for
      .productId = APP_PROPERTIES_ID,
    },
};

// Global OTA objects
chip::OTARequestor gRequestorCore;
chip::DeviceLayer::GenericOTARequestorDriver gRequestorUser;
chip::BDXDownloader gDownloader;
chip::OTAImageProcessorImpl gImageProcessor;

void OTAConfig::Init()
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorCore.Init(&(chip::Server::GetInstance()), &gRequestorUser, &gDownloader);

    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    chip::OTAImageProcessorParams ipParams;
    ipParams.imageFile = chip::CharSpan("test.txt");
    gImageProcessor.SetOTAImageProcessorParams(ipParams);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}
