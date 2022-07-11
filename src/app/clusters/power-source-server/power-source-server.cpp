/*
 *    Copyright (c) 2022 Project CHIP Authors
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
