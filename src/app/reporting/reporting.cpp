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
#include <app/util/attribute-storage.h>
#include <platform/LockTracker.h>

using namespace chip;
using namespace chip::app;

void MatterReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    // Attribute writes have asserted this already, but this assert should catch
    // applications notifying about changes from their end.
    assertChipStackLockedByCurrentThread();

    emberAfAttributeChanged(endpoint, clusterId, attributeId, emberAfGlobalInteractionModelAttributesChangedListener());
}

void MatterReportingAttributeChangeCallback(const ConcreteAttributePath & aPath)
{
    // Attribute writes have asserted this already, but this assert should catch
    // applications notifying about changes from their end.
    assertChipStackLockedByCurrentThread();

    emberAfAttributeChanged(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                            emberAfGlobalInteractionModelAttributesChangedListener());
}

void MatterReportingAttributeChangeCallback(EndpointId endpoint)
{
    // Attribute writes have asserted this already, but this assert should catch
    // applications notifying about changes from their end.
    assertChipStackLockedByCurrentThread();

    emberAfEndpointChanged(endpoint, emberAfGlobalInteractionModelAttributesChangedListener());
}
