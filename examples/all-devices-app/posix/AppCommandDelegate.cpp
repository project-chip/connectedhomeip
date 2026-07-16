/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "include/AppCommandDelegate.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/ambient-context-sensing-server/CodegenIntegration.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

struct CommandContext
{
    Json::Value value;
    EndpointId endpointId;
    AllDevicesAppCommandDelegate * delegate;
    AllDevicesAppNamedPipeCommandHandler * handler;
};

class IncreaseConfigurationVersionCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "IncreaseConfigurationVersion"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::BasicInformationCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "BasicInformationCluster not found on endpoint %d", endpointId);
            return;
        }

        CHIP_ERROR err = cluster->IncreaseConfigurationVersion();
        ChipLogProgress(AppServer, "IncreaseConfigurationVersion on endpoint %d: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
};

class SetOccupancyCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetOccupancy"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::OccupancySensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "OccupancySensingCluster not found on endpoint %d", endpointId);
            return;
        }

        if (!json.isMember("Occupancy") || !json["Occupancy"].isUInt())
        {
            ChipLogError(AppServer, "Invalid SetOccupancy command: missing 'Occupancy' field");
            return;
        }

        unsigned int occupancyVal = json["Occupancy"].asUInt();
        if (occupancyVal != 0 && occupancyVal != 1)
        {
            ChipLogError(AppServer, "Invalid occupancy value: %u", occupancyVal);
            return;
        }
        uint8_t occupancy = static_cast<uint8_t>(occupancyVal);

        cluster->SetOccupancy(occupancy != 0);
        ChipLogProgress(AppServer, "SetOccupancy to %d on endpoint %d", occupancy, endpointId);
    }
};

class SetHoldTimeCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetHoldTime"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::OccupancySensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "OccupancySensingCluster not found on endpoint %d", endpointId);
            return;
        }

        if (!json.isMember("HoldTime") || !json["HoldTime"].isUInt())
        {
            ChipLogError(AppServer, "Invalid SetHoldTime command: missing 'HoldTime' field");
            return;
        }

        unsigned int holdTimeVal = json["HoldTime"].asUInt();
        if (holdTimeVal > 0xFFFF)
        {
            ChipLogError(AppServer, "Invalid HoldTime value (out of range): %u", holdTimeVal);
            return;
        }
        uint16_t holdTime = static_cast<uint16_t>(holdTimeVal);
        cluster->SetHoldTime(holdTime);
        ChipLogProgress(AppServer, "SetHoldTime to %d on endpoint %d", holdTime, endpointId);
    }
};

/**
 * Named pipe handler for setting supported ambient context type
 *
 * Usage example:
 *   echo '{"Name":"SetAmbientContextSupport","EndpointId":1,"AmbientContextType":[{"TypeId":73, "TagId":2},{"TypeId":74,
 * "TagId":2},{"TypeId":75, "TagId":2}]}'> /tmp/acs_fifo
 *
 * JSON Arguments:
 *   - "Name": Must be "SetAmbientContextSupport"
 *   - "EndpointId": ID of endpoint
 *   - "AmbientContextType": array of the supported ambient context sensing type
 *
 * @param jsonValue - JSON payload from named pipe
 */
class SetAmbientContextSupportCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetAmbientContextSupport"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::AmbientContextSensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "AmbientContextSensingCluster not found on endpoint %d", endpointId);
            return;
        }
        // Validate AmbientContextType exists and is an array
        if (!json.isMember("AmbientContextType") || !json["AmbientContextType"].isArray())
        {
            std::string inputJson = json.toStyledString();
            ChipLogError(AppServer, "Missing or invalid AmbientContextType array: %s", inputJson.c_str());
            return;
        }
        const Json::Value & actArray = json["AmbientContextType"];
        if (actArray.empty())
        {
            ChipLogError(AppServer, "AmbientContextType array is empty");
            return;
        }
        if (actArray.size() > AmbientContextSensing::kMaxACTypeSupported)
        {
            ChipLogError(AppServer, "AmbientContextType array too large: %u", static_cast<unsigned>(actArray.size()));
            return;
        }
        Span<Globals::Structs::SemanticTagStruct::Type> semanticTags;
        std::unique_ptr<Globals::Structs::SemanticTagStruct::Type[]> tagBuf =
            std::make_unique<Globals::Structs::SemanticTagStruct::Type[]>(actArray.size());
        for (Json::ArrayIndex i = 0; i < actArray.size(); i++)
        {
            Json::Value item = actArray[i];
            if (!item.isObject() || !item.isMember("TypeId") || !item.isMember("TagId"))
            {
                std::string inputJson = json.toStyledString();
                ChipLogError(AppServer, "AmbientContextType[%u], missing/invalid TypeId/TagId in %s", static_cast<uint16_t>(i),
                             inputJson.c_str());
                return;
            }
            uint8_t typeId = static_cast<uint8_t>(item["TypeId"].asUInt());
            uint8_t tagId  = static_cast<uint8_t>(item["TagId"].asUInt());

            ChipLogDetail(AppServer, "AmbientContextType[%u] -> (TypeId, TagId) = (%u, %u)", static_cast<unsigned>(i), typeId,
                          tagId);
            tagBuf[i].namespaceID = typeId;
            tagBuf[i].tag         = tagId;
        }
        semanticTags = Span<Globals::Structs::SemanticTagStruct::Type>(tagBuf.get(), actArray.size());
        LogErrorOnFailure(cluster->SetAmbientContextTypeSupported(semanticTags));
    }
};

static bool GetAmbientContextType(const Json::Value & actArray,
                                  std::vector<Globals::Structs::SemanticTagStruct::Type> & semanticTags)
{
    // Validate AmbientContextType exists and is an array
    if (actArray.empty())
    {
        ChipLogError(AppServer, "AmbientContextType array is empty");
        return false;
    }
    for (Json::ArrayIndex i = 0; i < actArray.size(); i++)
    {
        Json::Value item = actArray[i];
        if (!item.isObject() || !item.isMember("TypeId") || !item.isMember("TagId"))
        {
            ChipLogError(AppServer, "AmbientContextType[%u], missing/invalid TypeId/TagId", static_cast<uint16_t>(i));
            return false;
        }
        uint8_t typeId = static_cast<uint8_t>(item["TypeId"].asUInt());
        uint8_t tagId  = static_cast<uint8_t>(item["TagId"].asUInt());

        Globals::Structs::SemanticTagStruct::Type tag = {
            .namespaceID = typeId,
            .tag         = tagId,
        };
        semanticTags.push_back(tag);
    }
    return true;
}

/**
 * Named pipe handler for ambient context detection
 *
 * Usage example:
 *   echo '{"Name":"AddAmbientContextDetect","EndpointId":1,"AmbientContextType":[{"TypeId":75, "TagId":2},{"TypeId":73,
 * "TagId":2}],"DetectionConfidence":100}'> /tmp/acs_fifo
 *
 * JSON Arguments:
 *   - "Name": Must be "AddAmbientContextDetect"
 *   - "EndpointId": ID of endpoint
 *   - "AmbientContextType": Type of the detection
 *
 * @param jsonValue - JSON payload from named pipe
 */
class AddAmbientContextDetectCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "AddAmbientContextDetect"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::AmbientContextSensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "AmbientContextSensingCluster not found on endpoint %d", endpointId);
            return;
        }
        // Validate AmbientContextType exists and is an array
        if (!json.isMember("AmbientContextType") || !json["AmbientContextType"].isArray())
        {
            std::string inputJson = json.toStyledString();
            ChipLogError(AppServer, "Missing or invalid AmbientContextType array: %s", inputJson.c_str());
            return;
        }
        const Json::Value & actArray = json["AmbientContextType"];
        if (actArray.empty())
        {
            ChipLogError(AppServer, "AmbientContextType array is empty");
            return;
        }
        std::vector<Globals::Structs::SemanticTagStruct::Type> semanticTags;
        semanticTags.reserve(actArray.size());
        if (!::GetAmbientContextType(actArray, semanticTags))
        {
            ChipLogError(AppServer, "Incorrect or unsupported detection");
            return;
        }
        auto tagList =
            chip::app::DataModel::List<const Globals::Structs::SemanticTagStruct::Type>(semanticTags.data(), semanticTags.size());
        AmbientContextSensing::Structs::AmbientContextTypeStruct::Type ACSType = { .ambientContextSensed = tagList };
        // Add DetectionConfidence
        if (json.isMember("DetectionConfidence"))
        {
            auto confidenceValue = json["DetectionConfidence"].asUInt();
            if ((confidenceValue == 0) || (confidenceValue > 100))
            {
                ChipLogError(AppServer, "Incorrect or unsupported confidence value");
                return;
            }
            ACSType.detectionConfidence.SetValue(static_cast<chip::Percent>(confidenceValue));
        }
        LogErrorOnFailure(cluster->AddDetection(ACSType));
    }
};

/**
 * Named pipe handler for predicted activity
 *
 * Usage example:
 *   echo '{"Name":"SetPredictedActivity","EndpointId":1,"PredAct":[
 {"StartTStamp":1769138873, "EndTStamp":1769138883,"AmbientContextType":[{"TypeId":75, "TagId":2},{"TypeId":73,
 "TagId":2}],"CrowdDetect":true,"CrowdCnt":10,"Conf":89},
 {"StartTStamp":1769138893, "EndTStamp":1769138903,"AmbientContextType":[{"TypeId":75, "TagId":3},{"TypeId":73,
 "TagId":4}],"CrowdDetect":true,"CrowdCnt":11,"Conf":90}
 ]}'> /tmp/acs_fifo
 *
 * JSON Arguments:
 *   - "Name": Must be "SetPredictedActivity"
 *   - "EndpointId": ID of endpoint
 *   - "PredAct": The server's prediction of upcoming changes
 *   - "StartTStamp": Start Timestamp
 *   - "EndTStamp": End Timestamp
 *   - "AmbientContextType": Type of the detection
 *   - "CrowdDetect": The predicted state of the CrowdDetected attribute
 *   - "CrowdCnt": The predicted value of the CrowdCount attribute
 *   - "Conf": Confidence level for the predicted activity state
 *
 * @param jsonValue - JSON payload from named pipe
 */
class SetPredictedActivityCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetPredictedActivity"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::AmbientContextSensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "AmbientContextSensingCluster not found on endpoint %d", endpointId);
            return;
        }
        // Validate PredictedActivity exists and is an array
        if (!json.isMember("PredAct") || !json["PredAct"].isArray())
        {
            std::string inputJson = json.toStyledString();
            ChipLogError(AppServer, "Missing or invalid PredictedActivity array: %s", inputJson.c_str());
            return;
        }
        const Json::Value & predictArray = json["PredAct"];
        if (predictArray.empty())
        {
            ChipLogError(AppServer, "PredictedActivity array is empty");
            return;
        }
        if (predictArray.size() > AmbientContextSensing::kMaxPredictedActivity)
        {
            ChipLogError(AppServer, "PredictedActivity array too large: %u", static_cast<unsigned>(predictArray.size()));
            return;
        }
        std::unique_ptr<AmbientContextSensing::Structs::PredictedActivityStruct::Type[]> predictArrayBuf =
            std::make_unique<AmbientContextSensing::Structs::PredictedActivityStruct::Type[]>(predictArray.size());
        std::vector<std::vector<Globals::Structs::SemanticTagStruct::Type>> allSemanticTags;
        allSemanticTags.resize(predictArray.size());

        for (Json::ArrayIndex i = 0; i < predictArray.size(); i++)
        {
            Json::Value item = predictArray[i];
            if (!item.isObject() || !item.isMember("StartTStamp") || !item.isMember("EndTStamp") || !item.isMember("Conf"))
            {
                std::string inputJson = json.toStyledString();
                ChipLogError(AppServer, "PredictedActivity[%u], missing/invalid fields in %s", static_cast<uint16_t>(i),
                             inputJson.c_str());
                return;
            }
            AmbientContextSensing::Structs::PredictedActivityStruct::Type predictAct;
            predictAct.startTimestamp = item["StartTStamp"].asUInt();
            predictAct.endTimestamp   = item["EndTStamp"].asUInt();
            VerifyOrReturn(predictAct.startTimestamp < predictAct.endTimestamp,
                           ChipLogError(AppServer, "PredictedActivity, incorrect startTime/endTime"));
            auto confidenceValue = item["Conf"].asUInt();
            if (confidenceValue > 100)
            {
                ChipLogError(AppServer, "PredictedActivity, incorrect confidence value");
                return;
            }
            predictAct.confidence = static_cast<chip::Percent>(confidenceValue);
            if (item.isMember("CrowdDetect"))
            {
                predictAct.crowdDetected.SetValue(static_cast<bool>(item["CrowdDetect"].asBool()));
            }
            if (item.isMember("CrowdCnt"))
            {
                const auto crowdCntValue = item["CrowdCnt"].asUInt();
                if ((crowdCntValue == 0) || (crowdCntValue > AmbientContextSensing::kMaxCrowdCount))
                {
                    ChipLogError(AppServer, "PredictedActivity[%u], invalid CrowdCnt value: %u", static_cast<uint16_t>(i),
                                 static_cast<unsigned>(crowdCntValue));
                    return;
                }
                predictAct.crowdCount.SetValue(static_cast<uint8_t>(crowdCntValue));
            }
            // Validate AmbientContextType exists and is an array
            if (!item.isMember("AmbientContextType") || !item["AmbientContextType"].isArray())
            {
                std::string inputJson = item.toStyledString();
                ChipLogError(AppServer, "Missing or invalid AmbientContextType array: %s", inputJson.c_str());
                return;
            }
            const Json::Value & actArray = item["AmbientContextType"];
            if (actArray.empty())
            {
                ChipLogError(AppServer, "AmbientContextType array is empty");
                return;
            }
            auto & semanticTags = allSemanticTags[i];
            semanticTags.clear();
            semanticTags.reserve(actArray.size());
            if (!::GetAmbientContextType(actArray, semanticTags))
            {
                ChipLogError(AppServer, "Incorrect or unsupported detection");
                return;
            }

            auto tagList = chip::app::DataModel::List<const Globals::Structs::SemanticTagStruct::Type>(semanticTags.data(),
                                                                                                       semanticTags.size());
            predictAct.ambientContextType.SetValue(tagList);
            predictArrayBuf[i] = predictAct;
        }
        Span<AmbientContextSensing::Structs::PredictedActivityStruct::Type> predictedActivityArray =
            Span<AmbientContextSensing::Structs::PredictedActivityStruct::Type>(predictArrayBuf.get(), predictArray.size());
        LogErrorOnFailure(cluster->SetPredictedActivity(predictedActivityArray));
    }
};

/**
 * Named pipe handler for setting object count
 *
 * Usage example:
 *   echo '{"Name":"SetObjCount","EndpointId":1,"ObjectCount":11}'> /tmp/acs_fifo
 *
 * JSON Arguments:
 *   - "Name": Must be "SetObjCount"
 *   - "EndpointId": ID of endpoint
 *   - "ObjectCount": value of ObjectCount
 *
 * @param jsonValue - JSON payload from named pipe
 */
class SetObjCountCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetObjCount"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::AmbientContextSensingCluster>(
                endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "AmbientContextSensingCluster not found on endpoint %d", endpointId);
            return;
        }
        uint16_t objCount;
        // Add ObjectCount
        if (!json.isMember("ObjectCount"))
        {
            ChipLogError(AppServer, "Missing or invalid value for ObjectCount");
            return;
        }
        objCount = static_cast<uint16_t>(json["ObjectCount"].asUInt());
        LogErrorOnFailure(cluster->SetObjectCount(objCount));
    }
};

class SetBooleanStateCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetBooleanState"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::BooleanStateCluster>(endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "BooleanStateCluster not found on endpoint %d", endpointId);
            return;
        }

        if (!json.isMember("NewState") || !json["NewState"].isBool())
        {
            ChipLogError(AppServer, "Invalid SetBooleanState command: missing 'NewState' field");
            return;
        }

        bool newState = json["NewState"].asBool();
        cluster->SetStateValue(newState);
        ChipLogProgress(AppServer, "SetBooleanState to %d on endpoint %d", newState, endpointId);
    }
};

class SetOnOffCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SetOnOff"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        auto * cluster =
            delegate->GetClusterImplementationRegistry().GetClusterByEndpoint<chip::app::Clusters::OnOffCluster>(endpointId);
        if (!cluster)
        {
            ChipLogError(AppServer, "OnOffCluster not found on endpoint %d", endpointId);
            return;
        }

        if (!json.isMember("OnOff") || !json["OnOff"].isBool())
        {
            ChipLogError(AppServer, "Invalid SetOnOff command: missing 'OnOff' field");
            return;
        }

        bool onOff     = json["OnOff"].asBool();
        CHIP_ERROR err = cluster->SetOnOff(onOff);
        ChipLogProgress(AppServer, "SetOnOff to %d on endpoint %d: %" CHIP_ERROR_FORMAT, onOff, endpointId, err.Format());
    }
};

} // namespace

void AllDevicesAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(json, value))
    {
        ChipLogError(AppServer, "Failed to parse JSON command: %s", reader.getFormattedErrorMessages().c_str());
        return;
    }

    if (!value.isMember("Name") || !value["Name"].isString() || !value.isMember("EndpointId") || !value["EndpointId"].isUInt())
    {
        ChipLogError(AppServer, "Invalid command format: %s", json);
        return;
    }

    std::string commandName = value["Name"].asString();

    unsigned int endpointIdVal = value["EndpointId"].asUInt();
    if (endpointIdVal > 0xFFFF)
    {
        ChipLogError(AppServer, "Invalid EndpointId (out of range): %u", endpointIdVal);
        return;
    }

    EndpointId endpointId = static_cast<EndpointId>(endpointIdVal);
    auto handlerIt        = mCommandHandlers.find(commandName);

    if (handlerIt == mCommandHandlers.end())
    {
        ChipLogError(AppServer, "Unknown command: %s", commandName.c_str());
        return;
    }

    auto * context = Platform::New<CommandContext>();
    if (context == nullptr)
    {
        ChipLogError(AppServer, "Failure to allocate command context! Ignoring command.");
        return;
    }
    context->value      = value;
    context->endpointId = endpointId;
    context->delegate   = this;
    context->handler    = handlerIt->second.get();

    CHIP_ERROR err = DeviceLayer::PlatformMgr().ScheduleWork(DispatchCommand, reinterpret_cast<intptr_t>(context));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
        Platform::Delete(context);
    }
}

void AllDevicesAppCommandDelegate::DispatchCommand(intptr_t context)
{
    auto * cmdContext = reinterpret_cast<CommandContext *>(context);
    cmdContext->handler->Handle(cmdContext->value, cmdContext->delegate, cmdContext->endpointId);
    Platform::Delete(cmdContext);
}

void AllDevicesAppCommandDelegate::RegisterCommandHandler(std::unique_ptr<AllDevicesAppNamedPipeCommandHandler> handler)
{
    mCommandHandlers[handler->GetName()] = std::move(handler);
}

void AllDevicesAppCommandDelegate::RegisterCommandHandlers()
{
    RegisterCommandHandler(std::make_unique<IncreaseConfigurationVersionCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetOccupancyCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetHoldTimeCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetAmbientContextSupportCommandHandler>());
    RegisterCommandHandler(std::make_unique<AddAmbientContextDetectCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetPredictedActivityCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetObjCountCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetBooleanStateCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetOnOffCommandHandler>());
}
