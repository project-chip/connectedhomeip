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

#include "CameraAppCommandDelegate.h"
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

CameraAppCommandHandler * CameraAppCommandHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified, "Camera App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "Camera App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "Camera App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return chip::Platform::New<CameraAppCommandHandler>(std::move(value));
}

void CameraAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self      = reinterpret_cast<CameraAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "ZoneTriggered")
    {
        std::vector<uint16_t> zoneIds;
        const Json::Value & zoneIdValue = self->mJsonValue["ZoneId"];

        // Check if ZoneId is an array or a single value
        if (zoneIdValue.isArray())
        {
            // Handle array of zone IDs
            for (const auto & zoneId : zoneIdValue)
            {
                zoneIds.push_back(static_cast<uint16_t>(zoneId.asUInt()));
            }
        }
        else
        {
            // Handle single zone ID
            zoneIds.push_back(static_cast<uint16_t>(zoneIdValue.asUInt()));
        }

        self->OnZoneTriggeredHandler(zoneIds);
    }
    else if (name == "SetHardPrivacyModeOn")
    {
        bool value = self->mJsonValue["Value"].asBool();
        self->OnSetHardPrivacyModeOnHandler(value);
    }
    else if (name == "AvAnalysisSessionStart")
    {
        self->OnAvAnalysisSessionStartHandler();
    }
    else if (name == "AvAnalysisPerceivedContext")
    {
        self->OnAvAnalysisPerceivedContextHandler();
    }
    else if (name == "AvAnalysisSessionEnd")
    {
        self->OnAvAnalysisSessionEndHandler();
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happen");
    }

exit:
    chip::Platform::Delete(self);
}

