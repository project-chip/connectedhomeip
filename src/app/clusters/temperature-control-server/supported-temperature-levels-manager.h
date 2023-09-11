/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

/**
 * Interface to help manage the supported temperature levels of the Temperature Control Cluster.
 */
class SupportedTemperatureLevelsIteratorDelegate
{
public:
    virtual ~SupportedTemperatureLevelsIteratorDelegate() = default;

    SupportedTemperatureLevelsIteratorDelegate() {}

    void Reset(EndpointId endpoint)
    {
        mEndpoint = endpoint;
        mIndex    = 0;
    }

    // Returns total size of SupportedTemperatureLevels list.
    virtual uint8_t Size() = 0;

    virtual CHIP_ERROR Next(MutableCharSpan & item) = 0;

protected:
    EndpointId mEndpoint;
    uint8_t mIndex;
};

SupportedTemperatureLevelsIteratorDelegate * GetInstance();

void SetInstance(SupportedTemperatureLevelsIteratorDelegate * instance);

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
