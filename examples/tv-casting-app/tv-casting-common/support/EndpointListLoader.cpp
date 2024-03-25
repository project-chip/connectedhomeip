/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "EndpointListLoader.h"

#include "clusters/Clusters.h"
#include "core/BaseCluster.h"
#include "core/CastingPlayer.h"
#include "core/Types.h"
#include "support/CastingStore.h"

#include "app/clusters/bindings/BindingManager.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace matter {
namespace casting {
namespace support {

using namespace matter::casting::core;

EndpointListLoader * EndpointListLoader::_endpointListLoader = nullptr;

EndpointListLoader::EndpointListLoader() {}

EndpointListLoader * EndpointListLoader::GetInstance()
{
    if (_endpointListLoader == nullptr)
    {
        _endpointListLoader = new EndpointListLoader();
    }
    return _endpointListLoader;
}

void EndpointListLoader::Initialize(chip::Messaging::ExchangeManager * exchangeMgr, const chip::SessionHandle * sessionHandle)
{
    mExchangeMgr   = exchangeMgr;
    mSessionHandle = sessionHandle;

    for (const auto & binding : chip::BindingTable::GetInstance())
    {
        if (binding.type == MATTER_UNICAST_BINDING && CastingPlayer::GetTargetCastingPlayer()->GetNodeId() == binding.nodeId)
        {
            // check to see if we discovered a new endpoint in the bindings
            chip::EndpointId endpointId                     = binding.remote;
            std::vector<memory::Strong<Endpoint>> endpoints = CastingPlayer::GetTargetCastingPlayer()->GetEndpoints();
            if (std::find_if(endpoints.begin(), endpoints.end(), [&endpointId](const memory::Strong<Endpoint> & endpoint) {
                    return endpoint->GetId() == endpointId;
                }) == endpoints.end())
            {
                mNewEndpointsToLoad++;
            }
        }
    }

    mPendingAttributeReads  = mNewEndpointsToLoad * kTotalDesiredAttributes;
    mEndpointAttributesList = new EndpointAttributes[mNewEndpointsToLoad];
    mEndpointServerLists    = new std::vector<chip::ClusterId>[mNewEndpointsToLoad];
}

CHIP_ERROR EndpointListLoader::Load()
{
    ChipLogProgress(AppServer, "EndpointListLoader::Load() called");

    VerifyOrReturnError(CastingPlayer::GetTargetCastingPlayer() != nullptr, CHIP_ERROR_INCORRECT_STATE);

    int endpointIndex      = -1;
    bool isLoadingRequired = false;
    for (const auto & binding : chip::BindingTable::GetInstance())
    {
        ChipLogProgress(AppServer,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
        if (binding.type == MATTER_UNICAST_BINDING && CastingPlayer::GetTargetCastingPlayer()->GetNodeId() == binding.nodeId)
        {
            // if we discovered a new Endpoint from the bindings, read its EndpointAttributes
            chip::EndpointId endpointId                     = binding.remote;
            std::vector<memory::Strong<Endpoint>> endpoints = CastingPlayer::GetTargetCastingPlayer()->GetEndpoints();
            if (std::find_if(endpoints.begin(), endpoints.end(), [&endpointId](const memory::Strong<Endpoint> & endpoint) {
                    return endpoint->GetId() == endpointId;
                }) == endpoints.end())
            {
                // Read attributes and mEndpointAttributesList for (endpointIndex + 1)
                ChipLogProgress(AppServer, "EndpointListLoader::Load Reading attributes for endpointId %d", endpointId);
                isLoadingRequired                            = true;
                mEndpointAttributesList[++endpointIndex].mId = endpointId;
                ReadVendorId(&mEndpointAttributesList[endpointIndex]);
                ReadProductId(&mEndpointAttributesList[endpointIndex]);
                ReadDeviceTypeList(&mEndpointAttributesList[endpointIndex]);
                ReadServerList(&mEndpointServerLists[endpointIndex], endpointId);
            }
        }
    }

    if (!isLoadingRequired)
    {
        ChipLogProgress(AppServer, "EndpointListLoader::Load found no new endpoints to load");
        mPendingAttributeReads = 0;
        Complete();
    }

    return CHIP_NO_ERROR;
}

void EndpointListLoader::Complete()
{
    ChipLogProgress(AppServer, "EndpointListLoader::Complete called with mPendingAttributeReads %lu", mPendingAttributeReads);
    if (mPendingAttributeReads > 0)
    {
        mPendingAttributeReads--;
    }

    if (mPendingAttributeReads == 0)
    {
        ChipLogProgress(AppServer, "EndpointListLoader::Complete Loading %lu endpoint(s)", mNewEndpointsToLoad);
        for (unsigned long i = 0; i < mNewEndpointsToLoad; i++)
        {
            EndpointAttributes endpointAttributes = mEndpointAttributesList[i];
            std::shared_ptr<Endpoint> endpoint =
                std::make_shared<Endpoint>(CastingPlayer::GetTargetCastingPlayer(), endpointAttributes);
            endpoint->RegisterClusters(mEndpointServerLists[i]);
            CastingPlayer::GetTargetCastingPlayer()->RegisterEndpoint(endpoint);
        }

        ChipLogProgress(AppServer, "EndpointListLoader::Complete finished Loading %lu endpoints", mNewEndpointsToLoad);

        // TODO cleanup
        // delete mEndpointAttributesList;
        mEndpointAttributesList = nullptr;
        // delete mEndpointServerLists;
        mEndpointServerLists = nullptr;
        mExchangeMgr         = nullptr;
        mSessionHandle       = nullptr;
        mNewEndpointsToLoad  = 0;

        // done loading endpoints, store TargetCastingPlayer
        CHIP_ERROR err = support::CastingStore::GetInstance()->AddOrUpdate(*CastingPlayer::GetTargetCastingPlayer());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "CastingStore::AddOrUpdate() failed. Err: %" CHIP_ERROR_FORMAT, err.Format());
        }

        // callback client OnCompleted
        VerifyOrReturn(CastingPlayer::GetTargetCastingPlayer()->mOnCompleted,
                       ChipLogError(AppServer, "EndpointListLoader::Complete mOnCompleted() not found"));
        CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(CHIP_NO_ERROR, CastingPlayer::GetTargetCastingPlayer());
    }
}

CHIP_ERROR EndpointListLoader::ReadVendorId(EndpointAttributes * endpointAttributes)
{
    core::MediaClusterBase cluster(*mExchangeMgr, *mSessionHandle, endpointAttributes->mId);

    return cluster.template ReadAttribute<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>(
        endpointAttributes,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType decodableVendorId) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            _endpointAttributes->mVendorId           = decodableVendorId;
            EndpointListLoader::GetInstance()->Complete();
        },
        [](void * context, CHIP_ERROR err) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            ChipLogError(AppServer, "EndpointListLoader ReadAttribute(VendorID) failed for endpointID %d. Err: %" CHIP_ERROR_FORMAT,
                         _endpointAttributes->mId, err.Format());
            EndpointListLoader::GetInstance()->Complete();
        });
}