void CameraAppCommandHandler::SetCameraDevice(Camera::CameraDevice * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void CameraAppCommandHandler::OnZoneTriggeredHandler(const std::vector<uint16_t> & zoneIds)
{
    mCameraDevice->HandleSimulatedZoneTriggeredEvent(zoneIds);
}

void CameraAppCommandHandler::OnSetHardPrivacyModeOnHandler(bool value)
{
    TEMPORARY_RETURN_IGNORED mCameraDevice->GetCameraAVStreamMgmtController().SetHardPrivacyModeOn(value);
}

static AvAnalysis::Structs::TrackedContext::Type ParseTrackedContext(const Json::Value & ctxVal)
{
    AvAnalysis::Structs::TrackedContext::Type tc;
    if (ctxVal.isMember("NamespaceId") || ctxVal.isMember("namespaceID"))
    {
        const auto & ns                  = ctxVal.isMember("NamespaceId") ? ctxVal["NamespaceId"] : ctxVal["namespaceID"];
        tc.identifiedContext.namespaceID = static_cast<uint8_t>(ns.asUInt());
    }
    if (ctxVal.isMember("Tag") || ctxVal.isMember("tag"))
    {
        const auto & tag         = ctxVal.isMember("Tag") ? ctxVal["Tag"] : ctxVal["tag"];
        tc.identifiedContext.tag = static_cast<uint16_t>(tag.asUInt());
    }
    if (ctxVal.isMember("IdentifiedContextId") || ctxVal.isMember("identifiedContextID"))
    {
        const auto & idVal = ctxVal.isMember("IdentifiedContextId") ? ctxVal["IdentifiedContextId"] : ctxVal["identifiedContextID"];
        tc.identifiedContextID = static_cast<uint16_t>(idVal.asUInt());
    }
    if (ctxVal.isMember("CurrentZone") || ctxVal.isMember("currentZone"))
    {
        const auto & czVal = ctxVal.isMember("CurrentZone") ? ctxVal["CurrentZone"] : ctxVal["currentZone"];
        if (czVal.isNull())
        {
            tc.currentZone.SetValue(DataModel::NullNullable);
        }
        else
        {
            tc.currentZone.SetValue(DataModel::MakeNullable(static_cast<uint16_t>(czVal.asUInt())));
        }
    }
    if (ctxVal.isMember("PreviousZone") || ctxVal.isMember("previousZone"))
    {
        const auto & pzVal = ctxVal.isMember("PreviousZone") ? ctxVal["PreviousZone"] : ctxVal["previousZone"];
        if (pzVal.isNull())
        {
            tc.previousZone.SetValue(DataModel::NullNullable);
        }
        else
        {
            tc.previousZone.SetValue(DataModel::MakeNullable(static_cast<uint16_t>(pzVal.asUInt())));
        }
    }
    return tc;
}

void CameraAppCommandHandler::OnAvAnalysisSessionStartHandler()
{
    VerifyOrReturn(mCameraDevice != nullptr);

    std::vector<uint16_t> zoneIds;
    bool zoneIdsNull = true;

    if (mJsonValue.isMember("ZoneIds") && !mJsonValue["ZoneIds"].isNull())
    {
        zoneIdsNull      = false;
        const auto & val = mJsonValue["ZoneIds"];
        if (val.isArray())
        {
            for (const auto & item : val)
            {
                zoneIds.push_back(static_cast<uint16_t>(item.asUInt()));
            }
        }
        else
        {
            zoneIds.push_back(static_cast<uint16_t>(val.asUInt()));
        }
    }
    else if (mJsonValue.isMember("ZoneId") && !mJsonValue["ZoneId"].isNull())
    {
        zoneIdsNull      = false;
        const auto & val = mJsonValue["ZoneId"];
        if (val.isArray())
        {
            for (const auto & item : val)
            {
                zoneIds.push_back(static_cast<uint16_t>(item.asUInt()));
            }
        }
        else
        {
            zoneIds.push_back(static_cast<uint16_t>(val.asUInt()));
        }
    }

    Optional<NodeId> sourceNodeId;
    if (mJsonValue.isMember("SourceNodeId") && !mJsonValue["SourceNodeId"].isNull())
    {
        sourceNodeId.SetValue(static_cast<NodeId>(mJsonValue["SourceNodeId"].asUInt64()));
    }

    TEMPORARY_RETURN_IGNORED mCameraDevice->GetAVAnalysisManager().TriggerSessionStart(zoneIds, zoneIdsNull, sourceNodeId);
}

void CameraAppCommandHandler::OnAvAnalysisPerceivedContextHandler()
{
    VerifyOrReturn(mCameraDevice != nullptr);

    std::vector<AvAnalysis::Structs::TrackedContext::Type> newContexts;
    std::vector<AvAnalysis::Structs::TrackedContext::Type> expiredContexts;

    if (mJsonValue.isMember("NewContexts") && mJsonValue["NewContexts"].isArray())
    {
        for (const auto & item : mJsonValue["NewContexts"])
        {
            newContexts.push_back(ParseTrackedContext(item));
        }
    }
    else if (mJsonValue.isMember("Context") && mJsonValue["Context"].isObject())
    {
        newContexts.push_back(ParseTrackedContext(mJsonValue["Context"]));
    }
    else if (mJsonValue.isMember("NamespaceId") || mJsonValue.isMember("Tag"))
    {
        newContexts.push_back(ParseTrackedContext(mJsonValue));
    }

    if (mJsonValue.isMember("ExpiredContexts") && mJsonValue["ExpiredContexts"].isArray())
    {
        for (const auto & item : mJsonValue["ExpiredContexts"])
        {
            expiredContexts.push_back(ParseTrackedContext(item));
        }
    }

    Optional<uint16_t> sessionId;
    if (mJsonValue.isMember("SessionId") && !mJsonValue["SessionId"].isNull())
    {
        sessionId.SetValue(static_cast<uint16_t>(mJsonValue["SessionId"].asUInt()));
    }

    Optional<NodeId> sourceNodeId;
    if (mJsonValue.isMember("SourceNodeId") && !mJsonValue["SourceNodeId"].isNull())
    {
        sourceNodeId.SetValue(static_cast<NodeId>(mJsonValue["SourceNodeId"].asUInt64()));
    }

    TEMPORARY_RETURN_IGNORED mCameraDevice->GetAVAnalysisManager().TriggerPerceivedContext(newContexts, expiredContexts, sessionId,
                                                                                           sourceNodeId);
}

void CameraAppCommandHandler::OnAvAnalysisSessionEndHandler()
{
    VerifyOrReturn(mCameraDevice != nullptr);

    Optional<uint16_t> sessionId;
    if (mJsonValue.isMember("SessionId") && !mJsonValue["SessionId"].isNull())
    {
        sessionId.SetValue(static_cast<uint16_t>(mJsonValue["SessionId"].asUInt()));
    }

    Optional<NodeId> sourceNodeId;
    if (mJsonValue.isMember("SourceNodeId") && !mJsonValue["SourceNodeId"].isNull())
    {
        sourceNodeId.SetValue(static_cast<NodeId>(mJsonValue["SourceNodeId"].asUInt64()));
    }

    TEMPORARY_RETURN_IGNORED mCameraDevice->GetAVAnalysisManager().TriggerSessionEnd(sessionId, sourceNodeId);
}

void CameraAppCommandDelegate::SetCameraDevice(Camera::CameraDevice * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

void CameraAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = CameraAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "Camera App: Unable to instantiate a command handler");
        return;
    }

    handler->SetCameraDevice(mCameraDevice);
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(CameraAppCommandHandler::HandleCommand,
                                                                           reinterpret_cast<intptr_t>(handler));
}
