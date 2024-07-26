/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include "include/thermostat-manager.h"
#include "include/thermostat-delegate-impl.h"

#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/thermostat-server/thermostat-server.h>
#include <controller/ReadInteraction.h>
#include <platform/PlatformManager.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::Controller;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::TemperatureMeasurement;
using namespace chip::app::Clusters::TemperatureMeasurement::Attributes;

using namespace chip::DeviceLayer;

static constexpr EndpointId kThermostatEndpoint = 1;

static constexpr uint16_t kMaxIntervalCeilingSeconds = 3600;

static const char * SystemModeString(SystemModeEnum systemMode);
static const char * RunningModeString(ThermostatRunningModeEnum runningMode);

/**********************************************************
 * Variable declarations
 *********************************************************/

ThermostatManager ThermostatManager::sThermostatMgr;

namespace {

template <typename DecodableAttributeType>
static void OnAttributeChangeReported(const ConcreteDataAttributePath & path, const DecodableAttributeType & value);

template <>
void OnAttributeChangeReported<MeasuredValue::TypeInfo::DecodableType>(const ConcreteDataAttributePath & path,
                                                                       const MeasuredValue::TypeInfo::DecodableType & value)
{
    ClusterId clusterId = path.mClusterId;
    if (clusterId != TemperatureMeasurement::Id)
    {
        ChipLogError(AppServer, "Attribute change reported for TemperatureMeasurement cluster on incorrect cluster id %u",
                     clusterId);
        return;
    }

    AttributeId attributeId = path.mAttributeId;
    if (attributeId != MeasuredValue::Id)
    {
        ChipLogError(AppServer, "Attribute change reported for TemperatureMeasurement cluster for incorrect attribute %u",
                     attributeId);
        return;
    }

    if (!value.IsNull())
    {
        ChipLogDetail(AppServer, "Attribute change reported for TemperatureMeasurement cluster - MeasuredValue is %d",
                      static_cast<short>(value.Value()));
    }
}

static void OnError(const ConcreteDataAttributePath * path, ChipError err)
{
    ChipLogError(AppServer, "Subscribing to cluster Id %u and attribute Id %u failed with error %" CHIP_ERROR_FORMAT,
                 path->mClusterId, path->mAttributeId, err.Format());
}

static void OnSubscriptionEstablished(const ReadClient & client, unsigned int value)
{
    ChipLogDetail(AppServer, "OnSubscriptionEstablished with subscription Id: %d", value);
}

template <typename DecodableAttributeType>
void SubscribeToAttribute(ClusterId clusterId, AttributeId attributeId, const EmberBindingTableEntry & binding,
                          OperationalDeviceProxy * peer_device)
{
    VerifyOrReturn(peer_device->GetSecureSession().HasValue(),
                   ChipLogError(AppServer, "SubscribeToAttribute failed. Secure session is null"));

    SubscribeAttribute<DecodableAttributeType>(
        peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, clusterId, attributeId,
        &OnAttributeChangeReported<DecodableAttributeType>, &OnError, 0, kMaxIntervalCeilingSeconds, &OnSubscriptionEstablished,
        nullptr, true /* fabricFiltered */, true /* keepExistingSubscription */);
}

static void ThermostatBoundDeviceChangedHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * peer_device,
                                                void * context)
{
    VerifyOrReturn(binding.clusterId.has_value(), ChipLogError(AppServer, "Cluster Id is null"));
    ClusterId clusterId = binding.clusterId.value();

    switch (clusterId)
    {
    case TemperatureMeasurement::Id:

        // Subscribe to the MeasuredValue attribute
        SubscribeToAttribute<MeasuredValue::TypeInfo::DecodableType>(clusterId, MeasuredValue::Id, binding, peer_device);
        break;
    default:
        ChipLogError(AppServer, "Unsupported Cluster Id");
        break;
    }
}

void NotifyBoundClusterChangedForAllClusters()
{
    BindingManager::GetInstance().NotifyBoundClusterChanged(kThermostatEndpoint, TemperatureMeasurement::Id, nullptr);
}

static void OnPlatformChipDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kBindingsChangedViaCluster)
    {
        NotifyBoundClusterChangedForAllClusters();
    }
}

