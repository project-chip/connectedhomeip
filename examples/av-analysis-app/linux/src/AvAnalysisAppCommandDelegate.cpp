/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AvAnalysisAppCommandDelegate.h"

#include <cinttypes>
#include <cstdint>

#include <lib/support/CHIPMem.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

void ParseZoneIds(const Json::Value & aValue, std::vector<uint16_t> & aZoneIds)
{
    auto append = [&aZoneIds](const Json::Value & aItem) {
        uint32_t zoneId = aItem.asUInt();
        if (CanCastTo<uint16_t>(zoneId))
        {
            aZoneIds.push_back(static_cast<uint16_t>(zoneId));
        }
        else
        {
            ChipLogError(AppServer, "AvAnalysisNode pipe: zone id %" PRIu32 " exceeds uint16_t range", zoneId);
        }
    };
    if (aValue.isArray())
    {
        for (const auto & item : aValue)
        {
            append(item);
        }
    }
    else
    {
        append(aValue);
    }
}

Optional<uint16_t> ParseSessionId(const Json::Value & aJson)
{
    Optional<uint16_t> sessionId;
    if (aJson.isMember("SessionId") && !aJson["SessionId"].isNull())
    {
        uint32_t value = aJson["SessionId"].asUInt();
        if (CanCastTo<uint16_t>(value))
        {
            sessionId.SetValue(static_cast<uint16_t>(value));
        }
        else
        {
            ChipLogError(AppServer, "AvAnalysisNode pipe: SessionId %" PRIu32 " exceeds uint16_t range", value);
        }
    }
    return sessionId;
}

Optional<NodeId> ParseSourceNodeId(const Json::Value & aJson)
{
    Optional<NodeId> sourceNodeId;
    if (aJson.isMember("SourceNodeId") && !aJson["SourceNodeId"].isNull())
    {
        sourceNodeId.SetValue(static_cast<NodeId>(aJson["SourceNodeId"].asUInt64()));
    }
    return sourceNodeId;
}

AvAnalysis::Structs::TrackedContext::Type ParseTrackedContext(const Json::Value & aContext)
{
    AvAnalysis::Structs::TrackedContext::Type tracked;
    if (aContext.isMember("NamespaceId") || aContext.isMember("namespaceID"))
    {
        const auto & field = aContext.isMember("NamespaceId") ? aContext["NamespaceId"] : aContext["namespaceID"];
        uint32_t value     = field.asUInt();
        if (CanCastTo<uint8_t>(value))
        {
            tracked.identifiedContext.namespaceID = static_cast<uint8_t>(value);
        }
    }
    if (aContext.isMember("Tag") || aContext.isMember("tag"))
    {
        const auto & field = aContext.isMember("Tag") ? aContext["Tag"] : aContext["tag"];
        uint32_t value     = field.asUInt();
        if (CanCastTo<uint16_t>(value))
        {
            tracked.identifiedContext.tag = static_cast<uint16_t>(value);
        }
    }
    if (aContext.isMember("IdentifiedContextId") || aContext.isMember("identifiedContextID"))
    {
        const auto & field =
            aContext.isMember("IdentifiedContextId") ? aContext["IdentifiedContextId"] : aContext["identifiedContextID"];
        uint32_t value = field.asUInt();
        if (CanCastTo<uint16_t>(value))
        {
            tracked.identifiedContextID = static_cast<uint16_t>(value);
        }
    }
    if (aContext.isMember("CurrentZone") || aContext.isMember("currentZone"))
    {
        const auto & field = aContext.isMember("CurrentZone") ? aContext["CurrentZone"] : aContext["currentZone"];
        if (field.isNull())
        {
            tracked.currentZone.SetValue(DataModel::NullNullable);
        }
        else
        {
            uint32_t value = field.asUInt();
            if (CanCastTo<uint16_t>(value))
            {
                tracked.currentZone.SetValue(DataModel::MakeNullable(static_cast<uint16_t>(value)));
            }
        }
    }
    return tracked;
}

} // namespace

AvAnalysisAppCommandHandler * AvAnalysisAppCommandHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: JSON parse error: %s", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }
    if (value.empty() || !value.isObject())
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: the command is not a JSON object");
        return nullptr;
    }
    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: the command has no Name");
        return nullptr;
    }
    return Platform::New<AvAnalysisAppCommandHandler>(std::move(value));
}

void AvAnalysisAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self            = reinterpret_cast<AvAnalysisAppCommandHandler *>(context);
    const std::string name = self->mJsonValue["Name"].asString();

    if (name == "AvAnalysisSessionStart")
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
        ChipLogError(AppServer, "AvAnalysisNode pipe: unknown command %s", name.c_str());
    }

    Platform::Delete(self);
}

void AvAnalysisAppCommandHandler::OnAvAnalysisSessionStartHandler()
{
    VerifyOrReturn(mAnalysisDelegate != nullptr);

    std::vector<uint16_t> zoneIds;
    bool zoneIdsNull = true;
    if (mJsonValue.isMember("ZoneIds") && !mJsonValue["ZoneIds"].isNull())
    {
        zoneIdsNull = false;
        ParseZoneIds(mJsonValue["ZoneIds"], zoneIds);
    }
    else if (mJsonValue.isMember("ZoneId") && !mJsonValue["ZoneId"].isNull())
    {
        zoneIdsNull = false;
        ParseZoneIds(mJsonValue["ZoneId"], zoneIds);
    }

    CHIP_ERROR err = mAnalysisDelegate->TriggerSessionStart(zoneIds, zoneIdsNull, ParseSourceNodeId(mJsonValue));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: AvAnalysisSessionStart refused: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void AvAnalysisAppCommandHandler::OnAvAnalysisPerceivedContextHandler()
{
    VerifyOrReturn(mAnalysisDelegate != nullptr);

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

    CHIP_ERROR err = mAnalysisDelegate->TriggerPerceivedContext(newContexts, expiredContexts, ParseSessionId(mJsonValue),
                                                                ParseSourceNodeId(mJsonValue));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: AvAnalysisPerceivedContext refused: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void AvAnalysisAppCommandHandler::OnAvAnalysisSessionEndHandler()
{
    VerifyOrReturn(mAnalysisDelegate != nullptr);

    CHIP_ERROR err = mAnalysisDelegate->TriggerSessionEnd(ParseSessionId(mJsonValue));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: AvAnalysisSessionEnd refused: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void AvAnalysisAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    AvAnalysisAppCommandHandler * handler = AvAnalysisAppCommandHandler::FromJSON(json);
    VerifyOrReturn(handler != nullptr);

    handler->SetAnalysisDelegate(mAnalysisDelegate);
    // The pipe is read on its own thread; the command runs on the Matter thread
    CHIP_ERROR err =
        DeviceLayer::PlatformMgr().ScheduleWork(AvAnalysisAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AvAnalysisNode pipe: command not scheduled: %" CHIP_ERROR_FORMAT, err.Format());
        Platform::Delete(handler);
    }
}
