/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/electrical-energy-measurement-server/CodegenIntegration.h>

#include <protocols/interaction_model/StatusCode.h>

#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <zap-generated/gen_config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

ElectricalEnergyMeasurementAttrAccess::ElectricalEnergyMeasurementAttrAccess(BitMask<Feature> aFeature,
                                                                             BitMask<OptionalAttributes> aOptionalAttrs,
                                                                             EndpointId endpointId) :
    mEndpointId(endpointId), mCluster([&]() {
        // Convert from BitMask<OptionalAttributes> to OptionalAttributesSet
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributesSet;
        if (aOptionalAttrs.Has(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset))
        {
            optionalAttributesSet.Set<CumulativeEnergyReset::Id>();
        }
        return ElectricalEnergyMeasurementCluster::Config(endpointId, aFeature, optionalAttributesSet);
    }())
{}

ElectricalEnergyMeasurementCluster * FindElectricalEnergyMeasurementClusterOnEndpoint(EndpointId endpointId)
{
    for (ClusterId clusterId : CodegenDataModelProvider::Instance().Registry().ClustersOnEndpoint(endpointId))
    {
        if (clusterId == ElectricalEnergyMeasurement::Id)
        {
            ConcreteClusterPath path(endpointId, clusterId);
            ServerClusterInterface * interface = CodegenDataModelProvider::Instance().Registry().Get(path);
            return static_cast<ElectricalEnergyMeasurementCluster *>(interface);
        }
    }
    return nullptr;
}

ElectricalEnergyMeasurement::MeasurementData * MeasurementDataForEndpoint(EndpointId endpointId)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnValue(cluster != nullptr, nullptr);

    return const_cast<MeasurementData *>(cluster->GetMeasurementData());
}

CHIP_ERROR SetMeasurementAccuracy(EndpointId endpointId, const MeasurementAccuracyStruct::Type & accuracy)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return cluster->SetMeasurementAccuracy(accuracy);
}

CHIP_ERROR SetCumulativeReset(EndpointId endpointId, const Optional<CumulativeEnergyResetStruct::Type> & cumulativeReset)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return cluster->SetCumulativeEnergyReset(cumulativeReset);
}

bool NotifyCumulativeEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                    const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);

    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->Features().Has(Feature::kCumulativeEnergy), false);

    cluster->SetCumulativeEnergyImported(energyImported);
    cluster->SetCumulativeEnergyExported(energyExported);

    Events::CumulativeEnergyMeasured::Type event;
    event.energyImported = energyImported;
    event.energyExported = energyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
#if defined(CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS) &&                                                       \
    CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(Zcl, "[NotifyCumulativeEnergyMeasured] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyCumulativeEnergyMeasured] Sent event [endpointId=%d,eventNumber=%lu]", endpointId,
                    static_cast<unsigned long>(eventNumber));
    return true;
}

bool NotifyPeriodicEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                  const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);

    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->Features().Has(Feature::kPeriodicEnergy), false);

    Events::PeriodicEnergyMeasured::Type event;

    if (cluster->Features().Has(Feature::kImportedEnergy))
    {
        cluster->SetPeriodicEnergyImported(energyImported);
        event.energyImported = energyImported;
    }

    if (cluster->Features().Has(Feature::kExportedEnergy))
    {
        cluster->SetPeriodicEnergyExported(energyExported);
        event.energyExported = energyExported;
    }

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
#if defined(CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS) &&                                                       \
    CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(Zcl, "[NotifyPeriodicEnergyMeasured] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyPeriodicEnergyMeasured] Sent event [endpointId=%d,eventNumber=%lu]", endpointId,
                    static_cast<unsigned long>(eventNumber));
    return true;
}

CHIP_ERROR ElectricalEnergyMeasurementAttrAccess::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());

    if (err != CHIP_NO_ERROR)
    {
#if defined(CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS) &&                                                       \
    CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT, mEndpointId,
                     ChipLogValueMEI(ElectricalEnergyMeasurement::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
    return err;
}

void ElectricalEnergyMeasurementAttrAccess::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));

    if (err != CHIP_NO_ERROR)
    {
#if defined(CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS) &&                                                       \
    CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT, mEndpointId,
                     ChipLogValueMEI(ElectricalEnergyMeasurement::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

bool ElectricalEnergyMeasurementAttrAccess::HasFeature(Feature aFeature) const
{
    return const_cast<RegisteredServerCluster<ElectricalEnergyMeasurementCluster> &>(mCluster).Cluster().Features().Has(aFeature);
}

bool ElectricalEnergyMeasurementAttrAccess::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    // Convert from OptionalAttributes enum to AttributeId and check OptionalAttributesSet from cluster
    switch (aOptionalAttrs)
    {
    case OptionalAttributes::kOptionalAttributeCumulativeEnergyReset:
        return const_cast<RegisteredServerCluster<ElectricalEnergyMeasurementCluster> &>(mCluster)
            .Cluster()
            .OptionalAttributes()
            .IsSet(CumulativeEnergyReset::Id);
    default:
        return false;
    }
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