void InitBindingManager(intptr_t context)
{
    auto & server    = Server::GetInstance();
    CHIP_ERROR error = BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to init binding manager");
    }

    BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(ThermostatBoundDeviceChangedHandler);
    NotifyBoundClusterChangedForAllClusters();
}

} // anonymous namespace

CHIP_ERROR ThermostatManager::Init()
{
    // Init binding manager

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformChipDeviceEvent, reinterpret_cast<intptr_t>(this));
    DeviceLayer::PlatformMgr().ScheduleWork(InitBindingManager);

    mLocalTemperature        = GetCurrentTemperature();
    mSystemMode              = GetSystemMode();
    mRunningMode             = GetRunningMode();
    mOccupiedCoolingSetpoint = GetCurrentCoolingSetPoint();
    mOccupiedHeatingSetpoint = GetCurrentHeatingSetPoint();
    // TODO: Gotta expose this properly on attribute
    mOccupiedSetback = 5; // 0.5 C

    ChipLogError(AppServer,
                 "Initialized a thermostat with \n "
                 "mSystemMode: %u (%s) \n mRunningMode: %u (%s) \n mLocalTemperature: %d \n mOccupiedHeatingSetpoint: %d \n "
                 "mOccupiedCoolingSetpoint: %d"
                 "NumberOfPresets: %d",
                 static_cast<uint8_t>(mSystemMode), SystemModeString(mSystemMode), static_cast<uint8_t>(mRunningMode),
                 RunningModeString(mRunningMode), mLocalTemperature, mOccupiedHeatingSetpoint, mOccupiedCoolingSetpoint,
                 GetNumberOfPresets());

    // TODO: Should this be called later?
    EvalThermostatState();

    return CHIP_NO_ERROR;
}

void ThermostatManager::AttributeChangeHandler(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t * value,
                                               uint16_t size)
{
    switch (endpointId)
    {
    case kThermostatEndpoint:
        ThermostatEndpointAttributeChangeHandler(clusterId, attributeId, value, size);
        break;

    default:
        ChipLogError(AppServer, "Attribute change reported for Thermostat on incorrect endpoint. Ignoring.");
        break;
    }
}

void ThermostatManager::ThermostatEndpointAttributeChangeHandler(ClusterId clusterId, AttributeId attributeId, uint8_t * value,
                                                                 uint16_t size)
{
    switch (clusterId)
    {
    case Thermostat::Id:
        ThermostatClusterAttributeChangeHandler(attributeId, value, size);
        break;

    default:
        ChipLogError(AppServer,
                     "Attribute change reported for Thermostat on incorrect cluster for the thermostat endpoint. Ignoring.");
        break;
    }
}

void ThermostatManager::ThermostatClusterAttributeChangeHandler(AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
    case LocalTemperature::Id: {
        mLocalTemperature = static_cast<int16_t>(Encoding::LittleEndian::Get16(value));
        ChipLogError(AppServer, "Local temperature changed to %d", mLocalTemperature);
        EvalThermostatState();
    }
    break;

    case OccupiedCoolingSetpoint::Id: {
        mOccupiedCoolingSetpoint = static_cast<int16_t>(Encoding::LittleEndian::Get16(value));
        ChipLogError(AppServer, "Cooling temperature changed to %d", mOccupiedCoolingSetpoint);
        EvalThermostatState();
    }
    break;

    case OccupiedHeatingSetpoint::Id: {
        mOccupiedHeatingSetpoint = static_cast<int16_t>(Encoding::LittleEndian::Get16(value));
        ChipLogError(AppServer, "Heating temperature changed to %d", mOccupiedHeatingSetpoint);
        EvalThermostatState();
    }
    break;

    case SystemMode::Id: {
        mSystemMode = static_cast<SystemModeEnum>(*value);
        ChipLogError(AppServer, "System mode changed to %u (%s)", *value, SystemModeString(mSystemMode));
        EvalThermostatState();
    }
    break;

    case ThermostatRunningMode::Id: {
        mRunningMode = static_cast<ThermostatRunningModeEnum>(*value);
        ChipLogError(AppServer, "Running mode changed to %u (%s)", *value, RunningModeString(mRunningMode));
    }
    break;

    default: {
        ChipLogError(AppServer, "Unhandled thermostat attribute %x", attributeId);
        return;
    }
    break;
    }
}

