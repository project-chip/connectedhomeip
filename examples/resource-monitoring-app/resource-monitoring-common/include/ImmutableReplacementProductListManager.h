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
 * This implementation returns an immutable list of replacement products.
 * It holds ReplacementProductListManager::kReplacementProductListMaxSize products in the list.
 */

class ImmutableReplacementProductListManager : public ReplacementProductListManager
{
public:
    CHIP_ERROR Next(ReplacementProductStruct & item) override;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
