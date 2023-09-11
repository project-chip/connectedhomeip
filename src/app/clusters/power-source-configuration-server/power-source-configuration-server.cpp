/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

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

constexpr uint8_t kMaxPowerSources = 6;

class PowerSourceConfigurationAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceConfigurationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>(0), PowerSourceConfiguration::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

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
            std::pair<uint16_t, uint8_t> orderEpPair[kMaxPowerSources];
            uint8_t idx = 0;
            for (auto endpoint : EnabledEndpointsWithServerCluster(PowerSource::Id))
            {
                uint8_t order = 0;
                if (idx >= kMaxPowerSources)
                    break;
                PowerSource::Attributes::Order::Get(endpoint, &order);
                orderEpPair[idx] = std::make_pair(endpoint, order);
                idx++;
            }

            for (int i = 0; i < idx - 1; i++)
            {
                for (int j = 0; j < idx - i - 1; j++)
                    if ((orderEpPair[j]).second > (orderEpPair[j + 1]).second)
                        swap(orderEpPair[j], orderEpPair[j + 1]);
            }
            for (int k = 0; k < idx; k++)
            {
                ReturnErrorOnFailure(encoder.Encode(orderEpPair[k].first));
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

void MatterPowerSourceConfigurationPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