SystemModeEnum ThermostatManager::GetSystemMode()
{
    SystemModeEnum systemMode;
    SystemMode::Get(kThermostatEndpoint, &systemMode);
    return systemMode;
}

ThermostatRunningModeEnum ThermostatManager::GetRunningMode()
{
    ThermostatRunningModeEnum runningMode;
    ThermostatRunningMode::Get(kThermostatEndpoint, &runningMode);
    return runningMode;
}

int16_t ThermostatManager::GetCurrentTemperature()
{
    DataModel::Nullable<int16_t> currentTemperature;
    currentTemperature.SetNull();
    LocalTemperature::Get(kThermostatEndpoint, currentTemperature);
    return currentTemperature.ValueOr(0);
}

int16_t ThermostatManager::GetCurrentHeatingSetPoint()
{
    int16_t heatingSetpoint;
    OccupiedHeatingSetpoint::Get(kThermostatEndpoint, &heatingSetpoint);
    return heatingSetpoint;
}

int16_t ThermostatManager::GetCurrentCoolingSetPoint()
{
    int16_t coolingSetpoint;
    OccupiedCoolingSetpoint::Get(kThermostatEndpoint, &coolingSetpoint);
    return coolingSetpoint;
}

uint8_t ThermostatManager::GetNumberOfPresets()
{
    return ThermostatDelegate::GetInstance().GetNumberOfPresets();
}

