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
#include <app/ClusterStateCache.h>
#include <app/InteractionModelEngine.h>
#include <tuple>

namespace chip {
namespace app {

namespace {

// Determine how much space a StatusIB takes up on the wire.
uint32_t SizeOfStatusIB(const StatusIB & aStatus)
{
    // 1 byte: anonymous tag control byte for struct.
    // 1 byte: control byte for uint8 value.
    // 1 byte: context-specific tag for uint8 value.
    // 1 byte: the uint8 value.
    // 1 byte: end of container.
    uint32_t size = 5;

    if (aStatus.mClusterStatus.HasValue())
    {
        // 1 byte: control byte for uint8 value.
        // 1 byte: context-specific tag for uint8 value.
        // 1 byte: the uint8 value.
        size += 3;
    }

    return size;
}

} // anonymous namespace

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::GetElementTLVSize(TLV::TLVReader * apData, uint32_t & aSize)
{
    Platform::ScopedMemoryBufferWithSize<uint8_t> backingBuffer;
    TLV::TLVReader reader;
    reader.Init(*apData);
    size_t totalBufSize = reader.GetTotalLength();
    backingBuffer.Calloc(totalBufSize);
    VerifyOrReturnError(backingBuffer.Get() != nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), totalBufSize);
    ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), reader));
    aSize = writer.GetLengthWritten();
    ReturnErrorOnFailure(writer.Finalize(backingBuffer));
    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::UpdateCache(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                                 const StatusIB & aStatus)
{
    AttributeState state;
    bool endpointIsNew = false;

    if (mCache.find(aPath.mEndpointId) == mCache.end())
    {
        //
        // Since we might potentially be creating a new entry at mCache[aPath.mEndpointId][aPath.mClusterId] that
        // wasn't there before, we need to check if an entry didn't exist there previously and remember that so that
        // we can appropriately notify our clients of the addition of a new endpoint.
        //
        endpointIsNew = true;
    }

    if (apData)
    {
        uint32_t elementSize = 0;
        ReturnErrorOnFailure(GetElementTLVSize(apData, elementSize));

        if constexpr (CanEnableDataCaching)
        {
            if (mCacheData)
            {
                Platform::ScopedMemoryBufferWithSize<uint8_t> backingBuffer;
                backingBuffer.Calloc(elementSize);
                VerifyOrReturnError(backingBuffer.Get() != nullptr, CHIP_ERROR_NO_MEMORY);
                TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), elementSize);
                ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), *apData));
                ReturnErrorOnFailure(writer.Finalize(backingBuffer));

                state.template Set<AttributeData>(std::move(backingBuffer));
            }
            else
            {
                state.template Set<uint32_t>(elementSize);
            }
        }
        else
        {
            state = elementSize;
        }

        //
        // Clear out the committed data version and only set it again once we have received all data for this cluster.
        // Otherwise, we may have incomplete data that looks like it's complete since it has a valid data version.
        //
        mCache[aPath.mEndpointId][aPath.mClusterId].mCommittedDataVersion.ClearValue();

        // This commits a pending data version if the last report path is valid and it is different from the current path.
        if (mLastReportDataPath.IsValidConcreteClusterPath() && mLastReportDataPath != aPath)
        {
            CommitPendingDataVersion();
        }

        bool foundEncompassingWildcardPath = false;
        for (const auto & path : mRequestPathSet)
        {
            if (path.IncludesAllAttributesInCluster(aPath))
            {
                foundEncompassingWildcardPath = true;
                break;
            }
        }

        // if this data item is encompassed by a wildcard path, let's go ahead and update its pending data version.
        if (foundEncompassingWildcardPath)
        {
            mCache[aPath.mEndpointId][aPath.mClusterId].mPendingDataVersion = aPath.mDataVersion;
        }

        mLastReportDataPath = aPath;
    }
    else
    {
        if constexpr (CanEnableDataCaching)
        {
            if (mCacheData)
            {
                state.template Set<StatusIB>(aStatus);
            }
            else
            {
                state.template Set<uint32_t>(SizeOfStatusIB(aStatus));
            }
        }
        else
        {
            state = SizeOfStatusIB(aStatus);
        }
    }

    //
    // if the endpoint didn't exist previously, let's track the insertion
    // so that we can inform our callback of a new endpoint being added appropriately.
    //
    if (endpointIsNew)
    {
        mAddedEndpoints.push_back(aPath.mEndpointId);
    }

    mCache[aPath.mEndpointId][aPath.mClusterId].mAttributes[aPath.mAttributeId] = std::move(state);

    if (mCacheData)
    {
        mChangedAttributeSet.insert(aPath);
    }

    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::UpdateEventCache(const EventHeader & aEventHeader, TLV::TLVReader * apData,
                                                                      const StatusIB * apStatus)
{
    if (apData)
    {
        //
        // If we've already seen this event before, there's no more work to be done.
        //
        if (mHighestReceivedEventNumber.HasValue() && aEventHeader.mEventNumber <= mHighestReceivedEventNumber.Value())
        {
            return CHIP_NO_ERROR;
        }
        if (mCacheData)
        {
            System::PacketBufferHandle handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
            VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);

            System::PacketBufferTLVWriter writer;
            writer.Init(std::move(handle), false);

            ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), *apData));
            ReturnErrorOnFailure(writer.Finalize(&handle));

            //
            // Compact the buffer down to a more reasonably sized packet buffer
            // if we can.
            //
            handle.RightSize();

            EventData eventData;
            eventData.first  = aEventHeader;
            eventData.second = std::move(handle);

            mEventDataCache.insert(std::move(eventData));
        }
        mHighestReceivedEventNumber.SetValue(aEventHeader.mEventNumber);
    }
    else if (apStatus)
    {
        if (mCacheData)
        {
            mEventStatusCache[aEventHeader.mPath] = *apStatus;
        }
    }

    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::OnReportBegin()
{
    mLastReportDataPath = ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);
    mChangedAttributeSet.clear();
    mAddedEndpoints.clear();
    mCallback.OnReportBegin();
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::CommitPendingDataVersion()
{
    if (!mLastReportDataPath.IsValidConcreteClusterPath())
    {
        return;
    }

    auto & lastClusterInfo = mCache[mLastReportDataPath.mEndpointId][mLastReportDataPath.mClusterId];
    if (lastClusterInfo.mPendingDataVersion.HasValue())
    {
        lastClusterInfo.mCommittedDataVersion = lastClusterInfo.mPendingDataVersion;
        lastClusterInfo.mPendingDataVersion.ClearValue();
    }
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::OnReportEnd()
{
    CommitPendingDataVersion();
    mLastReportDataPath = ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);
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

template <>
CHIP_ERROR ClusterStateCacheT<true>::Get(const ConcreteAttributePath & path, TLV::TLVReader & reader) const
{
    CHIP_ERROR err;
    auto attributeState = GetAttributeState(path.mEndpointId, path.mClusterId, path.mAttributeId, err);
    ReturnErrorOnFailure(err);

    if (attributeState->template Is<StatusIB>())
    {
        return CHIP_ERROR_IM_STATUS_CODE_RECEIVED;
    }

    if (!attributeState->template Is<AttributeData>())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    reader.Init(attributeState->template Get<AttributeData>().Get(), attributeState->template Get<AttributeData>().AllocatedSize());
    return reader.Next();
}

template <>
CHIP_ERROR ClusterStateCacheT<false>::Get(const ConcreteAttributePath & path, TLV::TLVReader & reader) const
{
    return CHIP_ERROR_KEY_NOT_FOUND;
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::Get(EventNumber eventNumber, TLV::TLVReader & reader) const
{
    CHIP_ERROR err;

    auto eventData = GetEventData(eventNumber, err);
    ReturnErrorOnFailure(err);

    System::PacketBufferTLVReader bufReader;

    bufReader.Init(eventData->second.Retain());
    ReturnErrorOnFailure(bufReader.Next());

    reader.Init(bufReader);
    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
const typename ClusterStateCacheT<CanEnableDataCaching>::EndpointState *
ClusterStateCacheT<CanEnableDataCaching>::GetEndpointState(EndpointId endpointId, CHIP_ERROR & err) const
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

template <bool CanEnableDataCaching>
const typename ClusterStateCacheT<CanEnableDataCaching>::ClusterState *
ClusterStateCacheT<CanEnableDataCaching>::GetClusterState(EndpointId endpointId, ClusterId clusterId, CHIP_ERROR & err) const
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

template <bool CanEnableDataCaching>
const typename ClusterStateCacheT<CanEnableDataCaching>::AttributeState *
ClusterStateCacheT<CanEnableDataCaching>::GetAttributeState(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                            CHIP_ERROR & err) const
{
    auto clusterState = GetClusterState(endpointId, clusterId, err);
    if (err != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    auto attributeState = clusterState->mAttributes.find(attributeId);
    if (attributeState == clusterState->mAttributes.end())
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
        return nullptr;
    }

    err = CHIP_NO_ERROR;
    return &attributeState->second;
}

template <bool CanEnableDataCaching>
const typename ClusterStateCacheT<CanEnableDataCaching>::EventData *
ClusterStateCacheT<CanEnableDataCaching>::GetEventData(EventNumber eventNumber, CHIP_ERROR & err) const
{
    EventData compareKey;

    compareKey.first.mEventNumber = eventNumber;
    auto eventData                = mEventDataCache.find(std::move(compareKey));
    if (eventData == mEventDataCache.end())
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
        return nullptr;
    }

    err = CHIP_NO_ERROR;
    return &(*eventData);
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
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

    // Copy the reader for forwarding
    TLV::TLVReader dataSnapshot;
    if (apData)
    {
        dataSnapshot.Init(*apData);
    }

    UpdateCache(aPath, apData, aStatus);

    //
    // Forward the call through.
    //
    mCallback.OnAttributeData(aPath, apData ? &dataSnapshot : nullptr, aStatus);
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::GetVersion(const ConcreteClusterPath & aPath,
                                                                Optional<DataVersion> & aVersion) const
{
    VerifyOrReturnError(aPath.IsValidConcreteClusterPath(), CHIP_ERROR_INVALID_ARGUMENT);
    CHIP_ERROR err;
    auto clusterState = GetClusterState(aPath.mEndpointId, aPath.mClusterId, err);
    ReturnErrorOnFailure(err);
    aVersion = clusterState->mCommittedDataVersion;
    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData,
                                                           const StatusIB * apStatus)
{
    VerifyOrDie(apData != nullptr || apStatus != nullptr);

    TLV::TLVReader dataSnapshot;
    if (apData)
    {
        dataSnapshot.Init(*apData);
    }

    UpdateEventCache(aEventHeader, apData, apStatus);
    mCallback.OnEventData(aEventHeader, apData ? &dataSnapshot : nullptr, apStatus);
}

template <>
CHIP_ERROR ClusterStateCacheT<true>::GetStatus(const ConcreteAttributePath & path, StatusIB & status) const
{
    CHIP_ERROR err;

    auto attributeState = GetAttributeState(path.mEndpointId, path.mClusterId, path.mAttributeId, err);
    ReturnErrorOnFailure(err);

    if (!attributeState->template Is<StatusIB>())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    status = attributeState->template Get<StatusIB>();
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR ClusterStateCacheT<false>::GetStatus(const ConcreteAttributePath & path, StatusIB & status) const
{
    return CHIP_ERROR_INVALID_ARGUMENT;
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::GetStatus(const ConcreteEventPath & path, StatusIB & status) const
{
    auto statusIter = mEventStatusCache.find(path);
    if (statusIter == mEventStatusCache.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    status = statusIter->second;
    return CHIP_NO_ERROR;
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::GetSortedFilters(std::vector<std::pair<DataVersionFilter, size_t>> & aVector) const
{
    for (auto const & endpointIter : mCache)
    {
        EndpointId endpointId = endpointIter.first;
        for (auto const & clusterIter : endpointIter.second)
        {
            if (!clusterIter.second.mCommittedDataVersion.HasValue())
            {
                continue;
            }
            DataVersion dataVersion = clusterIter.second.mCommittedDataVersion.Value();
            size_t clusterSize      = 0;
            ClusterId clusterId     = clusterIter.first;

            for (auto const & attributeIter : clusterIter.second.mAttributes)
            {
                if constexpr (CanEnableDataCaching)
                {
                    if (attributeIter.second.template Is<StatusIB>())
                    {
                        clusterSize += SizeOfStatusIB(attributeIter.second.template Get<StatusIB>());
                    }
                    else if (attributeIter.second.template Is<uint32_t>())
                    {
                        clusterSize += attributeIter.second.template Get<uint32_t>();
                    }
                    else
                    {
                        VerifyOrDie(attributeIter.second.template Is<AttributeData>());
                        TLV::TLVReader bufReader;
                        bufReader.Init(attributeIter.second.template Get<AttributeData>().Get(),
                                       attributeIter.second.template Get<AttributeData>().AllocatedSize());
                        ReturnOnFailure(bufReader.Next());
                        // Skip to the end of the element.
                        ReturnOnFailure(bufReader.Skip());

                        // Compute the amount of value data
                        clusterSize += bufReader.GetLengthRead();
                    }
                }
                else
                {
                    clusterSize += attributeIter.second;
                }
            }

            if (clusterSize == 0)
            {
                // No data in this cluster, so no point in sending a dataVersion
                // along at all.
                continue;
            }

            DataVersionFilter filter(endpointId, clusterId, dataVersion);

            aVector.push_back(std::make_pair(filter, clusterSize));
        }
    }

    std::sort(aVector.begin(), aVector.end(),
              [](const std::pair<DataVersionFilter, size_t> & x, const std::pair<DataVersionFilter, size_t> & y) {
                  return x.second > y.second;
              });
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::OnUpdateDataVersionFilterList(
    DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder, const Span<AttributePathParams> & aAttributePaths,
    bool & aEncodedDataVersionList)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVWriter backup;

    // Only put paths into mRequestPathSet if they cover clusters in their entirety and no other path in our path list
    // points to a specific attribute from any of those clusters.
    // this would help for data-out-of-sync issue when handling store data version for the particular case on two paths: (E1, C1,
    // wildcard), (wildcard, C1, A1)
    for (auto & attribute1 : aAttributePaths)
    {
        if (attribute1.HasWildcardAttributeId())
        {
            bool intersected = false;
            for (auto & attribute2 : aAttributePaths)
            {
                if (attribute2.HasWildcardAttributeId())
                {
                    continue;
                }

                if (attribute1.Intersects(attribute2))
                {
                    intersected = true;
                    break;
                }
            }

            if (!intersected)
            {
                mRequestPathSet.insert(attribute1);
            }
        }
    }

    std::vector<std::pair<DataVersionFilter, size_t>> filterVector;
    GetSortedFilters(filterVector);

    aEncodedDataVersionList = false;
    for (auto & filter : filterVector)
    {
        bool intersected = false;
        aDataVersionFilterIBsBuilder.Checkpoint(backup);

        // if the particular cached cluster does not intersect with user provided attribute paths, skip the cached one
        for (const auto & attributePath : aAttributePaths)
        {
            if (attributePath.IncludesAttributesInCluster(filter.first))
            {
                intersected = true;
                break;
            }
        }
        if (!intersected)
        {
            continue;
        }

        SuccessOrExit(err = aDataVersionFilterIBsBuilder.EncodeDataVersionFilterIB(filter.first));
        aEncodedDataVersionList = true;
    }

exit:
    if (err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        ChipLogProgress(DataManagement, "OnUpdateDataVersionFilterList out of space; rolling back");
        aDataVersionFilterIBsBuilder.Rollback(backup);
        err = CHIP_NO_ERROR;
    }
    return err;
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::ClearAttributes(EndpointId endpointId)
{
    mCache.erase(endpointId);
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::ClearAttributes(const ConcreteClusterPath & cluster)
{
    // Can't use GetEndpointState here, since that only handles const things.
    auto endpointIter = mCache.find(cluster.mEndpointId);
    if (endpointIter == mCache.end())
    {
        return;
    }

    auto & endpointState = endpointIter->second;
    endpointState.erase(cluster.mClusterId);
}

template <bool CanEnableDataCaching>
void ClusterStateCacheT<CanEnableDataCaching>::ClearAttribute(const ConcreteAttributePath & attribute)
{
    // Can't use GetClusterState here, since that only handles const things.
    auto endpointIter = mCache.find(attribute.mEndpointId);
    if (endpointIter == mCache.end())
    {
        return;
    }

    auto & endpointState = endpointIter->second;
    auto clusterIter     = endpointState.find(attribute.mClusterId);
    if (clusterIter == endpointState.end())
    {
        return;
    }

    auto & clusterState = clusterIter->second;
    clusterState.mAttributes.erase(attribute.mAttributeId);
}

template <bool CanEnableDataCaching>
CHIP_ERROR ClusterStateCacheT<CanEnableDataCaching>::GetLastReportDataPath(ConcreteClusterPath & aPath)
{
    if (mLastReportDataPath.IsValidConcreteClusterPath())
    {
        aPath = mLastReportDataPath;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

// Ensure that our out-of-line template methods actually get compiled.
template class ClusterStateCacheT<true>;
template class ClusterStateCacheT<false>;

} // namespace app
} // namespace chip
