/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/resource-monitoring-server/replacement-product-list-manager.h>
#include <app/util/af.h>
#include <app/util/config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

/**
 * This implementation statically defines the options.
 */

class StaticReplacementProductListManager : public ReplacementProductListManager
{
public:
    CHIP_ERROR Next(ReplacementProductStruct & item) override;

    ~StaticReplacementProductListManager() {}
    StaticReplacementProductListManager(ReplacementProductStruct * aReplacementProductsList, uint8_t aReplacementProductListSize)
    {
        mReplacementProductsList    = aReplacementProductsList;
        mReplacementProductListSize = aReplacementProductListSize;
    }

private:
    ReplacementProductStruct * mReplacementProductsList;
    uint8_t mReplacementProductListSize;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
