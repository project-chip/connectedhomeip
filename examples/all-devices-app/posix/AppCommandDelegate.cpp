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
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <platform/PlatformManager.h>
#include <binding-handler/binding-handler.h>

using namespace chip;
using namespace chip::app;

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

class SimulateBindingOnOffCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingOnOff"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        if (!json.isMember("On") || !json["On"].isBool())
        {
            ChipLogError(AppServer, "Invalid SimulateBindingOnOff command: missing 'On' field");
            return;
        }
        bool on = json["On"].asBool();
        SimulateBindingOnOff(endpointId, on);
        ChipLogProgress(AppServer, "SimulateBindingOnOff %d on endpoint %d", on, endpointId);
    }
};

class SimulateBindingToggleCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingToggle"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        SimulateBindingToggle(endpointId);
        ChipLogProgress(AppServer, "SimulateBindingToggle on endpoint %d", endpointId);
    }
};

class SimulateBindingMoveToLevelCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingMoveToLevel"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        if (!json.isMember("Level") || !json["Level"].isUInt())
        {
            ChipLogError(AppServer, "Invalid SimulateBindingMoveToLevel command: missing 'Level' field");
            return;
        }
        uint8_t level          = static_cast<uint8_t>(json["Level"].asUInt());
        uint8_t transitionTime = json.isMember("TransitionTime") && json["TransitionTime"].isUInt()
            ? static_cast<uint8_t>(json["TransitionTime"].asUInt())
            : 0;
        uint8_t optionsMask = json.isMember("OptionsMask") && json["OptionsMask"].isUInt()
            ? static_cast<uint8_t>(json["OptionsMask"].asUInt())
            : 0;

        SimulateBindingMoveToLevel(endpointId, level, transitionTime, optionsMask);
        ChipLogProgress(AppServer, "SimulateBindingMoveToLevel level=%u transitionTime=%u on endpoint %d", level, transitionTime, endpointId);
    }
};

class SimulateBindingMoveCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingMove"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        if (!json.isMember("MoveMode") || !json["MoveMode"].isUInt() || !json.isMember("Rate") || !json["Rate"].isUInt())
        {
            ChipLogError(AppServer, "Invalid SimulateBindingMove command: missing 'MoveMode' or 'Rate' field");
            return;
        }
        uint8_t moveMode    = static_cast<uint8_t>(json["MoveMode"].asUInt());
        uint8_t rate        = static_cast<uint8_t>(json["Rate"].asUInt());
        uint8_t optionsMask = json.isMember("OptionsMask") && json["OptionsMask"].isUInt()
            ? static_cast<uint8_t>(json["OptionsMask"].asUInt())
            : 0;

        SimulateBindingMove(endpointId, moveMode, rate, optionsMask);
        ChipLogProgress(AppServer, "SimulateBindingMove moveMode=%u rate=%u on endpoint %d", moveMode, rate, endpointId);
    }
};

class SimulateBindingStepCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingStep"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        if (!json.isMember("StepMode") || !json["StepMode"].isUInt() || !json.isMember("StepSize") || !json["StepSize"].isUInt())
        {
            ChipLogError(AppServer, "Invalid SimulateBindingStep command: missing 'StepMode' or 'StepSize' field");
            return;
        }
        uint8_t stepMode       = static_cast<uint8_t>(json["StepMode"].asUInt());
        uint8_t stepSize       = static_cast<uint8_t>(json["StepSize"].asUInt());
        uint8_t transitionTime = json.isMember("TransitionTime") && json["TransitionTime"].isUInt()
            ? static_cast<uint8_t>(json["TransitionTime"].asUInt())
            : 0;
        uint8_t optionsMask = json.isMember("OptionsMask") && json["OptionsMask"].isUInt()
            ? static_cast<uint8_t>(json["OptionsMask"].asUInt())
            : 0;

        SimulateBindingStep(endpointId, stepMode, stepSize, transitionTime, optionsMask);
        ChipLogProgress(AppServer, "SimulateBindingStep stepMode=%u stepSize=%u on endpoint %d", stepMode, stepSize, endpointId);
    }
};

class SimulateBindingStopCommandHandler : public AllDevicesAppNamedPipeCommandHandler
{
public:
    const char * GetName() const override { return "SimulateBindingStop"; }
    void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, EndpointId endpointId) override
    {
        SimulateBindingStop(endpointId);
        ChipLogProgress(AppServer, "SimulateBindingStop on endpoint %d", endpointId);
    }
};

void AllDevicesAppCommandDelegate::RegisterCommandHandler(std::unique_ptr<AllDevicesAppNamedPipeCommandHandler> handler)
{
    mCommandHandlers[handler->GetName()] = std::move(handler);
}

void AllDevicesAppCommandDelegate::RegisterCommandHandlers()
{
    RegisterCommandHandler(std::make_unique<IncreaseConfigurationVersionCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetOccupancyCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetHoldTimeCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetBooleanStateCommandHandler>());
    RegisterCommandHandler(std::make_unique<SetOnOffCommandHandler>());

    RegisterCommandHandler(std::make_unique<SimulateBindingOnOffCommandHandler>());
    RegisterCommandHandler(std::make_unique<SimulateBindingToggleCommandHandler>());
    RegisterCommandHandler(std::make_unique<SimulateBindingMoveToLevelCommandHandler>());
    RegisterCommandHandler(std::make_unique<SimulateBindingMoveCommandHandler>());
    RegisterCommandHandler(std::make_unique<SimulateBindingStepCommandHandler>());
    RegisterCommandHandler(std::make_unique<SimulateBindingStopCommandHandler>());
}
