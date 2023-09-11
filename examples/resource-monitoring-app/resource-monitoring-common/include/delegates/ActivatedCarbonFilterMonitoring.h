/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ResourceMonitoring {

/// This is an application level delegate to handle ActivatedCarbonFilterMonitoringDelegate commands according to the specific
/// business logic.
class ActivatedCarbonFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~ActivatedCarbonFilterMonitoringDelegate() override = default;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
