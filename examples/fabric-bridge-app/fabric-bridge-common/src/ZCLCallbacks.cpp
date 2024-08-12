/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BridgedDeviceManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/ZclString.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

#define ZCL_ADMINISTRATOR_COMMISSIONING_CLUSTER_REVISION (1u)

// External attribute read callback function
Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    if (clusterId == AdministratorCommissioning::Id)
    {
        // TODO(#34791) This is a workaround to prevent crash. CADMIN is still reading incorrect
        // Attribute values on dynamic endpoint as it only reads the root node and not the actual bridge
        // device we are representing here, when addressing the issue over there we can more easily
        // resolve this workaround.
        if ((attributeMetadata->attributeId == AdministratorCommissioning::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
        {
            uint16_t rev = ZCL_ADMINISTRATOR_COMMISSIONING_CLUSTER_REVISION;
            memcpy(buffer, &rev, sizeof(rev));
            return Protocols::InteractionModel::Status::Success;
        }
    }

    return Protocols::InteractionModel::Status::Failure;
}
