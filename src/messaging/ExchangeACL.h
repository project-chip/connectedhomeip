/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <transport/FabricTable.h>

namespace chip {
namespace Messaging {

/**
 * @brief Defines a class that encapsulates ACL target information (cluster and endpoint ID).
 *        The class can be extended to add other parameters to the ACL target.
 */
class DLL_EXPORT ACLTarget
{
public:
    ACLTarget(ClusterId clusterId, EndpointId endpoint) : mClusterId(clusterId), mEndpoint(endpoint) {}
    virtual ~ACLTarget() {}

    ClusterId GetClusterId() { return mClusterId; }
    EndpointId GetEndpointId() { return mEndpoint; }

private:
    ClusterId mClusterId;
    EndpointId mEndpoint;
};

/**
 * @brief Defines a class that encapsulates ACL subject information (e.g. NodeId for CASE session).
 *        The class can be extended to add parameters to the ACL Subject.
 */
class DLL_EXPORT ACLSubject
{
public:
    virtual ~ACLSubject() {}

private:
};

/**
 * @brief Defines the common interface for PASE/CASE/GroupID based ACL permissions check.
 */
class DLL_EXPORT ExchangeACL
{
public:
    enum class PermissionLevel
    {
        kNone,
        kView,
        kOperate,
        kManage,
        kAdminister,
    };

    virtual ~ExchangeACL() {}

    /**
     * @brief
     *   Check access permissions for the message received from subject (sender) that
     *   are trying to access the target (e.g. cluster and endpoint).
     *
     * @param subject    The subject of the access check (e.g. source node ID)
     * @param target     The target of the message (i.e. cluster and endpoint)
     *
     * @return Permissions granted by the configured ACLs
     */
    virtual PermissionLevel GetPermissionLevel(const ACLSubject & subject, const ACLTarget & target) = 0;
};

class DLL_EXPORT CASEACLSubject
{
public:
    CASEACLSubject(NodeId id) : mNodeId(id) {}
    virtual ~CASEACLSubject() {}

    NodeId GetNodeId() { return mNodeId; }

private:
    NodeId mNodeId;
};

/**
 * @brief Specialized class that can perform ACL permissions check on messages that are
 *        exchanged on a CASE session.
 */
class DLL_EXPORT CASEExchangeACL : public ExchangeACL
{
public:
    CASEExchangeACL(Transport::FabricInfo * info) : mFabricInfo(info) {}
    virtual ~CASEExchangeACL() {}

    PermissionLevel GetPermissionLevel(const ACLSubject & subject, const ACLTarget & target) override
    {
        // TODO: Lookup the ACL corresponding to the subject, and the target,
        //       and enforce it.

        ReturnErrorCodeIf(mFabricInfo == nullptr, PermissionLevel::kNone);

        return PermissionLevel::kOperate;
    }

private:
    Transport::FabricInfo * mFabricInfo;
};

} // namespace Messaging
} // namespace chip
