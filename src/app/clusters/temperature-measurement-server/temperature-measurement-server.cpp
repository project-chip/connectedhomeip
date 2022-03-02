/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Temperature Measurement Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureMeasurement;
using namespace chip::app::Clusters::TemperatureMeasurement::Attributes;

constexpr int16_t kMinMeasuredValueLowerLimit = -27315; // -273.15°C
constexpr int16_t kMinMeasuredValueUpperLimit = 32766;  // 327.66°C
constexpr int16_t kMaxMeasuredValueLowerLimit = -27314; // -273.14°C
constexpr int16_t kMaxMeasuredValueUpperLimit = 32767;  // 327.67°C
constexpr int16_t kMaxToleranceValue          = 2048;

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

using Status = Protocols::InteractionModel::Status;

Protocols::InteractionModel::Status MatterTemperatureMeasurementClusterServerPreAttributeChangedCallback(
    const app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    EndpointId endpoint = attributePath.mEndpointId;
    int16_t requested;

    DataModel::Nullable<int16_t> MinMeasuredValue;
    DataModel::Nullable<int16_t> MaxMeasuredValue;

    if (MeasuredValue::Get(endpoint, MinMeasuredValue) != EMBER_ZCL_STATUS_SUCCESS)
    {
        MinMeasuredValue.SetNull();
    }

    if (MeasuredValue::Get(endpoint, MaxMeasuredValue) != EMBER_ZCL_STATUS_SUCCESS)
    {
        MaxMeasuredValue.SetNull();
    }

    switch (attributePath.mAttributeId)
    {
    case MeasuredValue::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));

        if (requested < (MinMeasuredValue.IsNull() ? kMinMeasuredValueLowerLimit : MinMeasuredValue.Value()))
        {
            return Status::InvalidValue;
        }
        else if (requested > (MaxMeasuredValue.IsNull() ? kMaxMeasuredValueUpperLimit : MaxMeasuredValue.Value()))
        {
            return Status::InvalidValue;
        }

        return Status::Success;
    }
    case MinMeasuredValue::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));

        if (requested < kMinMeasuredValueLowerLimit || requested > kMinMeasuredValueUpperLimit)
        {
            return Status::InvalidValue;
        }

        return Status::Success;
    }
    case MaxMeasuredValue::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));

        if (requested < kMaxMeasuredValueLowerLimit || requested > kMaxMeasuredValueUpperLimit)
        {
            return Status::InvalidValue;
        }

        return Status::Success;
    }
    case Tolerance::Id: {
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));

        if (requested < 0 || requested > kMaxToleranceValue)
        {
            return Status::InvalidValue;
        }

        return Status::Success;
    }
    default:
        return Status::Success;
    }
}
