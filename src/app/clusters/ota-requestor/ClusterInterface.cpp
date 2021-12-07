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

/* This file contains the glue code for passing the incoming OTA Requestor cluster commands
 * to the OTA Requestor object that handles them
 */

#include "OTARequestorInterface.h"

// OTA Software Update Requestor Cluster AnnounceOtaProvider Command callback
bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    EmberAfStatus status;
    OTARequestorInterface * requestor = GetRequestorInstance();

    if (requestor != nullptr)
    {
        status = requestor->HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// TBD for now. Not clear what is the API for registering a command response handler
/* Callbacks for QueryImage response */
// Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(QueryImageResponseHandler,
// nullptr);

// void QueryImageResponseHandler(void * context, const QueryImageResponse::DecodableType & response)
//{ }
