/*
 *json
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
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

OtaProviderAppCommandHandler * OtaProviderAppCommandHandler::FromJSON(const char *)
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

    return chip::Platform::New<OtaProviderAppCommandHandler>(std::move(value));
}

std::string OtaProviderAppCommandHandler::GetQueryImageResponse()
{
    return "Test";
}

void OtaProviderAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self      = reinterpret_cast<OtaProviderAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "GetQueryImageResponse")
    {
        std::string queryImage = self->GetQueryImageResponse();
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happen");
    }

exit:
    chip::Platform::Delete(self);
}

void OtaProviderAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = OtaProviderAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "Camera App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(OtaProviderAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
}
