/*
 *
 * Copyright (c) 2025 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "chef-microwave-oven-control.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h> // Defines Attributes namespace
#include <app-common/zap-generated/ids/Clusters.h>   // Defines Cluster Id
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory> // For std::unique_ptr
#include <platform/CHIPDeviceLayer.h>

// Style based on tcc-mode.cpp
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl; // Brings MicrowaveOvenControlServer and MicrowaveOvenControl::Id into scope
// We will qualify Attributes:: like chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Id
// or use Attributes::CookPower::Id if the context is already within MicrowaveOvenControl namespace for methods.
using chip::Protocols::InteractionModel::Status;

static std::unique_ptr<MicrowaveOvenControlServer> gMicrowaveOvenControlServerInstance;

CHIP_ERROR MicrowaveOvenControlServer::Init()
{
    const EndpointId kFixedChefEndpoint = 1;
    mEndpointId                         = kFixedChefEndpoint;

    mMinCookPower  = 10;
    mMaxCookPower  = 100;
    mCookPowerStep = 10;

    Status status;

    status = chip::app::Clusters::MicrowaveOvenControl::Attributes::MinCookPower::Set(mEndpointId, mMinCookPower);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "MicrowaveControl: Failed to set MinCookPower in ZAP for ep %u, status:0x%x", mEndpointId,
                        to_underlying(status));
    }

    status = chip::app::Clusters::MicrowaveOvenControl::Attributes::MaxCookPower::Set(mEndpointId, mMaxCookPower);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "MicrowaveControl: Failed to set MaxCookPower in ZAP for ep %u, status:0x%x", mEndpointId,
                        to_underlying(status));
    }

    status = chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPowerStep::Set(mEndpointId, mCookPowerStep);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "MicrowaveControl: Failed to set CookPowerStep in ZAP for ep %u, status:0x%x", mEndpointId,
                        to_underlying(status));
    }

    uint16_t currentCookPower;
    status = chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Get(mEndpointId, &currentCookPower);
    if (status != Status::Success || currentCookPower < mMinCookPower || currentCookPower > mMaxCookPower)
    {
        ChipLogProgress(Zcl, "MicrowaveControl: Initial CookPower for ep %u is invalid or not set (status:0x%x), defaulting to %u",
                        mEndpointId, to_underlying(status), mMinCookPower);
        chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Set(mEndpointId, mMinCookPower);
    }

    uint8_t currentPowerSetting;
    status = chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Get(mEndpointId, &currentPowerSetting);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "MicrowaveControl: Initial PowerSetting for ep %u not found (status:0x%x), defaulting to 0 (LOW)",
                        mEndpointId, to_underlying(status));
        chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Set(mEndpointId, 0);
    }

    ChipLogProgress(Zcl, "Chef MicrowaveOvenControlServer::Init complete for endpoint %u. MinP:%u MaxP:%u Step:%u", mEndpointId,
                    mMinCookPower, mMaxCookPower, mCookPowerStep);
    return CHIP_NO_ERROR;
}

uint32_t MicrowaveOvenControlServer::GetFeatureMap(EndpointId endpoint)
{
    VerifyOrReturnValue(endpoint == mEndpointId, 0);
    uint32_t featureMapValue = 0;
    Status status = chip::app::Clusters::MicrowaveOvenControl::Attributes::FeatureMap::Get(endpoint, &featureMapValue);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to read FeatureMap for ep %u: status 0x%x. Will use calculated fallback.",
                     endpoint, to_underlying(status));
        featureMapValue = (to_underlying(chip::app::Clusters::MicrowaveOvenControl::Features::PowerNumeric) |
                           to_underlying(chip::app::Clusters::MicrowaveOvenControl::Features::PowerLimits));
    }
    return featureMapValue;
}

CHIP_ERROR MicrowaveOvenControlServer::GetCookPower(uint16_t & value)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    Status status = chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Get(mEndpointId, &value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to read CookPower for ep %u: status 0x%x", mEndpointId, to_underlying(status));
        return CHIP_ERROR_READ_FAILED;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MicrowaveOvenControlServer::GetPowerSetting(uint8_t & value)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    Status status = chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Get(mEndpointId, &value);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to read PowerSetting for ep %u: status 0x%x", mEndpointId,
                     to_underlying(status));
        return CHIP_ERROR_READ_FAILED;
    }
    return CHIP_NO_ERROR;
}

Status MicrowaveOvenControlServer::SetCookPower(uint16_t cookPower)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, Status::Failure);

    if (cookPower < mMinCookPower || cookPower > mMaxCookPower)
    {
        ChipLogDetail(Zcl, "MicrowaveControl: SetCookPower value %u (ep %u) is out of range [%u, %u]", cookPower, mEndpointId,
                      mMinCookPower, mMaxCookPower);
        return Status::ConstraintError;
    }

    if (mCookPowerStep != 0 && cookPower != 0)
    {
        if (cookPower != mMinCookPower && (cookPower < mMinCookPower || ((cookPower - mMinCookPower) % mCookPowerStep != 0)))
        {
            ChipLogDetail(Zcl, "MicrowaveControl: SetCookPower value %u (ep %u) does not align with step %u from min %u",
                          cookPower, mEndpointId, mCookPowerStep, mMinCookPower);
            return Status::ConstraintError;
        }
    }

    Status status = chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Set(mEndpointId, cookPower);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to write CookPower for ep %u: status 0x%x", mEndpointId,
                     to_underlying(status));
    }
    else
    {
        ChipLogProgress(Zcl, "MicrowaveControl: SetCookPower to %u for endpoint %u", cookPower, mEndpointId);
    }
    return status;
}

Status MicrowaveOvenControlServer::SetPowerSetting(uint8_t powerSetting)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, Status::Failure);

    Status status = chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Set(mEndpointId, powerSetting);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to write PowerSetting for ep %u: status 0x%x", mEndpointId,
                     to_underlying(status));
    }
    else
    {
        ChipLogProgress(Zcl, "MicrowaveControl: SetPowerSetting to %u for endpoint %u", powerSetting, mEndpointId);
    }
    return status;
}

void chip::app::Clusters::MicrowaveOvenControl::Shutdown() // Qualify with full namespace
{
    if (gMicrowaveOvenControlServerInstance)
    {
        gMicrowaveOvenControlServerInstance.reset();
        ChipLogProgress(Zcl, "MicrowaveControl: Shutdown complete.");
    }
}


void emberAfMicrowaveOvenControlClusterServerInitCallback(chip::EndpointId endpointId)
{
    const EndpointId kChefMicrowaveEndpoint = 1;
    VerifyOrDie(endpointId == kChefMicrowaveEndpoint);
    VerifyOrDie(gMicrowaveOvenControlServerInstance == nullptr);

    ChipLogProgress(Zcl, "MicrowaveControl: Initializing ServerInstance for endpoint %u via ZAP InitCallback", endpointId);

    gMicrowaveOvenControlServerInstance = std::make_unique<MicrowaveOvenControlServer>();
    VerifyOrDie(gMicrowaveOvenControlServerInstance != nullptr);

    CHIP_ERROR err = gMicrowaveOvenControlServerInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MicrowaveControl: Failed to initialize ServerInstance for ep %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
        gMicrowaveOvenControlServerInstance.reset();
    }
}

Status chefMicrowaveOvenControlReadAttributeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                     const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                     uint16_t maxReadLength)
{
    const EndpointId kChefMicrowaveEndpoint = 1;
    VerifyOrReturnValue(endpointId == kChefMicrowaveEndpoint, Status::UnsupportedAccess);
    VerifyOrReturnValue(gMicrowaveOvenControlServerInstance != nullptr, Status::Failure);
    VerifyOrReturnValue(clusterId == chip::app::Clusters::MicrowaveOvenControl::Id, Status::UnsupportedCluster); // Use fully qualified Id
    VerifyOrReturnValue(attributeMetadata != nullptr && buffer != nullptr, Status::InvalidAction);

    ChipLogDetail(Zcl, "MicrowaveControl Read: EP:%u Clus:0x%X Attr:0x%X MaxLen:%u", endpointId, clusterId,
                  attributeMetadata->attributeId, maxReadLength);

    Status status         = Status::Success;
    CHIP_ERROR encodeError = CHIP_NO_ERROR;

    switch (attributeMetadata->attributeId)
    {
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Id: { // Fully qualify Attribute ID
        uint16_t value;
        CHIP_ERROR err = gMicrowaveOvenControlServerInstance->GetCookPower(value);
        if (err == CHIP_NO_ERROR)
        {
            encodeError = chip::app::DataModel::Encode(buffer, maxReadLength, value);
        }
        else
        {
            status = Status::Failure;
        }
        break;
    }
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Id: { // Fully qualify Attribute ID
        uint8_t value;
        CHIP_ERROR err = gMicrowaveOvenControlServerInstance->GetPowerSetting(value);
        if (err == CHIP_NO_ERROR)
        {
            encodeError = chip::app::DataModel::Encode(buffer, maxReadLength, value);
        }
        else
        {
            status = Status::Failure;
        }
        break;
    }
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::MinCookPower::Id: // Fully qualify Attribute ID
        encodeError = chip::app::DataModel::Encode(buffer, maxReadLength, gMicrowaveOvenControlServerInstance->GetMinCookPowerConstant());
        break;
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::MaxCookPower::Id: // Fully qualify Attribute ID
        encodeError = chip::app::DataModel::Encode(buffer, maxReadLength, gMicrowaveOvenControlServerInstance->GetMaxCookPowerConstant());
        break;
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPowerStep::Id: // Fully qualify Attribute ID
        encodeError = chip::app::DataModel::Encode(buffer, maxReadLength, gMicrowaveOvenControlServerInstance->GetCookPowerStepConstant());
        break;
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::FeatureMap::Id: { // Fully qualify Attribute ID
        uint32_t featureMapValue = gMicrowaveOvenControlServerInstance->GetFeatureMap(endpointId);
        encodeError              = chip::app::DataModel::Encode(buffer, maxReadLength, featureMapValue);
        break;
    }
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::ClusterRevision::Id: // Fully qualify Attribute ID
        encodeError = chip::app::DataModel::Encode(buffer, maxReadLength,
                                                   static_cast<uint16_t>(ZCL_MICROWAVE_OVEN_CONTROL_CLUSTER_REVISION));
        break;
    default:
        ChipLogDetail(Zcl, "MicrowaveControl Read: Unsupported attribute ID: %lu", attributeMetadata->attributeId);
        status = Status::UnsupportedAttribute;
        break;
    }

    if (status == Status::Success)
    {
        if (encodeError == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            status = Status::ResourceExhausted;
        }
        else if (encodeError != CHIP_NO_ERROR)
        {
            status = Status::Failure;
            ChipLogError(Zcl, "MicrowaveControl: Encode failed for Attr 0x%lX: %" CHIP_ERROR_FORMAT,
                         attributeMetadata->attributeId, encodeError.Format());
        }
    }

    if (status != Status::Success && status != Status::UnsupportedAttribute)
    {
        ChipLogDetail(Zcl, "MicrowaveControl Read: Failed for Attr:0x%lX, Status:0x%X", attributeMetadata->attributeId,
                      to_underlying(status));
    }
    return status;
}

Status chefMicrowaveOvenControlWriteAttributeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                      const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    const EndpointId kChefMicrowaveEndpoint = 1;
    VerifyOrReturnValue(endpointId == kChefMicrowaveEndpoint, Status::UnsupportedAccess);
    VerifyOrReturnValue(gMicrowaveOvenControlServerInstance != nullptr, Status::Failure);
    VerifyOrReturnValue(clusterId == chip::app::Clusters::MicrowaveOvenControl::Id, Status::UnsupportedCluster); // Use fully qualified Id
    VerifyOrReturnValue(attributeMetadata != nullptr && buffer != nullptr, Status::InvalidAction);

    ChipLogDetail(Zcl, "MicrowaveControl Write: EP:%u Clus:0x%X Attr:0x%X", endpointId, clusterId,
                  attributeMetadata->attributeId);

    Status status = Status::UnsupportedWrite;

    switch (attributeMetadata->attributeId)
    {
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::CookPower::Id: { // Fully qualify Attribute ID
        uint16_t value;
        CHIP_ERROR err = chip::app::DataModel::Decode(buffer, attributeMetadata->size, value);
        if (err == CHIP_NO_ERROR)
        {
            status = gMicrowaveOvenControlServerInstance->SetCookPower(value);
        }
        else
        {
            ChipLogError(Zcl, "MicrowaveControl Write: CookPower decode failed: %" CHIP_ERROR_FORMAT, err.Format());
            status = Status::InvalidValue;
        }
        break;
    }
    case chip::app::Clusters::MicrowaveOvenControl::Attributes::PowerSetting::Id: { // Fully qualify Attribute ID
        uint8_t value;
        CHIP_ERROR err = chip::app::DataModel::Decode(buffer, attributeMetadata->size, value);
        if (err == CHIP_NO_ERROR)
        {
            status = gMicrowaveOvenControlServerInstance->SetPowerSetting(value);
        }
        else
        {
            ChipLogError(Zcl, "MicrowaveControl Write: PowerSetting decode failed: %" CHIP_ERROR_FORMAT, err.Format());
            status = Status::InvalidValue;
        }
        break;
    }
    default:
        ChipLogDetail(Zcl, "MicrowaveControl Write: Unsupported or ReadOnly attribute ID: %lu", attributeMetadata->attributeId);
        break;
    }

    if (status != Status::Success && status != Status::UnsupportedWrite)
    {
        ChipLogDetail(Zcl, "MicrowaveControl Write: Failed for Attr:0x%lX, Status:0x%X", attributeMetadata->attributeId,
                      to_underlying(status));
    }
    return status;
}