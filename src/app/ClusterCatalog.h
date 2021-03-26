/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2020 Google, LLC.
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

#pragma once

#include <app/Catalog.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {
/*
 *  @class ClusterCatalog
 *
 *  @brief A Chip provided reference implementation of the Catalog interface for a collection of cluster data instances
 *         that all refer to the same node Id. It provides an array-backed, bounded storage for these instances.
 */
template <typename T>
class ClusterCatalog : public CatalogInterface<T>
{
public:
    struct CatalogItem
    {
        EndpointId mEndpointId;
        T * mpCluster;
    };

    /*
     * Instances a cluster catalog given a pointer to the underlying array store.
     */
    ClusterCatalog(NodeId aNodeId, CatalogItem * apCatalogStore, uint16_t aNumMaxCatalogItems);

    /*
     * Add a new cluster data instance into the catalog for a particular endpoint and return a data handle to it.
     */
    CHIP_ERROR Add(EndpointId aEndpointId, T * aItem, ClusterDataHandle & aHandle);

    /*
     * Add a new cluster data instance bound to a user-selected cluster handle for a particular endpoint (which in this particular
     * implementation, denotes the offset in the array). The handle is to be between 0 and the size of the array. Also, the caller
     * should ensure no gaps form after every call made to this method.
     */
    CHIP_ERROR AddAt(EndpointId aEndpointId, T * aItem, ClusterDataHandle aHandle);

    /**
     * Removes a cluster instance from the catalog.
     */
    CHIP_ERROR Remove(ClusterDataHandle aHandle);

    CHIP_ERROR LocateClusterDataHandle(AttributePath::Parser & aParser, ClusterDataHandle & aClusterDataHandle) const;
    CHIP_ERROR LocateClusterDataHandle(ClusterId aClusterId, EndpointId aEndpointId, ClusterDataHandle & aHandle) const;
    CHIP_ERROR LocateClusterInstance(ClusterDataHandle aClusterDataHandle, T ** aClusterInstance) const;

    CHIP_ERROR GetClusterId(ClusterDataHandle aHandle, ClusterId & aClusterId) const;
    CHIP_ERROR GetEndpointId(ClusterDataHandle aHandle, EndpointId & aEndpointId) const;
    CHIP_ERROR GetNodeId(ClusterDataHandle aHandle, chip::NodeId & aNodeId) const;

private:
    CatalogItem * mpCatalogStore    = nullptr;
    chip::NodeId mNodeId            = 0;
    uint16_t mNumMaxCatalogItems    = 0;
    uint16_t mNumOfUsedCatalogItems = 0;
};

typedef ClusterCatalog<ClusterDataSink> ClusterDataSinkCatalog;
typedef ClusterCatalog<ClusterDataSource> ClusterDataSourceCatalog;

}; // namespace app
}; // namespace chip
