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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

#include <app/util/af.h>
#include <app/util/common.h>
#include <array>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    switch (attributePath.mClusterId)
    {
    case Clusters::IlluminanceMeasurement::Id:
        if (attributePath.mAttributeId == Clusters::IlluminanceMeasurement::Attributes::MinMeasuredValue::Id)
        {
            uint16_t min = 0;
            if (EMBER_ZCL_STATUS_SUCCESS ==
                Clusters::IlluminanceMeasurement::Attributes::MinMeasuredValue::Get(attributePath.mEndpointId, &min))
            {
                min = min < 10 ? 10 : min;
                min = min > 0xfffd ? 0xfffd : min;
                Clusters::IlluminanceMeasurement::Attributes::MinMeasuredValue::Set(attributePath.mEndpointId, min);
            }
        }
        break;
    default:
        break;
    }
}
