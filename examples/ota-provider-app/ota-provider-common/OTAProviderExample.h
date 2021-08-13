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

#pragma once

#include <app/Command.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>

using chip::app::clusters::OTAProviderDelegate;

/**
 * A reference implementation for an OTA Provider. Includes a method for providing a path to a local OTA file to serve.
 */
class OTAProviderExample : public OTAProviderDelegate
{
public:
    OTAProviderExample();

    void SetOTAFilePath(const char * path);

    // Inherited from OTAProviderDelegate
    EmberAfStatus HandleQueryImage(chip::app::CommandHandler * commandObj, uint16_t vendorId, uint16_t productId,
                                   uint16_t imageType, uint16_t hardwareVersion, uint32_t currentVersion,
                                   uint8_t protocolsSupported, const chip::ByteSpan & location, bool clientCanConsent,
                                   const chip::ByteSpan & metadataForServer) override;
    EmberAfStatus HandleApplyUpdateRequest(chip::app::CommandHandler * commandObj, const chip::ByteSpan & updateToken,
                                           uint32_t newVersion) override;
    EmberAfStatus HandleNotifyUpdateApplied(const chip::ByteSpan & updateToken, uint32_t currentVersion) override;

private:
    static constexpr size_t kFilepathBufLen = 256;
    char mOTAFilePath[kFilepathBufLen]; // null-terminated
};
