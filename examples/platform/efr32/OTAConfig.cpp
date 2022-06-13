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

    /// Pointer to information about the certificate
    .cert = NULL,

    /// Pointer to Long Token Data Section
    .longTokenSectionAddress = NULL,
};

// Global OTA objects
chip::DefaultOTARequestor gRequestorCore;
chip::DefaultOTARequestorStorage gRequestorStorage;
chip::DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
chip::BDXDownloader gDownloader;
chip::OTAImageProcessorImpl gImageProcessor;

void OTAConfig::Init()
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    // Periodic query timeout must be set prior to requestor being initialized
    gRequestorUser.SetPeriodicQueryTimeout(OTA_PERIODIC_TIMEOUT);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    gImageProcessor.SetOTAImageFile("test.txt");
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}
