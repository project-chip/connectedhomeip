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

CHIP_ERROR ClusterStateCache::UpdateCache(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                          const StatusIB & aStatus)
{
    AttributeState state;
    System::PacketBufferHandle handle;
    System::PacketBufferTLVWriter writer;
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
        state.Set<StatusIB>(aStatus);
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
    mChangedAttributeSet.insert(aPath);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterStateCache::UpdateEventCache(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus)
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
        System::PacketBufferHandle handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);

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

        mHighestReceivedEventNumber.SetValue(aEventHeader.mEventNumber);
    }
    else if (apStatus)
    {
        mEventStatusCache[aEventHeader.mPath] = *apStatus;
    }

    return CHIP_NO_ERROR;
}

void ClusterStateCache::OnReportBegin()
{
    mLastReportDataPath = ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);
    mChangedAttributeSet.clear();
    mAddedEndpoints.clear();
    mCallback.OnReportBegin();
}

void ClusterStateCache::CommitPendingDataVersion()
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

void ClusterStateCache::OnReportEnd()
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

CHIP_ERROR ClusterStateCache::Get(const ConcreteAttributePath & path, TLV::TLVReader & reader)
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

CHIP_ERROR ClusterStateCache::Get(EventNumber eventNumber, TLV::TLVReader & reader)
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

ClusterStateCache::EndpointState * ClusterStateCache::GetEndpointState(EndpointId endpointId, CHIP_ERROR & err)
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

ClusterStateCache::ClusterState * ClusterStateCache::GetClusterState(EndpointId endpointId, ClusterId clusterId, CHIP_ERROR & err)
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

const ClusterStateCache::AttributeState * ClusterStateCache::GetAttributeState(EndpointId endpointId, ClusterId clusterId,
                                                                               AttributeId attributeId, CHIP_ERROR & err)
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

const ClusterStateCache::EventData * ClusterStateCache::GetEventData(EventNumber eventNumber, CHIP_ERROR & err)
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

void ClusterStateCache::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
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

CHIP_ERROR ClusterStateCache::GetVersion(EndpointId mEndpointId, ClusterId mClusterId, Optional<DataVersion> & aVersion)
{
    CHIP_ERROR err;
    auto clusterState = GetClusterState(mEndpointId, mClusterId, err);
    ReturnErrorOnFailure(err);
    aVersion = clusterState->mCommittedDataVersion;
    return CHIP_NO_ERROR;
}

void ClusterStateCache::OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus)
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

CHIP_ERROR ClusterStateCache::GetStatus(const ConcreteAttributePath & path, StatusIB & status)
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

CHIP_ERROR ClusterStateCache::GetStatus(const ConcreteEventPath & path, StatusIB & status)
{
    auto statusIter = mEventStatusCache.find(path);
    if (statusIter == mEventStatusCache.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    status = statusIter->second;
    return CHIP_NO_ERROR;
}

void ClusterStateCache::GetSortedFilters(std::vector<std::pair<DataVersionFilter, size_t>> & aVector)
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
            uint32_t clusterSize    = 0;
            ClusterId clusterId     = clusterIter.first;

            for (auto const & attributeIter : clusterIter.second.mAttributes)
            {
                if (attributeIter.second.Is<StatusIB>())
                {
                    clusterSize +=
                        5; // 1 byte: anonymous tag control byte for struct. 1 byte: control byte for uint8 value. 1 byte:
                           // context-specific tag for uint8 value.1 byte: the uint8 value. 1 byte: end of container.
                    if (attributeIter.second.Get<StatusIB>().mClusterStatus.HasValue())
                    {
                        clusterSize += 3; // 1 byte: control byte for uint8 value. 1 byte: context-specific tag for uint8 value. 1
                                          // byte: the uint8 value.
                    }
                }
                else
                {
                    System::PacketBufferTLVReader bufReader;
                    bufReader.Init(attributeIter.second.Get<System::PacketBufferHandle>().Retain());
                    ReturnOnFailure(bufReader.Next());
                    // Skip to the end of the element.
                    ReturnOnFailure(bufReader.Skip());

                    // Compute the amount of value data
                    clusterSize += bufReader.GetLengthRead();
                }
            }
            if (clusterSize == 0)
            {
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

CHIP_ERROR ClusterStateCache::OnUpdateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                            const Span<AttributePathParams> & aAttributePaths,
                                                            bool & aEncodedDataVersionList)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVWriter backup;

    for (auto & attribute : aAttributePaths)
    {
        if (attribute.HasAttributeWildcard())
        {
            mRequestPathSet.insert(attribute);
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

        DataVersionFilterIB::Builder & filterIB = aDataVersionFilterIBsBuilder.CreateDataVersionFilter();
        SuccessOrExit(err = aDataVersionFilterIBsBuilder.GetError());
        ClusterPathIB::Builder & filterPath = filterIB.CreatePath();
        SuccessOrExit(err = filterIB.GetError());
        SuccessOrExit(
            err = filterPath.Endpoint(filter.first.mEndpointId).Cluster(filter.first.mClusterId).EndOfClusterPathIB().GetError());
        SuccessOrExit(err = filterIB.DataVersion(filter.first.mDataVersion.Value()).EndOfDataVersionFilterIB().GetError());
        ChipLogProgress(DataManagement, "Update DataVersionFilter: Endpoint=%u Cluster=" ChipLogFormatMEI " Version=%" PRIu32,
                        filter.first.mEndpointId, ChipLogValueMEI(filter.first.mClusterId), filter.first.mDataVersion.Value());

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

} // namespace app
} // namespace chip
