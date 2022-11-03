/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Power Source Server Cluster
 ***************************************************************************/

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
using namespace chip::app::Clusters::PowerSource::Attributes;

namespace {

class PowerSourceAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), PowerSource::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

PowerSourceAttrAccess gAttrAccess;

CHIP_ERROR PowerSourceAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aPath.mAttributeId)
    {
    case ActiveBatFaults::Id:
        // TODO: Needs implementation.
        err = aEncoder.EncodeEmptyList();
        break;
    default:
        break;
    }

    return err;
}

} // anonymous namespace

void MatterPowerSourcePluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
