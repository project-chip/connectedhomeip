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
using namespace chip::app::Clusters::MeterIdentification::Structs;

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

CHIP_ERROR MeterIdentificationDelegate::SetMeterType(DataModel::Nullable<MeterTypeEnum> newValue)
{
    DataModel::Nullable<MeterTypeEnum> oldValue = mMeterType;

    mMeterType = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, MeterType::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetCustomerName(CharSpan &newValue)
{
    CharSpan oldValue = mCustomerName;

    mCustomerName = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, CustomerName::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetUtilityName(CharSpan &newValue)
{
    CharSpan oldValue = mUtilityName;

    mUtilityName = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, UtilityName::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPointOfDelivery(CharSpan &newValue)
{
    CharSpan oldValue = mPointOfDelivery;

    mPointOfDelivery = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PointOfDelivery::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPowerThreshold(DataModel::Nullable<uint64_t> newValue)
{
    DataModel::Nullable<uint64_t> oldValue = mPowerThreshold;

    mPowerThreshold = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThreshold::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MeterIdentificationDelegate::SetPowerThresholdSource(DataModel::Nullable<PowerThresholdSourceEnum> newValue)
{
    DataModel::Nullable<PowerThresholdSourceEnum> oldValue = mPowerThresholdSource;

    mPowerThresholdSource = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, MeterIdentification::Id, PowerThresholdSource::Id);
    }

    return CHIP_NO_ERROR;
}
