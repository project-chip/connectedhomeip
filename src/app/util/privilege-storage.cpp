/**
 *
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

#include <lib/core/CHIPConfig.h>

#include "privilege-storage.h"

#if !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES
#include <zap-generated/access.h>
#endif // !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

#include <lib/support/CodeUtils.h>

using chip::AttributeId;
using chip::ClusterId;
using chip::CommandId;
using chip::EventId;

namespace {

#ifdef GENERATED_ACCESS_READ_ATTRIBUTE__CLUSTER
namespace GeneratedAccessReadAttribute {
constexpr ClusterId kCluster[]                 = GENERATED_ACCESS_READ_ATTRIBUTE__CLUSTER;
constexpr AttributeId kAttribute[]             = GENERATED_ACCESS_READ_ATTRIBUTE__ATTRIBUTE;
constexpr chip::Access::Privilege kPrivilege[] = GENERATED_ACCESS_READ_ATTRIBUTE__PRIVILEGE;
static_assert(MATTER_ARRAY_SIZE(kCluster) == MATTER_ARRAY_SIZE(kAttribute) &&
                  MATTER_ARRAY_SIZE(kAttribute) == MATTER_ARRAY_SIZE(kPrivilege),
              "Generated parallel arrays must be same size");
} // namespace GeneratedAccessReadAttribute
#endif

#ifdef GENERATED_ACCESS_WRITE_ATTRIBUTE__CLUSTER
namespace GeneratedAccessWriteAttribute {
constexpr ClusterId kCluster[]                 = GENERATED_ACCESS_WRITE_ATTRIBUTE__CLUSTER;
constexpr AttributeId kAttribute[]             = GENERATED_ACCESS_WRITE_ATTRIBUTE__ATTRIBUTE;
constexpr chip::Access::Privilege kPrivilege[] = GENERATED_ACCESS_WRITE_ATTRIBUTE__PRIVILEGE;
static_assert(MATTER_ARRAY_SIZE(kCluster) == MATTER_ARRAY_SIZE(kAttribute) &&
                  MATTER_ARRAY_SIZE(kAttribute) == MATTER_ARRAY_SIZE(kPrivilege),
              "Generated parallel arrays must be same size");
} // namespace GeneratedAccessWriteAttribute
#endif

#ifdef GENERATED_ACCESS_INVOKE_COMMAND__CLUSTER
namespace GeneratedAccessInvokeCommand {
constexpr ClusterId kCluster[]                 = GENERATED_ACCESS_INVOKE_COMMAND__CLUSTER;
constexpr CommandId kCommand[]                 = GENERATED_ACCESS_INVOKE_COMMAND__COMMAND;
constexpr chip::Access::Privilege kPrivilege[] = GENERATED_ACCESS_INVOKE_COMMAND__PRIVILEGE;
static_assert(MATTER_ARRAY_SIZE(kCluster) == MATTER_ARRAY_SIZE(kCommand) &&
                  MATTER_ARRAY_SIZE(kCommand) == MATTER_ARRAY_SIZE(kPrivilege),
              "Generated parallel arrays must be same size");
} // namespace GeneratedAccessInvokeCommand
#endif

#ifdef GENERATED_ACCESS_READ_EVENT__CLUSTER
namespace GeneratedAccessReadEvent {
constexpr ClusterId kCluster[]                 = GENERATED_ACCESS_READ_EVENT__CLUSTER;
constexpr EventId kEvent[]                     = GENERATED_ACCESS_READ_EVENT__EVENT;
constexpr chip::Access::Privilege kPrivilege[] = GENERATED_ACCESS_READ_EVENT__PRIVILEGE;
static_assert(MATTER_ARRAY_SIZE(kCluster) == MATTER_ARRAY_SIZE(kEvent) &&
                  MATTER_ARRAY_SIZE(kEvent) == MATTER_ARRAY_SIZE(kPrivilege),
              "Generated parallel arrays must be same size");
} // namespace GeneratedAccessReadEvent
#endif

} // anonymous namespace

chip::Access::Privilege MatterGetAccessPrivilegeForReadAttribute(ClusterId cluster, AttributeId attribute)
{
#ifdef GENERATED_ACCESS_READ_ATTRIBUTE__CLUSTER
    using namespace GeneratedAccessReadAttribute;
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(kCluster); ++i)
    {
        if (kCluster[i] == cluster && kAttribute[i] == attribute)
        {
            return kPrivilege[i];
        }
    }
#endif
    return chip::Access::Privilege::kView;
}

chip::Access::Privilege MatterGetAccessPrivilegeForWriteAttribute(ClusterId cluster, AttributeId attribute)
{
#ifdef GENERATED_ACCESS_WRITE_ATTRIBUTE__CLUSTER
    using namespace GeneratedAccessWriteAttribute;
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(kCluster); ++i)
    {
        if (kCluster[i] == cluster && kAttribute[i] == attribute)
        {
            return kPrivilege[i];
        }
    }
#endif
    return chip::Access::Privilege::kOperate;
}

chip::Access::Privilege MatterGetAccessPrivilegeForInvokeCommand(ClusterId cluster, CommandId command)
{
#ifdef GENERATED_ACCESS_INVOKE_COMMAND__CLUSTER
    using namespace GeneratedAccessInvokeCommand;
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(kCluster); ++i)
    {
        if (kCluster[i] == cluster && kCommand[i] == command)
        {
            return kPrivilege[i];
        }
    }
#endif
    return chip::Access::Privilege::kOperate;
}

chip::Access::Privilege MatterGetAccessPrivilegeForReadEvent(ClusterId cluster, EventId event)
{
#ifdef GENERATED_ACCESS_READ_EVENT__CLUSTER
    using namespace GeneratedAccessReadEvent;
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(kCluster); ++i)
    {
        if (kCluster[i] == cluster && kEvent[i] == event)
        {
            return kPrivilege[i];
        }
    }
#endif
    return chip::Access::Privilege::kView;
}
