/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/codegen-data-model-provider/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>

// separated out for code-reuse
#include <app/ember_coupling/EventPathValidity.mixin.h>

namespace chip {
namespace app {

bool CodegenDataModelProvider::EventPathIncludesAccessibleConcretePath(const EventPathParams & path, const Access::SubjectDescriptor & descriptor)
{

    if (!path.HasWildcardEndpointId())
    {
        // No need to check whether the endpoint is enabled, because
        // emberAfFindEndpointType returns null for disabled endpoints.
        return HasValidEventPathForEndpoint(path.mEndpointId, path, descriptor);
    }

    for (uint16_t endpointIndex = 0; endpointIndex < emberAfEndpointCount(); ++endpointIndex)
    {
        if (!emberAfEndpointIndexIsEnabled(endpointIndex))
        {
            continue;
        }
        if (HasValidEventPathForEndpoint(emberAfEndpointFromIndex(endpointIndex), path, descriptor))
        {
            return true;
        }
    }
    return false;
}

} // namespace app
} // namespace chip
