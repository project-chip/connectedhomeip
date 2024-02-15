/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "TargetNavigatorManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <json/json.h>

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::TargetNavigator;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;

TargetNavigatorManager::TargetNavigatorManager(ContentAppAttributeDelegate * attributeDelegate, std::list<std::string> targets,
                                               uint8_t currentTarget) :
    mAttributeDelegate(attributeDelegate)
{
    mTargets       = targets;
    mCurrentTarget = currentTarget;
}

CHIP_ERROR TargetNavigatorManager::HandleGetTargetList(AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "TargetNavigatorManager::HandleNavigateTarget");

    if (mAttributeDelegate != nullptr)
    {
        chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::TargetNavigator::Id,
                                                   chip::app::Clusters::TargetNavigator::Attributes::TargetList::Id);
        std::string resStr = mAttributeDelegate->Read(aPath);
        ChipLogProgress(Zcl, "TargetNavigatorManager::HandleNavigateTarget response %s", resStr.c_str());

        if (resStr.length() != 0)
        {
            Json::Reader reader;
            Json::Value value;
            if (reader.parse(resStr, value))
            {
                std::string attrId = to_string(chip::app::Clusters::TargetNavigator::Attributes::TargetList::Id);
                ChipLogProgress(Zcl, "TargetNavigatorManager::HandleNavigateTarget response parsing done. reading attr %s",
                                attrId.c_str());
                if (value[attrId].isArray())
                {
                    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
                        int i                  = 0;
                        std::string targetId   = to_string(static_cast<uint32_t>(
                            chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::Fields::kIdentifier));
                        std::string targetName = to_string(
                            static_cast<uint32_t>(chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::Fields::kName));
                        for (Json::Value & entry : value[attrId])
                        {
                            if (!entry[targetId].isUInt() || !entry[targetName].isString() || entry[targetId].asUInt() > 255)
                            {
                                // invalid target ID. Ignore.
                                ChipLogError(Zcl, "TargetNavigatorManager::HandleNavigateTarget invalid target ignored");
                                i++;
                                continue;
                            }
                            Structs::TargetInfoStruct::Type outputInfo;
                            outputInfo.identifier = static_cast<uint8_t>(entry[targetId].asUInt());
                            outputInfo.name       = CharSpan::fromCharString(entry[targetName].asCString());
                            ReturnErrorOnFailure(encoder.Encode(outputInfo));
                            i++;
                        }
                        return CHIP_NO_ERROR;
                    });
                }
            }
        }
    }

    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        int i = 0;
        for (std::string & entry : mTargets)
        {
            Structs::TargetInfoStruct::Type outputInfo;
            outputInfo.identifier = static_cast<uint8_t>(i + 1);
            outputInfo.name       = CharSpan::fromCharString(entry.c_str());
            ReturnErrorOnFailure(encoder.Encode(outputInfo));
            i++;
        }
        return CHIP_NO_ERROR;
    });
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    ChipLogProgress(Zcl, "TargetNavigatorManager::HandleGetCurrentTarget");

    if (mAttributeDelegate != nullptr)
    {
        chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::TargetNavigator::Id,
                                                   chip::app::Clusters::TargetNavigator::Attributes::TargetList::Id);
        std::string resStr = mAttributeDelegate->Read(aPath);
        ChipLogProgress(Zcl, "TargetNavigatorManager::HandleGetCurrentTarget response %s", resStr.c_str());

        if (resStr.length() != 0)
        {
            Json::Reader reader;
            Json::Value value;
            if (reader.parse(resStr, value))
            {
                std::string attrId = to_string(chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::Id);
                ChipLogProgress(Zcl, "TargetNavigatorManager::HandleGetCurrentTarget response parsing done. reading attr %s",
                                attrId.c_str());
                if (value[attrId].isUInt() && value[attrId].asUInt() < 256)
                {
                    return static_cast<uint8_t>(value[attrId].asUInt());
                }
            }
        }
    }

    return mCurrentTarget;
}

void TargetNavigatorManager::HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & helper,
                                                  const uint64_t & target, const CharSpan & data)
{
    NavigateTargetResponseType response;
    if (target == kNoCurrentTarget || target > mTargets.size())
    {
        response.data   = chip::MakeOptional(CharSpan::fromCharString("error"));
        response.status = StatusEnum::kTargetNotFound;
        helper.Success(response);
        return;
    }
    mCurrentTarget = static_cast<uint8_t>(target);

    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

uint16_t TargetNavigatorManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "TargetNavigatorManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