CHIP_ERROR EndpointListLoader::ReadProductId(EndpointAttributes * endpointAttributes)
{
    core::MediaClusterBase cluster(*mExchangeMgr, *mSessionHandle, endpointAttributes->mId);

    return cluster.template ReadAttribute<chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo>(
        endpointAttributes,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType decodableProductId) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            _endpointAttributes->mProductId          = decodableProductId;
            EndpointListLoader::GetInstance()->Complete();
        },
        [](void * context, CHIP_ERROR err) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            ChipLogError(AppServer,
                         "EndpointListLoader ReadAttribute(ProductID) failed for endpointID %d. Err: %" CHIP_ERROR_FORMAT,
                         _endpointAttributes->mId, err.Format());
            EndpointListLoader::GetInstance()->Complete();
        });
}

CHIP_ERROR EndpointListLoader::ReadDeviceTypeList(EndpointAttributes * endpointAttributes)
{
    core::MediaClusterBase cluster(*mExchangeMgr, *mSessionHandle, endpointAttributes->mId);

    return cluster.template ReadAttribute<chip::app::Clusters::Descriptor::Attributes::DeviceTypeList::TypeInfo>(
        endpointAttributes,
        [](void * context,
           chip::app::Clusters::Descriptor::Attributes::DeviceTypeList::TypeInfo::DecodableArgType decodableDeviceTypeList) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            auto iter                                = decodableDeviceTypeList.begin();
            while (iter.Next())
            {
                auto & deviceType = iter.GetValue();
                _endpointAttributes->mDeviceTypeList.push_back(deviceType);
            }
            EndpointListLoader::GetInstance()->Complete();
        },
        [](void * context, CHIP_ERROR err) {
            EndpointAttributes * _endpointAttributes = static_cast<EndpointAttributes *>(context);
            ChipLogError(AppServer,
                         "EndpointListLoader ReadAttribute(DeviceTypeList) failed for endpointID %d. Err: %" CHIP_ERROR_FORMAT,
                         _endpointAttributes->mId, err.Format());
            EndpointListLoader::GetInstance()->Complete();
        });
}

CHIP_ERROR EndpointListLoader::ReadServerList(std::vector<chip::ClusterId> * endpointServerList, chip::EndpointId endpointId)
{
    core::MediaClusterBase cluster(*mExchangeMgr, *mSessionHandle, endpointId);

    return cluster.template ReadAttribute<chip::app::Clusters::Descriptor::Attributes::ServerList::TypeInfo>(
        endpointServerList,
        [](void * context,
           chip::app::Clusters::Descriptor::Attributes::ServerList::TypeInfo::DecodableArgType decodableServerList) {
            std::vector<chip::ClusterId> * _endpointServerList = static_cast<std::vector<chip::ClusterId> *>(context);
            auto iter                                          = decodableServerList.begin();
            while (iter.Next())
            {
                auto & clusterId = iter.GetValue();
                _endpointServerList->push_back(clusterId);
            }
            EndpointListLoader::GetInstance()->Complete();
        },
        [](void * context, CHIP_ERROR err) {
            ChipLogError(AppServer, "EndpointListLoader ReadAttribute(ServerList) failed. Err: %" CHIP_ERROR_FORMAT, err.Format());
            EndpointListLoader::GetInstance()->Complete();
        });
}

}; // namespace support
}; // namespace casting
}; // namespace matter
