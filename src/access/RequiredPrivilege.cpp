/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "RequiredPrivilege.h"

namespace {

using namespace chip;
using namespace chip::Access;

// Privilege override entries are stored in a table per operation (read attribute,
// write attribute, invoke command, read entry). Entries in each table are sorted
// by their primary compound key of cluster/endpoint/field, and therefore cannot
// have duplicate privilege for any such combination. Cluster cannot be invalid,
// so that is used as a sentinel for empty/unused entries, which are kept at the
// end of the table. Endpoint and field can be invalid, which means wildcard.
// Sorting the entries in this way allows a cluster to override specific endpoints
// or fields with privilege, while still using wildcard to override all the
// unspecified ones.
struct PrivilegeOverride
{
    ClusterId mCluster;
    EndpointId mEndpoint;
    FieldId mField;
    Privilege mPrivilege;

    PrivilegeOverride() : mCluster(kInvalidClusterId), mEndpoint(kInvalidEndpointId), mField(kInvalidFieldId) {}

    PrivilegeOverride(ClusterId cluster, EndpointId endpoint, FieldId field, Privilege privilege) :
        mCluster(cluster), mEndpoint(endpoint), mField(field), mPrivilege(privilege)
    {}

    static_assert(sizeof(FieldId) >= sizeof(AttributeId), "FieldId must be able to hold AttributeId");
    static_assert(sizeof(FieldId) >= sizeof(CommandId), "FieldId must be able to hold CommandId");
    static_assert(sizeof(FieldId) >= sizeof(EventId), "FieldId must be able to hold EventId");
};

PrivilegeOverride
    privilegeOverrideForReadAttribute[CHIP_CONFIG_ACCESS_CONTROL_MAX_REQUIRED_PRIVILEGE_OVERRIDES_FOR_READ_ATTRIBUTE] = {
        PrivilegeOverride(0x0000'001f, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
    };

PrivilegeOverride
    privilegeOverrideForWriteAttribute[CHIP_CONFIG_ACCESS_CONTROL_MAX_REQUIRED_PRIVILEGE_OVERRIDES_FOR_WRITE_ATTRIBUTE] = {
        PrivilegeOverride(0x0000'001f, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
    };

PrivilegeOverride
    privilegeOverrideForInvokeCommand[CHIP_CONFIG_ACCESS_CONTROL_MAX_REQUIRED_PRIVILEGE_OVERRIDES_FOR_INVOKE_COMMAND] = {
        PrivilegeOverride(0x0000'001f, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
    };

PrivilegeOverride privilegeOverrideForReadEvent[CHIP_CONFIG_ACCESS_CONTROL_MAX_REQUIRED_PRIVILEGE_OVERRIDES_FOR_READ_EVENT] = {
    PrivilegeOverride(0x0000'001f, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
};

enum class Operation
{
    kReadAttribute  = 0,
    kWriteAttribute = 1,
    kInvokeCommand  = 2,
    kReadEvent      = 3
};

PrivilegeOverride * const privilegeOverride[] = { privilegeOverrideForReadAttribute, privilegeOverrideForWriteAttribute,
                                                  privilegeOverrideForInvokeCommand, privilegeOverrideForReadEvent };

const size_t maxPrivilegeOverride[] = { sizeof(privilegeOverrideForReadAttribute) / sizeof(privilegeOverrideForReadAttribute[0]),
                                        sizeof(privilegeOverrideForWriteAttribute) / sizeof(privilegeOverrideForWriteAttribute[0]),
                                        sizeof(privilegeOverrideForInvokeCommand) / sizeof(privilegeOverrideForInvokeCommand[0]),
                                        sizeof(privilegeOverrideForReadEvent) / sizeof(privilegeOverrideForReadEvent[0]) };

Privilege GetRequiredPrivilege(Operation operation, ClusterId cluster, EndpointId endpoint, FieldId field,
                               Privilege defaultPrivilege)
{
    VerifyOrDie(cluster != kInvalidClusterId && endpoint != kInvalidEndpointId && field != kInvalidFieldId);

    const auto * const pStart = privilegeOverride[static_cast<int>(operation)];
    const auto * const pEnd   = pStart + maxPrivilegeOverride[static_cast<int>(operation)];

    // TODO: note that the sorted nature of the table can be taken advantage of to skip around
    // (e.g. binary search), but if so, ensure to look for specific (non-wilcard) entries before
    // using a wildcard entry.

    for (const auto * p = pStart; p < pEnd; ++p)
    {
        if (p->mCluster == kInvalidClusterId || p->mCluster > cluster)
        {
            break; // not found
        }
        if (p->mCluster == cluster && (p->mEndpoint == endpoint || p->mEndpoint == kInvalidEndpointId) &&
            (p->mField == field || p->mField == kInvalidFieldId))
        {
            return p->mPrivilege;
        }
    }

    return defaultPrivilege;
}

CHIP_ERROR OverrideRequiredPrivilege(Operation operation, ClusterId cluster, EndpointId endpoint, FieldId field,
                                     Privilege privilege)
{
    // Can't override required privilege for access control cluster or all clusters.
    ReturnErrorCodeIf(cluster == 0x00000'001f || cluster == kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);

    // Required privilege must be more stringent than default.
    ReturnErrorCodeIf(privilege == Privilege::kView ||
                          (privilege == Privilege::kOperate &&
                           (operation == Operation::kWriteAttribute || operation == Operation::kInvokeCommand)),
                      CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: find insertion spot in table (assuming sorted by cluster, endpoint, field),
    //       insert, report errors appropriately (e.g. duplicate entry, no more space)

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR UnoverrideRequiredPrivilege(Operation operation, ClusterId cluster, EndpointId endpoint, FieldId field)
{
    // Can't unoverride required privilege for access control cluster or all clusters.
    ReturnErrorCodeIf(cluster == 0x00000'001f || cluster == kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: find override in table, remove it, keep table sorted, report errors appropriately
    //       (e.g. entry not found)

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace

namespace chip {
namespace Access {

Privilege RequiredPrivilege::ForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute)
{
    return GetRequiredPrivilege(Operation::kReadAttribute, cluster, endpoint, attribute, Privilege::kView);
}

Privilege RequiredPrivilege::ForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute)
{
    return GetRequiredPrivilege(Operation::kWriteAttribute, cluster, endpoint, attribute, Privilege::kOperate);
}

Privilege RequiredPrivilege::ForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command)
{
    return GetRequiredPrivilege(Operation::kInvokeCommand, cluster, endpoint, command, Privilege::kOperate);
}

Privilege RequiredPrivilege::ForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event)
{
    return GetRequiredPrivilege(Operation::kReadEvent, cluster, endpoint, event, Privilege::kView);
}

CHIP_ERROR RequiredPrivilege::OverrideForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute,
                                                       Privilege privilege)
{
    return OverrideRequiredPrivilege(Operation::kReadAttribute, cluster, endpoint, attribute, privilege);
}

CHIP_ERROR RequiredPrivilege::OverrideForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute,
                                                        Privilege privilege)
{
    return OverrideRequiredPrivilege(Operation::kWriteAttribute, cluster, endpoint, attribute, privilege);
}

CHIP_ERROR RequiredPrivilege::OverrideForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command,
                                                       Privilege privilege)
{
    return OverrideRequiredPrivilege(Operation::kInvokeCommand, cluster, endpoint, command, privilege);
}

CHIP_ERROR RequiredPrivilege::OverrideForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event, Privilege privilege)
{
    return OverrideRequiredPrivilege(Operation::kReadEvent, cluster, endpoint, event, privilege);
}

CHIP_ERROR RequiredPrivilege::UnoverrideForReadAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute)
{
    return UnoverrideRequiredPrivilege(Operation::kReadAttribute, cluster, endpoint, attribute);
}

CHIP_ERROR RequiredPrivilege::UnoverrideForWriteAttribute(ClusterId cluster, EndpointId endpoint, AttributeId attribute)
{
    return UnoverrideRequiredPrivilege(Operation::kWriteAttribute, cluster, endpoint, attribute);
}

CHIP_ERROR RequiredPrivilege::UnoverrideForInvokeCommand(ClusterId cluster, EndpointId endpoint, CommandId command)
{
    return UnoverrideRequiredPrivilege(Operation::kInvokeCommand, cluster, endpoint, command);
}

CHIP_ERROR RequiredPrivilege::UnoverrideForReadEvent(ClusterId cluster, EndpointId endpoint, EventId event)
{
    return UnoverrideRequiredPrivilege(Operation::kReadEvent, cluster, endpoint, event);
}

} // namespace Access
} // namespace chip