CHIP_ERROR ThermostatManager::SetSystemMode(SystemModeEnum systemMode)
{
    uint8_t systemModeValue = static_cast<uint8_t>(systemMode);
    if (mSystemMode == systemMode)
    {
        ChipLogDetail(AppServer, "Already in system mode: %u (%s)", systemModeValue, SystemModeString(systemMode));
        return CHIP_NO_ERROR;
    }

    ChipLogError(AppServer, "Setting system mode: %u (%s)", systemModeValue, SystemModeString(systemMode));
    Protocols::InteractionModel::Status status = SystemMode::Set(kThermostatEndpoint, systemMode);

    // TODO: CHIP_ERROR_WRITE_FAILED might not be the best error code to send
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatManager::SetRunningMode(ThermostatRunningModeEnum runningMode)
{
    uint8_t runningModeValue = static_cast<uint8_t>(runningMode);
    if (mRunningMode == runningMode)
    {
        ChipLogDetail(AppServer, "Already in running mode: %u (%s)", runningModeValue, RunningModeString(runningMode));
        return CHIP_NO_ERROR;
    }

    ChipLogError(AppServer, "Setting running mode: %u (%s)", runningModeValue, RunningModeString(runningMode));
    Protocols::InteractionModel::Status status = ThermostatRunningMode::Set(kThermostatEndpoint, runningMode);

    // TODO: CHIP_ERROR_WRITE_FAILED might not be the best error code to send
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatManager::SetCurrentTemperature(int16_t temperature)
{
    Protocols::InteractionModel::Status status = LocalTemperature::Set(kThermostatEndpoint, temperature);
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatManager::SetCurrentHeatingSetPoint(int16_t heatingSetpoint)
{
    Protocols::InteractionModel::Status status = OccupiedHeatingSetpoint::Set(kThermostatEndpoint, heatingSetpoint);
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatManager::SetCurrentCoolingSetPoint(int16_t coolingSetpoint)
{
    Protocols::InteractionModel::Status status = OccupiedCoolingSetpoint::Set(kThermostatEndpoint, coolingSetpoint);
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

void ThermostatManager::EvalThermostatState()
{
    ChipLogError(AppServer,
                 "Eval Thermostat Running Mode \n "
                 "mSystemMode: %u (%s) \n mRunningMode: %u (%s) \n mLocalTemperature: %d \n mOccupiedHeatingSetpoint: %d \n "
                 "mOccupiedCoolingSetpoint: %d",
                 static_cast<uint8_t>(mSystemMode), SystemModeString(mSystemMode), static_cast<uint8_t>(mRunningMode),
                 RunningModeString(mRunningMode), mLocalTemperature, mOccupiedHeatingSetpoint, mOccupiedCoolingSetpoint);

    switch (mSystemMode)
    {
    case SystemModeEnum::kOff: {
        SetRunningMode(ThermostatRunningModeEnum::kOff);
        break;
    }
    case SystemModeEnum::kHeat: {
        UpdateRunningModeForHeating();
        break;
    }
    case SystemModeEnum::kCool: {
        UpdateRunningModeForCooling();
        break;
    }
    case SystemModeEnum::kAuto: {
        UpdateRunningModeForHeating();
        UpdateRunningModeForCooling();
        break;
    }
    default:
        break;
    }
}

void ThermostatManager::UpdateRunningModeForHeating()
{
    const int16_t heatingOnThreshold  = mOccupiedHeatingSetpoint - mOccupiedSetback * 10;
    const int16_t heatingOffThreshold = mOccupiedHeatingSetpoint + mOccupiedSetback * 10;

    if (mRunningMode == ThermostatRunningModeEnum::kHeat)
    {
        if (mLocalTemperature >= heatingOffThreshold)
        {
            ChipLogDetail(AppServer, "Eval Heat - Turning off");
            SetRunningMode(ThermostatRunningModeEnum::kOff);
        }
        else
        {
            ChipLogDetail(AppServer, "Eval Heat - Keep Heating");
        }
    }
    else
    {
        if (mLocalTemperature <= heatingOnThreshold)
        {
            ChipLogDetail(AppServer, "Eval Heat - Turn on");
            SetRunningMode(ThermostatRunningModeEnum::kHeat);
        }
        else
        {
            ChipLogDetail(AppServer, "Eval Heat - Nothing to do");
        }
    }
}

void ThermostatManager::UpdateRunningModeForCooling()
{
    const int16_t coolingOffThreshold = mOccupiedCoolingSetpoint - mOccupiedSetback * 10;
    const int16_t coolingOnThreshold  = mOccupiedCoolingSetpoint + mOccupiedSetback * 10;

    if (mRunningMode == ThermostatRunningModeEnum::kCool)
    {
        if (mLocalTemperature <= coolingOffThreshold)
        {
            ChipLogDetail(AppServer, "Eval Cool - Turning off");
            SetRunningMode(ThermostatRunningModeEnum::kOff);
        }
        else
        {
            ChipLogDetail(AppServer, "Eval Cool - Keep Cooling");
        }
    }
    else
    {
        if (mLocalTemperature >= coolingOnThreshold)
        {
            ChipLogDetail(AppServer, "Eval Cool - Turn on");
            SetRunningMode(ThermostatRunningModeEnum::kCool);
        }
        else
        {
            ChipLogDetail(AppServer, "Eval Cool - Nothing to do");
        }
    }
}

static const char * SystemModeString(SystemModeEnum systemMode)
{
    switch (systemMode)
    {
    case SystemModeEnum::kOff:
        return "Off";
    case SystemModeEnum::kAuto:
        return "Auto";
    case SystemModeEnum::kCool:
        return "Cool";
    case SystemModeEnum::kHeat:
        return "Heat";
    default:
        return "Unknown";
    }
}

static const char * RunningModeString(ThermostatRunningModeEnum runningMode)
{
    switch (runningMode)
    {
    case ThermostatRunningModeEnum::kOff:
        return "Off";
    case ThermostatRunningModeEnum::kCool:
        return "Cool";
    case ThermostatRunningModeEnum::kHeat:
        return "Heat";
    default:
        return "Unknown";
    }
}

void MatterPostAttributeChangeCallback(const ConcreteAttributePath & attributePath, uint8_t type, uint16_t size, uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(AppServer, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    ChipLogProgress(AppServer,
                    "Attribute ID changed: " ChipLogFormatMEI " Endpoint: %d ClusterId: %d Type: %u Value: %u, length %u",
                    ChipLogValueMEI(attributeId), attributePath.mEndpointId, clusterId, type, *value, size);

    ThermostatMgr().AttributeChangeHandler(attributePath.mEndpointId, clusterId, attributeId, value, size);
}

void emberAfThermostatClusterInitCallback(EndpointId endpoint)
{
    // Register the delegate for the Thermostat
    auto & delegate = ThermostatDelegate::GetInstance();

    // Set the default delegate for endpoint kThermostatEndpoint.
    VerifyOrDie(endpoint == kThermostatEndpoint);
    SetDefaultDelegate(endpoint, &delegate);
}
