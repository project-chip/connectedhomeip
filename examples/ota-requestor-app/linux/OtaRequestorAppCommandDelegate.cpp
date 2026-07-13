/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "OtaRequestorAppCommandDelegate.h"
#include "OTAConsentHandler.h"
#include <app/clusters/ota-requestor/OTARequestorUserConsentDelegate.h>
#include <lib/support/Defer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

OtaRequestorAppCommandHandler * OtaRequestorAppCommandHandler::FromJSON(const char * json,
                                                                        OtaRequestorAppCommandDelegate * delegate)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified,
                     "OTA Requestor Example: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "OTA Requestor Example: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "OTA Requestor Example: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<OtaRequestorAppCommandHandler>(OtaRequestorAppCommandHandler::PrivateCtorTag{}, std::move(value),
                                                        delegate);
}

static std::string ToString(const Json::Value & v)
{
    Json::StreamWriterBuilder w;
    w["indentation"] = "";
    return Json::writeString(w, v);
}

Json::Value OtaRequestorAppCommandHandler::BuildCanConsent(uint16_t endpoint)
{
    Json::Value payload(Json::objectValue);
    bool canConsent                = gConsentHandler.GetRequestorConsent().ValueOr(false);
    payload["RequestorCanConsent"] = canConsent;

    return payload;
}

Json::Value OtaRequestorAppCommandHandler::BuildUserConsentState(uint16_t endpoint, std::string consentState)
{
    Json::Value payload(Json::objectValue);
    chip::ota::UserConsentState ktmpConsentState;
    
    if( consentState == "granted" ){
        ktmpConsentState = chip::ota::UserConsentState::kGranted;
    }else if ( consentState == "denied" ){
        ktmpConsentState = chip::ota::UserConsentState::kDenied;
    }else if ( consentState == "deferred" ){
        ktmpConsentState = chip::ota::UserConsentState::kObtaining;
    }else{
        ChipLogError(SoftwareUpdate, "Invalid  option for ConsentState: %s", consentState.c_str());
        ktmpConsentState = chip::ota::UserConsentState::kUnknown;
    }

    gConsentHandler.SetUserConsentState(ktmpConsentState);
    chip::ota::UserConsentState state = gConsentHandler.GetUserConsentState();
    payload["UserConsentState"]       = chip::ota::OTARequestorUserConsentDelegate::UserConsentStateToString(state);

    return payload;
}

void OtaRequestorAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self = reinterpret_cast<OtaRequestorAppCommandHandler *>(context);

    if (self == nullptr)
    {
        return;
    }

    auto cleanup = MakeDefer([&]() { Platform::Delete(self); });

    std::string name;
    std::string consentState;
    std::string cluster;
    uint16_t endpoint                         = 0;
    OtaRequestorAppCommandDelegate * delegate = nullptr;

    if (self->mCommandPayload.empty())
    {
        ChipLogError(NotSpecified, "Invalid JSON event command received");
        return;
    }

    name     = self->mCommandPayload["Name"].asString();
    cluster  = self->mCommandPayload.get("Cluster", "").asString();
    endpoint = static_cast<uint16_t>(self->mCommandPayload.get("Endpoint", 0).asUInt());
    delegate = self->mDelegate;

    if (name == "QueryRequestorCanConsent")
    {
        Json::Value out(Json::objectValue);
        out["Name"]     = "ConsentResponse";
        out["Cluster"]  = cluster;
        out["Endpoint"] = endpoint;

        if (cluster == "OtaSoftwareUpdateRequestor")
        {
            out["Payload"] = self->BuildCanConsent(endpoint);
        }
        else
        {
            out["Error"] = "Unsupported cluster for snapshot";
        }

        if (delegate && delegate->GetPipes())
        {
            delegate->GetPipes()->WriteToOutPipe(ToString(out));
        }
        return;
    }

    if (name == "SetUserConsentState")
    {  
        consentState = self->mCommandPayload["consentState"].asString();
        Json::Value out(Json::objectValue);
        out["Name"]     = "UserConsentStateResponse";
        out["Cluster"]  = cluster;
        out["Endpoint"] = endpoint;

        if (cluster == "OtaSoftwareUpdateRequestor")
        {

            out["Payload"] = self->BuildUserConsentState(endpoint, consentState);
        }
        else
        {
            out["Error"] = "Unsupported cluster for snapshot";
        }

        if (delegate && delegate->GetPipes())
        {
            delegate->GetPipes()->WriteToOutPipe(ToString(out));
        }
        return;
    }

    ChipLogError(NotSpecified, "Unhandled command: Should never happen");
}

void OtaRequestorAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = OtaRequestorAppCommandHandler::FromJSON(json, this);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "OTA Requestor App: Unable to instantiate a command handler");
        return;
    }

    // Defer handling to the platform work queue to ensure this command is executed asynchronously and not from the command parsing
    // context
    SuccessOrDie(PlatformMgr().ScheduleWork(OtaRequestorAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler)));
}
