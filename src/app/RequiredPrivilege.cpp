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

#include <app-common/zap-generated/cluster-objects.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

// Privilege override entries are stored in a table per operation (read attribute,
// write attribute, invoke command, read entry). Cluster cannot be invalid, but
// endpoint and field can be invalid, which means wildcard. For each cluster,
// more specific entries should be before less specific entries, so they take effect.
struct PrivilegeOverride
{
    ClusterId mCluster;
    EndpointId mEndpoint;
    Privilege mPrivilege; // NOTE: here so packing is tighter
    FieldId mField;

    constexpr PrivilegeOverride(ClusterId cluster, EndpointId endpoint, FieldId field, Privilege privilege) :
        mCluster(cluster), mEndpoint(endpoint), mPrivilege(privilege), mField(field)
    {}

    static_assert(sizeof(FieldId) >= sizeof(AttributeId), "FieldId must be able to hold AttributeId");
    static_assert(sizeof(FieldId) >= sizeof(CommandId), "FieldId must be able to hold CommandId");
    static_assert(sizeof(FieldId) >= sizeof(EventId), "FieldId must be able to hold EventId");
};

// WARNING: for each cluster, put more specific entries before less specific entries
constexpr PrivilegeOverride kPrivilegeOverrideForReadAttribute[] = {
    PrivilegeOverride(Clusters::AccessControl::Id, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
};

// WARNING: for each cluster, put more specific entries before less specific entries
constexpr PrivilegeOverride kPrivilegeOverrideForWriteAttribute[] = {
    PrivilegeOverride(Clusters::AccessControl::Id, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
};

// WARNING: for each cluster, put more specific entries before less specific entries
constexpr PrivilegeOverride kPrivilegeOverrideForInvokeCommand[] = {
    PrivilegeOverride(Clusters::AccessControl::Id, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
};

// WARNING: for each cluster, put more specific entries before less specific entries
constexpr PrivilegeOverride kPrivilegeOverrideForReadEvent[] = {
    PrivilegeOverride(Clusters::AccessControl::Id, kInvalidEndpointId, kInvalidFieldId, Privilege::kAdminister),
};

enum class Operation
{
    kReadAttribute  = 0,
    kWriteAttribute = 1,
    kInvokeCommand  = 2,
    kReadEvent      = 3
};

constexpr Privilege kDefaultPrivilege[] = {
    Privilege::kView,    // for read attribute
    Privilege::kOperate, // for write attribute
    Privilege::kOperate, // for invoke command
    Privilege::kView     // for read event
};

const PrivilegeOverride * const kPrivilegeOverride[] = { kPrivilegeOverrideForReadAttribute, kPrivilegeOverrideForWriteAttribute,
                                                         kPrivilegeOverrideForInvokeCommand, kPrivilegeOverrideForReadEvent };

constexpr size_t kNumPrivilegeOverride[] = { ArraySize(kPrivilegeOverrideForReadAttribute),
                                             ArraySize(kPrivilegeOverrideForWriteAttribute),
                                             ArraySize(kPrivilegeOverrideForInvokeCommand),
                                             ArraySize(kPrivilegeOverrideForReadEvent) };

Privilege GetRequiredPrivilege(Operation operation, ClusterId cluster, EndpointId endpoint, FieldId field)
{
    VerifyOrDie(cluster != kInvalidClusterId && endpoint != kInvalidEndpointId && field != kInvalidFieldId);

    const auto * const pStart = kPrivilegeOverride[static_cast<int>(operation)];
    const auto * const pEnd   = pStart + kNumPrivilegeOverride[static_cast<int>(operation)];

    for (const auto * p = pStart; p < pEnd; ++p)
    {
        if (p->mCluster == cluster && (p->mEndpoint == endpoint || p->mEndpoint == kInvalidEndpointId) &&
            (p->mField == field || p->mField == kInvalidFieldId))
        {
            return p->mPrivilege;
        }
    }

    return kDefaultPrivilege[static_cast<int>(operation)];
}

} // namespace

namespace chip {
namespace app {

Privilege RequiredPrivilege::ForReadAttribute(const ConcreteAttributePath & path)
{
    return GetRequiredPrivilege(Operation::kReadAttribute, path.mClusterId, path.mEndpointId, path.mAttributeId);
}

Privilege RequiredPrivilege::ForWriteAttribute(const ConcreteAttributePath & path)
{
    return GetRequiredPrivilege(Operation::kWriteAttribute, path.mClusterId, path.mEndpointId, path.mAttributeId);
}

Privilege RequiredPrivilege::ForInvokeCommand(const ConcreteCommandPath & path)
{
    return GetRequiredPrivilege(Operation::kInvokeCommand, path.mClusterId, path.mEndpointId, path.mCommandId);
}

Privilege RequiredPrivilege::ForReadEvent(const ConcreteEventPath & path)
{
    return GetRequiredPrivilege(Operation::kReadEvent, path.mClusterId, path.mEndpointId, path.mEventId);
}

} // namespace app
} // namespace chip
