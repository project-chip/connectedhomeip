/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "system/SystemPacketBuffer.h"
#include <app/AttributeCache.h>
#include <app/InteractionModelEngine.h>
#include <tuple>

namespace chip {
namespace app {

CHIP_ERROR AttributeCache::UpdateCache(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
{
    AttributeState state;
    System::PacketBufferHandle handle;
    System::PacketBufferTLVWriter writer;

    if (apData)
    {
        handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);

        writer.Init(std::move(handle), false);

        ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), *apData));
        ReturnErrorOnFailure(writer.Finalize(&handle));

        //
        // Compact the buffer down to a more reasonably sized packet buffer
        // if we can.
        //
        handle.RightSize();

        state.Set<System::PacketBufferHandle>(std::move(handle));
    }
    else
    {
        state.Set<StatusIB>(aStatus);
    }

    //
    // if the endpoint didn't exist previously, let's track the insertion
    // so that we can inform our callback of a new endpoint being added appropriately.
    //
    if (mCache.find(aPath.mEndpointId) == mCache.end())
    {
        mAddedEndpoints.push_back(aPath.mEndpointId);
    }

    mCache[aPath.mEndpointId][aPath.mClusterId][aPath.mAttributeId] = std::move(state);
    mChangedAttributeSet.insert(aPath);
    return CHIP_NO_ERROR;
}

void AttributeCache::OnReportBegin()
{
    mChangedAttributeSet.clear();
    mAddedEndpoints.clear();
    mCallback.OnReportBegin();
}

void AttributeCache::OnReportEnd()
{
    std::set<std::tuple<EndpointId, ClusterId>> changedClusters;

    //
    // Add the EndpointId and ClusterId into a set so that we only
    // convey unique combinations in the subsequent OnClusterChanged callback.
    //
    for (auto & path : mChangedAttributeSet)
    {
        mCallback.OnAttributeChanged(this, path);
        changedClusters.insert(std::make_tuple(path.mEndpointId, path.mClusterId));
    }

    for (auto & item : changedClusters)
    {
        mCallback.OnClusterChanged(this, std::get<0>(item), std::get<1>(item));
    }

    for (auto endpoint : mAddedEndpoints)
    {
        mCallback.OnEndpointAdded(this, endpoint);
    }

    mCallback.OnReportEnd();
}

void AttributeCache::OnAttributeData(const ConcreteDataAttributePath & aPath, DataVersion aVersion, TLV::TLVReader * apData,
                                     const StatusIB & aStatus)
{
    //
    // Since the cache itself is a ReadClient::Callback, it may be incorrectly passed in directly when registering with the
    // ReadClient. This should be avoided, since that bypasses the built-in buffered reader adapter callback that is needed for
    // lists to work correctly.
    //
    // Instead, the right callback should be retrieved using GetBufferedCallback().
    //
    // To catch such errors, we validate that the provided concrete path never indicates a raw list item operation (which the
    // buffered reader will handle and convert for us).
    //
    //
    VerifyOrDie(!aPath.IsListItemOperation());

    UpdateCache(aPath, apData, aStatus);

    //
    // Forward the call through.
    //
    mCallback.OnAttributeData(aPath, aVersion, apData, aStatus);
}

CHIP_ERROR AttributeCache::Get(const ConcreteAttributePath & path, TLV::TLVReader & reader)
{
    CHIP_ERROR err;

    auto attributeState = GetAttributeState(path.mEndpointId, path.mClusterId, path.mAttributeId, err);
    ReturnErrorOnFailure(err);

    if (attributeState->Is<StatusIB>())
    {
        return CHIP_ERROR_IM_STATUS_CODE_RECEIVED;
    }

    System::PacketBufferTLVReader bufReader;

    bufReader.Init(attributeState->Get<System::PacketBufferHandle>().Retain());
    ReturnErrorOnFailure(bufReader.Next());

    reader.Init(bufReader);
    return CHIP_NO_ERROR;
}

AttributeCache::EndpointState * AttributeCache::GetEndpointState(EndpointId endpointId, CHIP_ERROR & err)
{
    auto endpointIter = mCache.find(endpointId);
    if (endpointIter == mCache.end())
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
        return nullptr;
    }

    err = CHIP_NO_ERROR;
    return &endpointIter->second;
}

AttributeCache::ClusterState * AttributeCache::GetClusterState(EndpointId endpointId, ClusterId clusterId, CHIP_ERROR & err)
{
    auto endpointState = GetEndpointState(endpointId, err);
    if (err != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    auto clusterState = endpointState->find(clusterId);
    if (clusterState == endpointState->end())
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
        return nullptr;
    }

    err = CHIP_NO_ERROR;
    return &clusterState->second;
}

AttributeCache::AttributeState * AttributeCache::GetAttributeState(EndpointId endpointId, ClusterId clusterId,
                                                                   AttributeId attributeId, CHIP_ERROR & err)
{
    auto clusterState = GetClusterState(endpointId, clusterId, err);
    if (err != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    auto attributeState = clusterState->find(attributeId);
    if (attributeState == clusterState->end())
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
        return nullptr;
    }

    err = CHIP_NO_ERROR;
    return &attributeState->second;
}

CHIP_ERROR AttributeCache::GetStatus(const ConcreteAttributePath & path, StatusIB & status)
{
    CHIP_ERROR err;

    auto attributeState = GetAttributeState(path.mEndpointId, path.mClusterId, path.mAttributeId, err);
    ReturnErrorOnFailure(err);

    if (!attributeState->Is<StatusIB>())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    status = attributeState->Get<StatusIB>();
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
