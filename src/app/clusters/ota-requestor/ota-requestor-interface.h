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
 * Any implementation of the OTA Requestor (e.g. the OTARequestor class) must implement
 * this interface.
 */

#pragma once

// Interface class to connect the OTA Software Update Requestor cluster command processing
// with the core OTA Requestor logic. The OTARequestor class implements this interface
class OTARequestorInterface
{
public:
    // Handler for the AnnounceOTAProvider command
    virtual bool HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData ch) = 0;

    // Handler for the QueryImageResponse command
    virtual bool HandleQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, CharSpan imageURI,
                                          uint32_t softwareVersion, CharSpan softwareVersionString, ByteSpan updateToken,
                                          bool userConsentNeeded, ByteSpan metadataForRequester) = 0;

    // Handler for the ApplyUpdateResponse command
    virtual bool HandleApplyUpdateResponse(ApplyUpdateResponse::DecodableType);
};
