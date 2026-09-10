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
#include <cstdint>
#include <lib/support/SafeInt.h>
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
    else if (name == "AmbientContextTriggered")
    {
        const Json::Value & namespaceValue        = self->mJsonValue["NamespaceId"];
        const Json::Value & tagValue              = self->mJsonValue["TagId"];
        const Json::Value & zoneIdValues          = self->mJsonValue["ZoneIds"];
        const Json::Value & identifedContextValue = self->mJsonValue["IdentifiedContextId"];

        VerifyOrExit(namespaceValue.isUInt() && namespaceValue.asUInt() <= UINT8_MAX,
                     ChipLogError(NotSpecified, "Camera App: NamespaceId is missing, invalid or out of unsigned 8-bit range"));
        VerifyOrExit(tagValue.isUInt() && tagValue.asUInt() <= UINT8_MAX,
                     ChipLogError(NotSpecified, "Camera App: TagId is missing, invalid or out of unsigned 8-bit range"));
        VerifyOrExit(
            identifedContextValue.isUInt() && identifedContextValue.asUInt() <= UINT16_MAX,
            ChipLogError(NotSpecified, "Camera App: IdentifiedContextId is missing, invalid or out of unsigned 16-bit range"));

        // ZoneIds is always an array, it may be empty
        VerifyOrExit(zoneIdValues.isArray(), ChipLogError(NotSpecified, "Camera App: ZoneIds must be an array"));

        std::vector<uint16_t> zoneIds;
        for (const auto & zoneId : zoneIdValues)
        {
            VerifyOrExit(zoneId.isUInt() && zoneId.asUInt() <= UINT16_MAX,
                         ChipLogError(NotSpecified, "Camera App: ZoneIds entry is invalid or out of unsigned 16-bit range"));
            zoneIds.push_back(static_cast<uint16_t>(zoneId.asUInt()));
        }

        self->OnAmbientContextTriggeredHandler(static_cast<uint8_t>(namespaceValue.asUInt()),
                                               static_cast<uint8_t>(tagValue.asUInt()), zoneIds,
                                               static_cast<uint16_t>(identifedContextValue.asUInt()));
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

void CameraAppCommandHandler::OnAmbientContextTriggeredHandler(uint8_t namespaceId, uint8_t tagId, std::vector<uint16_t> zoneIds,
                                                               uint16_t identifiedContextId)
{
    mCameraDevice->HandleSimulatedAmbientContextTriggeredEvent(namespaceId, tagId, zoneIds, identifiedContextId);
}

static AvAnalysis::Structs::TrackedContext::Type ParseTrackedContext(const Json::Value & ctxVal)
{
    AvAnalysis::Structs::TrackedContext::Type tc;
    if (ctxVal.isMember("NamespaceId") || ctxVal.isMember("namespaceID"))
    {
        const auto & ns = ctxVal.isMember("NamespaceId") ? ctxVal["NamespaceId"] : ctxVal["namespaceID"];
        uint32_t val    = ns.asUInt();
        if (chip::CanCastTo<uint8_t>(val))
        {
            tc.identifiedContext.namespaceID = static_cast<uint8_t>(val);
        }
    }
    if (ctxVal.isMember("Tag") || ctxVal.isMember("tag"))
    {
        const auto & tag = ctxVal.isMember("Tag") ? ctxVal["Tag"] : ctxVal["tag"];
        uint32_t val     = tag.asUInt();
        if (chip::CanCastTo<uint16_t>(val))
        {
            tc.identifiedContext.tag = static_cast<uint16_t>(val);
        }
    }
    if (ctxVal.isMember("IdentifiedContextId") || ctxVal.isMember("identifiedContextID"))
    {
        const auto & idVal = ctxVal.isMember("IdentifiedContextId") ? ctxVal["IdentifiedContextId"] : ctxVal["identifiedContextID"];
        uint32_t val       = idVal.asUInt();
        if (chip::CanCastTo<uint16_t>(val))
        {
            tc.identifiedContextID = static_cast<uint16_t>(val);
        }
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
            uint32_t val = czVal.asUInt();
            if (chip::CanCastTo<uint16_t>(val))
            {
                tc.currentZone.SetValue(DataModel::MakeNullable(static_cast<uint16_t>(val)));
            }
            else
            {
                ChipLogError(NotSpecified, "CurrentZone value %" PRIu32 " exceeds uint16_t range", val);
            }
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
            uint32_t val = pzVal.asUInt();
            if (chip::CanCastTo<uint16_t>(val))
            {
                tc.previousZone.SetValue(DataModel::MakeNullable(static_cast<uint16_t>(val)));
            }
            else
            {
                ChipLogError(NotSpecified, "PreviousZone value %" PRIu32 " exceeds uint16_t range", val);
            }
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
                uint32_t zVal = item.asUInt();
                if (chip::CanCastTo<uint16_t>(zVal))
                {
                    zoneIds.push_back(static_cast<uint16_t>(zVal));
                }
                else
                {
                    ChipLogError(NotSpecified, "ZoneIds value %" PRIu32 " exceeds uint16_t range", zVal);
                }
            }
        }
        else
        {
            uint32_t zVal = val.asUInt();
            if (chip::CanCastTo<uint16_t>(zVal))
            {
                zoneIds.push_back(static_cast<uint16_t>(zVal));
            }
            else
            {
                ChipLogError(NotSpecified, "ZoneIds value %" PRIu32 " exceeds uint16_t range", zVal);
            }
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
                uint32_t zVal = item.asUInt();
                if (chip::CanCastTo<uint16_t>(zVal))
                {
                    zoneIds.push_back(static_cast<uint16_t>(zVal));
                }
                else
                {
                    ChipLogError(NotSpecified, "ZoneId value %" PRIu32 " exceeds uint16_t range", zVal);
                }
            }
        }
        else
        {
            uint32_t zVal = val.asUInt();
            if (chip::CanCastTo<uint16_t>(zVal))
            {
                zoneIds.push_back(static_cast<uint16_t>(zVal));
            }
            else
            {
                ChipLogError(NotSpecified, "ZoneId value %" PRIu32 " exceeds uint16_t range", zVal);
            }
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
        uint32_t sVal = mJsonValue["SessionId"].asUInt();
        if (chip::CanCastTo<uint16_t>(sVal))
        {
            sessionId.SetValue(static_cast<uint16_t>(sVal));
        }
        else
        {
            ChipLogError(NotSpecified, "SessionId value %" PRIu32 " exceeds uint16_t range", sVal);
        }
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
        uint32_t sVal = mJsonValue["SessionId"].asUInt();
        if (chip::CanCastTo<uint16_t>(sVal))
        {
            sessionId.SetValue(static_cast<uint16_t>(sVal));
        }
        else
        {
            ChipLogError(NotSpecified, "SessionId value %" PRIu32 " exceeds uint16_t range", sVal);
        }
    }

    TEMPORARY_RETURN_IGNORED mCameraDevice->GetAVAnalysisManager().TriggerSessionEnd(sessionId);
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
