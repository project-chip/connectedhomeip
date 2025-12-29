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

#include "OtaProviderAppCommandDelegate.h"
#include <lib/support/Defer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

OtaProviderAppCommandHandler * OtaProviderAppCommandHandler::FromJSON(const char * json, OtaProviderAppCommandDelegate * delegate)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified,
                     "OTA Provider Example: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "OTA Provider Example: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "OTA Provider Example: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<OtaProviderAppCommandHandler>(std::move(value), delegate);
}

static std::string ToString(const Json::Value & v)
{
    Json::StreamWriterBuilder w;
    w["indentation"] = "";
    return Json::writeString(w, v);
}

Json::Value OtaProviderAppCommandHandler::BuildOtaProviderSnapshot(uint16_t endpoint)
{
    Json::Value payload(Json::objectValue);

    payload["VendorID"]            = gOtaProvider.GetVendorId();
    payload["ProductID"]           = gOtaProvider.GetProductId();
    payload["SoftwareVersion"]     = gOtaProvider.GetSoftwareVersion();
    payload["HardwareVersion"]     = gOtaProvider.GetHardwareVersion();
    payload["Location"]            = gOtaProvider.GetLocation();
    payload["RequestorCanConsent"] = gOtaProvider.GetRequestorCanConsent();

    const auto & protos = gOtaProvider.GetProtocolsSupported();

    Json::Value arr(Json::arrayValue);
    for (size_t i = 0; i < gOtaProvider.kProtocolsSupportedCount; ++i)
    {
        arr.append(Json::UInt(protos[i]));
    }
    payload["ProtocolsSupported"] = arr;

    return payload;
}

void OtaProviderAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self = reinterpret_cast<OtaProviderAppCommandHandler *>(context);

    if (self == nullptr)
    {
        return;
    }

    auto cleanup = MakeDefer([&]() { Platform::Delete(self); });

    std::string name;
    std::string cluster;
    uint16_t endpoint                        = 0;
    OtaProviderAppCommandDelegate * delegate = nullptr;

    if (self->mCommandPayload.empty())
    {
        ChipLogError(NotSpecified, "Invalid JSON event command received");
        return;
    }

    name     = self->mCommandPayload["Name"].asString();
    cluster  = self->mCommandPayload.get("Cluster", "").asString();
    endpoint = static_cast<uint16_t>(self->mCommandPayload.get("Endpoint", 0).asUInt());
    delegate = self->mDelegate;

    if (name == "QueryImageSnapshot")
    {
        Json::Value out(Json::objectValue);
        out["Name"]     = "SnapshotResponse";
        out["Cluster"]  = cluster;
        out["Endpoint"] = endpoint;

        if (cluster == "OtaSoftwareUpdateProvider")
        {
            out["Payload"] = self->BuildOtaProviderSnapshot(endpoint);
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

    ChipLogError(NotSpecified, "Unhandled command: Should never happen")
}

void OtaProviderAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = OtaProviderAppCommandHandler::FromJSON(json, this);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "OTA Provider App: Unable to instantiate a command handler");
        return;
    }

    // Defer handling to the platform work queue to ensure this command is executed asynchronously and not from the command parsing
    // context
    SuccessOrDie(PlatformMgr().ScheduleWork(OtaProviderAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler)));
}
