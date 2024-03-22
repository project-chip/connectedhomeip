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
#include "reporting.h"

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af.h>
#include <platform/LockTracker.h>

using namespace chip;
using namespace chip::app;

namespace {

void IncreaseClusterDataVersion(const ConcreteClusterPath & aConcreteClusterPath)
{
    DataVersion * version = emberAfDataVersionStorage(aConcreteClusterPath);
    if (version == nullptr)
    {
        ChipLogError(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " not found in IncreaseClusterDataVersion!",
                     aConcreteClusterPath.mEndpointId, ChipLogValueMEI(aConcreteClusterPath.mClusterId));
    }
    else
    {
        (*(version))++;
        ChipLogDetail(DataManagement, "Endpoint %x, Cluster " ChipLogFormatMEI " update version to %" PRIx32,
                      aConcreteClusterPath.mEndpointId, ChipLogValueMEI(aConcreteClusterPath.mClusterId), *(version));
    }
}

} // namespace

void MatterReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    // Attribute writes have asserted this already, but this assert should catch
    // applications notifying about changes from their end.
    assertChipStackLockedByCurrentThread();

    AttributePathParams info;
    info.mClusterId   = clusterId;
    info.mAttributeId = attributeId;
    info.mEndpointId  = endpoint;

    IncreaseClusterDataVersion(ConcreteClusterPath(endpoint, clusterId));
    InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(info);
}

void MatterReportingAttributeChangeCallback(const ConcreteAttributePath & aPath)
{
    return MatterReportingAttributeChangeCallback(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
}

void MatterReportingAttributeChangeCallback(EndpointId endpoint)
{
    // Attribute writes have asserted this already, but this assert should catch
    // applications notifying about changes from their end.
    assertChipStackLockedByCurrentThread();

    AttributePathParams info;
    info.mEndpointId = endpoint;

    // We are adding or enabling a whole endpoint, in this case, we do not touch the cluster data version.

    InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(info);
}
