/*
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

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSourceConfiguration::Attributes;
using namespace chip::app::Clusters::PowerSource;

namespace {

// Can be upto max endpoints count
constexpr uint8_t kMaxPowerSources = 10;

class PowerSourceConfigurationAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceConfigurationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>(0), PowerSourceConfiguration::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

int compareOrder(const void * a, const void * b)
{
    return (int) ((*(uint16_t *) a - *(uint16_t *) b));
}

PowerSourceConfigurationAttrAccess gAttrAccess;

CHIP_ERROR PowerSourceConfigurationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPath.mClusterId != PowerSourceConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case Sources::Id:
        err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
            uint16_t orderEpPair[kMaxPowerSources * 2];
            uint8_t idx = 0, order = 0;
            memset(orderEpPair, 0, sizeof(orderEpPair));
            for (auto endpoint : EnabledEndpointsWithServerCluster(PowerSource::Id))
            {
                if (idx >= sizeof(orderEpPair))
                    break;
                PowerSource::Attributes::Order::Get(endpoint, &order);
                orderEpPair[idx]   = (uint16_t) order;
                orderEpPair[++idx] = (uint16_t) endpoint;
                idx++;
            }

            std::qsort(orderEpPair, idx / 2, sizeof(uint16_t) * 2, compareOrder);
            for (uint8_t i = 0; i < idx; i += 2)
            {
                ReturnErrorOnFailure(encoder.Encode((uint16_t) orderEpPair[i + 1]));
            }
            return CHIP_NO_ERROR;
        });
        break;
    default:
        break;
    }

    return err;
}

} // anonymous namespace

void MatterPowerSourceConfigurationPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
