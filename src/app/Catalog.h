/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 *    @file
 *    Defines catalogs that are used to house cluster data sources/sinks and can be used with the
 *    various Interaction Model engines to correctly map to/from path to actual cluster
 *    data instances.
 *
 */

#pragma once

#include <app/ClusterData.h>
#include <app/MessageDef/AttributePath.h>

namespace chip {
namespace app {
/* Unique handle to a particular cluster data instance within a catalog */
typedef uint16_t ClusterDataHandle;

/* A structure representing a path to a property or set of attributes within a cluster instance */
struct AttributePathParams
{
    AttributePathParams() {}
    AttributePathParams(ClusterDataHandle aDataHandle, AttributePathHandle aAttributePathHandle) :
        mClusterDataHandle(aDataHandle), mAttributePathHandle(aAttributePathHandle)
    {}
    bool operator==(const AttributePathParams & aClusterPathParams) const
    {
        return ((aClusterPathParams.mClusterDataHandle == mClusterDataHandle) &&
                (aClusterPathParams.mAttributePathHandle == mAttributePathHandle));
    }
    bool IsValid() { return !(mAttributePathHandle == kNullAttributePathHandle); }
    ClusterDataHandle mClusterDataHandle     = 0;
    AttributePathHandle mAttributePathHandle = kNullAttributePathHandle;
};

/*
 *  @class CatalogInterface
 *
 *  @brief A catalog interface that all concrete catalogs need to adhere to.
 *
 */
template <typename T>
class CatalogInterface
{
public:
    /**
     * Given a AttributePath Parser
     * and return the matching handle to the cluster.
     */
    virtual CHIP_ERROR LocateClusterDataHandle(AttributePath::Parser & aParser, ClusterDataHandle & aClusterDataHandle) const = 0;
    /**
     * Given a cluster id and endpoint id, return a reference to the matching cluster data instance.
     */
    virtual CHIP_ERROR LocateClusterDataHandle(ClusterId aClusterId, EndpointId aEndpointId, ClusterDataHandle & aHandle) const = 0;
    /**
     * Given a handle, return a reference to the matching cluster data instance.
     */
    virtual CHIP_ERROR LocateClusterInstance(ClusterDataHandle aClusterDataHandle, T ** aClusterInstance) const = 0;

    virtual CHIP_ERROR GetClusterId(ClusterDataHandle aHandle, ClusterId & aClusterId) const    = 0;
    virtual CHIP_ERROR GetEndpointId(ClusterDataHandle aHandle, EndpointId & aEndpointId) const = 0;
    virtual CHIP_ERROR GetNodeId(ClusterDataHandle aHandle, NodeId & aNodeId) const             = 0;

    virtual ~CatalogInterface() = default;
};

}; // namespace app
}; // namespace chip

#include <app/ClusterCatalog.h>
