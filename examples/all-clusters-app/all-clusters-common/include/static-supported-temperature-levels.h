/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/util/af.h>
#include <zap-generated/gen_config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

/**
 * This implementation statically defines the options.
 */

class AppSupportedTemperatureLevelsDelegate : public SupportedTemperatureLevelsIteratorDelegate
{
    struct EndpointPair
    {
        EndpointId mEndpointId;
        CharSpan * mTemperatureLevels;
        uint8_t mSize;

        EndpointPair(EndpointId aEndpointId, CharSpan * aTemperatureLevels, uint8_t aSize) :
            mEndpointId(aEndpointId), mTemperatureLevels(aTemperatureLevels), mSize(aSize)
        {}

        ~EndpointPair() {}
    };

    static CharSpan temperatureLevelOptions[3];

public:
    static const EndpointPair supportedOptionsByEndpoints[EMBER_AF_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

    uint8_t Size() override;

    CHIP_ERROR Next(MutableCharSpan & item) override;

    ~AppSupportedTemperatureLevelsDelegate() {}
};

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
