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

#include <app/ClusterCatalog.h>
#include <app/MessageDef/AttributePath.h>

namespace chip {
namespace app {
template <typename T>
ClusterCatalog<T>::ClusterCatalog(NodeId aNodeId, CatalogItem * aCatalogStore, uint16_t aNumMaxCatalogItems)
{
    mNodeId = aNodeId;
    mpCatalogStore          = aCatalogStore;
    mNumMaxCatalogItems    = aNumMaxCatalogItems;
    mNumOfUsedCatalogItems = 0;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::Add(EndpointId aEndpointId, T * aItem, ClusterDataHandle & aHandle)
{
    if (mNumOfUsedCatalogItems >= mNumMaxCatalogItems)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    mpCatalogStore[mNumOfUsedCatalogItems].mEndpointId = aEndpointId;
    mpCatalogStore[mNumOfUsedCatalogItems].mpCluster   = aItem;
    aHandle                                            = mNumOfUsedCatalogItems++;

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::AddAt(chip::EndpointId aEndpointId, T * aItem, ClusterDataHandle aHandle)
{
    if (aHandle >= mNumMaxCatalogItems)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mpCatalogStore[aHandle].mEndpointId = aEndpointId;
    mpCatalogStore[aHandle].mpCluster       = aItem;
    mNumOfUsedCatalogItems++;

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::Remove(ClusterDataHandle aHandle)
{
    if (aHandle >= mNumOfUsedCatalogItems)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mpCatalogStore[aHandle].mpCluster = nullptr;

    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::LocateClusterDataHandle(app::AttributePath::Parser & aParser, ClusterDataHandle & aClusterDataHandle) const
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    ClusterId clusterId  = 0;
    EndpointId endpointId = 0;

    err = aParser.GetClusterId(&clusterId);
    SuccessOrExit(err);

    err = aParser.GetEndpointId(&endpointId);
    if ((CHIP_NO_ERROR != err) && (CHIP_END_OF_TLV != err))
    {
        ExitNow();
    }

    VerifyOrExit(clusterId != 0, err = CHIP_ERROR_TLV_TAG_NOT_FOUND);

    err = LocateClusterDataHandle(clusterId, endpointId, aClusterDataHandle);
    SuccessOrExit(err);

exit:
    return err;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::LocateClusterDataHandle(ClusterId aClusterId, EndpointId aEndpointId, ClusterDataHandle & aHandle) const
{
    for (uint16_t i = 0; i < mNumOfUsedCatalogItems; i++)
    {
        if (mpCatalogStore[i].mpCluster != nullptr)
        {
            if ((mpCatalogStore[i].mpCluster->GetSchemaEngine()->GetClusterId() == aClusterId) &&
                (mpCatalogStore[i].mEndpointId == aEndpointId))
            {
                aHandle = i;
                return CHIP_NO_ERROR;
            }
        }
    }

    return CHIP_ERROR_INVALID_PROFILE_ID;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::LocateClusterInstance(ClusterDataHandle aHandle, T ** aClusterInstance) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure the handle exists and mItem is not NULL
    VerifyOrExit((aHandle < mNumOfUsedCatalogItems && mpCatalogStore[aHandle].mpCluster != nullptr), err = CHIP_ERROR_INVALID_ARGUMENT);
    *aClusterInstance = mpCatalogStore[aHandle].mpCluster;

exit:
    return err;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::GetClusterId(ClusterDataHandle aHandle, ClusterId &aClusterId) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit((aHandle < mNumOfUsedCatalogItems && mpCatalogStore[aHandle].mpCluster != nullptr), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mpCatalogStore[aHandle].mpCluster->GetSchemaEngine()!= nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    aClusterId = mpCatalogStore[aHandle].mpCluster->GetSchemaEngine()->GetClusterId();

exit:
    return err;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::GetEndpointId(ClusterDataHandle aHandle, chip::EndpointId &aEndpointId) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure the handle exists and mpCluster is not nullptr
    VerifyOrExit((aHandle < mNumOfUsedCatalogItems && mpCatalogStore[aHandle].mpCluster != nullptr), err = CHIP_ERROR_INVALID_ARGUMENT);
    aEndpointId = mpCatalogStore[aHandle].mEndpointId;

exit:
    return err;
}

template <typename T>
CHIP_ERROR ClusterCatalog<T>::GetNodeId(ClusterDataHandle aHandle, chip::NodeId &aNodeId) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aNodeId = mNodeId;

    return err;
}

};
};

