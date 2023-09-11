/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/EventLogging.h>
#include <app/util/af-enums.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BridgedDeviceBasicInformation;

namespace {

void ReachableChanged(EndpointId endpointId)
{
    bool reachable = false;
    if (EMBER_ZCL_STATUS_SUCCESS != Attributes::Reachable::Get(endpointId, &reachable))
    {
        ChipLogError(Zcl, "ReachabledChanged: Failed to get Reachable value");
    }

    Events::ReachableChanged::Type event{ reachable };
    EventNumber eventNumber;
    if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
    {
        ChipLogError(Zcl, "ReachableChanged: Failed to record ReachableChanged event");
    }
}

} // anonymous namespace

void MatterBridgedDeviceBasicInformationClusterServerAttributeChangedCallback(const ConcreteAttributePath & attributePath)
{
    if (attributePath.mClusterId != BridgedDeviceBasicInformation::Id)
    {
        ChipLogError(Zcl, "MatterBridgedDeviceBasicClusterServerAttributeChangedCallback: Incorrect cluster ID");
        return;
    }

    switch (attributePath.mAttributeId)
    {
    case Attributes::Reachable::Id:
        ReachableChanged(attributePath.mEndpointId);
        break;
    default:
        break;
    }
}
