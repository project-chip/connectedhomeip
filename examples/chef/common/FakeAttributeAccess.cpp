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

#include "FakeAttributeAccess.h"

#include <optional>

#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <pigweed/rpc_services/AccessInterceptor.h>
#include <pigweed/rpc_services/AccessInterceptorRegistry.h>

#if MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/temperature-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/valve-configuration-and-control-server/CodegenIntegration.h>
#endif

namespace chip {
namespace app {
namespace Clusters {
namespace Chef {
/**
 * Once clusters are converted to code driven, fake attribute access through datamodel provider
 * will be blocked. Therefore we make this attribute accessor to allow fake attribute access for
 * code driven apps.
 */
class AttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor
{
public:
    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        ChipLogProgress(Zcl, "Inside AttributeAccessor::Write for Cluster: " ChipLogFormatMEI " , Attribute: " ChipLogFormatMEI,
                        ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        switch (path.mClusterId)
        {
#if MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case TemperatureMeasurement::Id:
            switch (path.mAttributeId)
            {
            case TemperatureMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<int16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = TemperatureMeasurement::SetMeasuredValue(path.mEndpointId, measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (measuredValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to %d.", measuredValue.Value());
                }
                return ::pw::OkStatus();
            }
            }
#endif // MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case ValveConfigurationAndControl::Id:
            switch (path.mAttributeId)
            {
            case ValveConfigurationAndControl::Attributes::CurrentLevel::Id:
                Percent level;
                CHIP_ERROR err = decoder.Decode(level);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode currentLevel: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = ValveConfigurationAndControl::UpdateCurrentLevel(path.mEndpointId, level);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to update currentLevel: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                ChipLogProgress(Zcl, "[Pw] Successfully set current level to " ChipLogFormatMEI ".", ChipLogValueMEI(level));
            }
#endif
        }
        return std::nullopt;
    }
};

namespace {
static AttributeAccessor gAttributeAccessor;
} // namespace

void RegisterAttributeAccessor()
{
    chip::rpc::PigweedDebugAccessInterceptorRegistry::Instance().Register(&gAttributeAccessor);
}

} // namespace Chef
} // namespace Clusters
} // namespace app
} // namespace chip
