/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ActivatedCarbonFilterMonitoring {
/// This is an application level Delegate to handle ActivatedCarbonfilterMonitoringDelegate commands according to the specific
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

void Shutdown();

} // namespace ActivatedCarbonFilterMonitoring

namespace HepaFilterMonitoring {
/// This is an application level Delegate to handle HepaFilterMonitoringDelegate commands according to the specific business logic.
class HepaFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~HepaFilterMonitoringDelegate() override = default;
};

class ImmutableReplacementProductListManager : public ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR
    Next(chip::app::Clusters::ResourceMonitoring::ReplacementProductStruct & item) override;
};

void Shutdown();

} // namespace HepaFilterMonitoring

} // namespace Clusters
} // namespace app
} // namespace chip
