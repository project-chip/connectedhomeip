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

#include <MeterIdentificationDelegate.h>
#include <app/reporting/reporting.h>

#include <app/clusters/meter-identification-server/meter-identification-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using namespace chip::app::Clusters::MeterIdentification::Attributes;

using chip::app::Clusters::MeterIdentification::MeterTypeEnum;
using chip::app::Clusters::MeterIdentification::PowerThresholdSourceEnum;

CHIP_ERROR MeterIdentificationInstance::Init()
{
    return Instance::Init();
}

void MeterIdentificationInstance::Shutdown()
{
    Instance::Shutdown();
}

// --------------- Internal Attribute Set APIs

void MeterIdentificationDelegate::Init()
{
    ChipLogProgress(Zcl, "MeterIdentificationDelegate::Init");

    SetMeterType(MeterTypeEnum::kPrivate);
    SetUtilityName(CharSpan::fromCharString("Test Utility Name"));
    SetPointOfDelivery(CharSpan::fromCharString("Test PointOfDelivery"));
    SetPowerThreshold(100);
    SetPowerThresholdSource(PowerThresholdSourceEnum::kEquipment);
}

CHIP_ERROR MeterIdentificationDelegate::LoadJson(Json::Value & root)
{
    Json::Value value = root.get("MeterType", Json::Value());
    if (!value.empty())
    {
        if(value.isInt())
        {
            SetMeterType(static_cast<MeterTypeEnum>(value.asInt()));
        }
        else
        {
            SetMeterType(std::nullopt);
        }
    }

    value = root.get("UtilityName", Json::Value());
    if (!value.empty())
    {
        if(value.isString())
        {
            SetUtilityName(CharSpan::fromCharString(value.asCString()));
        }
        else
        {
            SetUtilityName(CharSpan());
        }
    }

    value = root.get("PointOfDelivery", Json::Value());
    if (!value.empty())
    {
        if(value.isString())
        {
            SetPointOfDelivery(CharSpan::fromCharString(value.asCString()));
        }
        else
        {
            SetPointOfDelivery(CharSpan());
        }
    }

    value = root.get("PowerThreshold", Json::Value());
    if (!value.empty())
    {
        if(value.isInt())
        {
            SetPowerThreshold(value.asInt());
        }
        else
        {
            SetPowerThreshold(std::nullopt);
        }
    }

    value = root.get("PowerThresholdSource", Json::Value());
    if (!value.empty())
    {
        if(value.isInt())
        {
            SetPowerThresholdSource(static_cast<PowerThresholdSourceEnum>(value.asInt()));
        }
        else
        {
            SetPowerThresholdSource(std::nullopt);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetMeterType(DataModel::Nullable<MeterTypeEnum> newValue)
{
    // DataModel::Nullable<MeterTypeEnum> oldValue = mMeterType;

    mMeterType = newValue;
    // if (oldValue != newValue)
    //{
         MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterType::Id);
    // }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetUtilityName(CharSpan newValue)
{
    // CharSpan oldValue = mUtilityName;

    chip::Platform::MemoryFree((void*)mUtilityName.data());
    if (!newValue.empty())
    {
        size_t len = newValue.size();
        char *str = (char*)chip::Platform::MemoryAlloc(len);
        memcpy(str, newValue.data(), len);
        mUtilityName = CharSpan(str, len);
    }
    else
    {
        mUtilityName = CharSpan();
    }

    // if (!oldValue.data_equal(newValue))
    //{
    //     MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, UtilityName::Id);
    // }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPointOfDelivery(CharSpan newValue)
{
    // CharSpan oldValue = mPointOfDelivery;

    chip::Platform::MemoryFree((void*)mPointOfDelivery.data());
    if (!newValue.empty())
    {
        size_t len = newValue.size();
        char *str = (char*)chip::Platform::MemoryAlloc(len);
        memcpy(str, newValue.data(), len);
        mPointOfDelivery = CharSpan(str, len);
    }
    else
    {
        mPointOfDelivery = CharSpan();
    }

    // if (!oldValue.data_equal(newValue))
    //{
    //     MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PointOfDelivery::Id);
    // }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPowerThreshold(DataModel::Nullable<uint64_t> newValue)
{
    // DataModel::Nullable<uint64_t> oldValue = mPowerThreshold;

    mPowerThreshold = newValue;
    // if (oldValue != newValue)
    //{
    //     MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThreshold::Id);
    // }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPowerThresholdSource(DataModel::Nullable<PowerThresholdSourceEnum> newValue)
{
    // DataModel::Nullable<PowerThresholdSourceEnum> oldValue = mPowerThresholdSource;

    mPowerThresholdSource = newValue;
    // if (oldValue != newValue)
    //{
    //     MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThresholdSource::Id);
    // }

    return CHIP_NO_ERROR;
}
