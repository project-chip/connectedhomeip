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

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @brief Interface to allow interaction with interaction model and ember layers. Can be faked for unit testing.
 */
class MatterContext
{
public:
    MatterContext(EndpointId endpointId) : mEndpointId(endpointId) {}
    virtual ~MatterContext() = default;

    virtual void MarkDirty(AttributeId attributeId) { MatterReportingAttributeChangeCallback(mEndpointId, Id, attributeId); }

private:
    EndpointId mEndpointId = kInvalidEndpointId;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
