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

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;

SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * EEMFirstInstance = nullptr;

inline void RegisterLegacyEEM(SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * inst)
{
    inst->mpNext     = EEMFirstInstance;
    EEMFirstInstance = inst;
}

inline void UnregisterLegacyEEM(SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * inst)
{
    SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * current  = EEMFirstInstance;
    SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * previous = nullptr;

    while (current != nullptr)
    {
        if (current == inst)
        {
            if (previous == nullptr)
            {
                // Removing first element
                EEMFirstInstance = current->mpNext;
            }
            else
            {
                previous->mpNext = current->mpNext;
            }
            break;
        }
        previous = current;
        current  = current->mpNext;
    }
}

} // namespace

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
    mCluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = endpointId,
        .featureFlags       = aFeature,
        .optionalAttributes = aOptionalAttrs,
    })
{
    mClusterListNode.mValue = &mCluster.Cluster();
}

const ElectricalEnergyMeasurement::MeasurementData * MeasurementDataForEndpoint(EndpointId endpointId)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnValue(cluster != nullptr, nullptr);

    return cluster->GetMeasurementData();
}

CHIP_ERROR SetMeasurementAccuracy(EndpointId endpointId, const MeasurementAccuracyStruct::Type & accuracy)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetMeasurementAccuracy(accuracy);
}

CHIP_ERROR SetCumulativeReset(EndpointId endpointId, const Optional<CumulativeEnergyResetStruct::Type> & cumulativeReset)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetCumulativeEnergyReset(cumulativeReset);
}

bool NotifyCumulativeEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                    const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);

    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->Features().Has(Feature::kCumulativeEnergy), false);

    cluster->CumulativeEnergySnapshot(energyImported, energyExported);
    return true;
}

bool NotifyPeriodicEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                  const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(endpointId);

    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->Features().Has(Feature::kPeriodicEnergy), false);

    cluster->PeriodicEnergySnapshot(energyImported, energyExported);
    return true;
}

CHIP_ERROR ElectricalEnergyMeasurementAttrAccess::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    RegisterLegacyEEM(&mClusterListNode);

    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(ElectricalEnergyMeasurement::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
    return err;
}

void ElectricalEnergyMeasurementAttrAccess::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));

    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(ElectricalEnergyMeasurement::Id), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }

    UnregisterLegacyEEM(&mClusterListNode);
}

bool ElectricalEnergyMeasurementAttrAccess::HasFeature(Feature aFeature) const
{
    return mCluster.Cluster().Features().Has(aFeature);
}

bool ElectricalEnergyMeasurementAttrAccess::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    // Convert from OptionalAttributes enum to AttributeId and check OptionalAttributesSet from cluster
    switch (aOptionalAttrs)
    {
    case OptionalAttributes::kOptionalAttributeCumulativeEnergyReset:
        return mCluster.Cluster().OptionalAttributes().IsSet(CumulativeEnergyReset::Id);
    default:
        return false;
    }
}

ElectricalEnergyMeasurementCluster * FindElectricalEnergyMeasurementClusterOnEndpoint(EndpointId endpointId)
{
    SingleLinkedListNode<ElectricalEnergyMeasurementCluster *> * current = EEMFirstInstance;
    while (current != nullptr)
    {
        if (current->mValue != nullptr && current->mValue->GetPaths()[0].mEndpointId == endpointId)
        {
            return current->mValue;
        }
        current = current->mpNext;
    }
    return nullptr;
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
