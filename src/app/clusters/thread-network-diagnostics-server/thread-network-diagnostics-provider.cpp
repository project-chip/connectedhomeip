/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "thread-network-diagnostics-provider.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Encode.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK)
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#if CHIP_DEVICE_CONFIG_THREAD_FTD
#include <openthread/thread_ftd.h>
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
#endif // (CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK)

using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {

/*
 * @brief Get runtime value from the thread network based on the given attribute ID.
 *        The info is encoded via the AttributeValueEncoder.
 *
 * @param attributeId Id of the attribute for the requested info.
 * @param aEncoder Encoder to encode the attribute value.
 *
 * @return CHIP_NO_ERROR = Succes.
 *         CHIP_ERROR_NOT_IMPLEMENTED = Runtime value for this attribute not yet available to send as reply
 *                                      Use standard read.
 *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE = Is not a Runtime readable attribute. Use standard read
 *         All other errors should be treated as a read error and reported as such.
 *
 * @note This function implementation can compile in 3 different outcomes
 *       (1) CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK:
 *           - Generic implementation fetching the valid thread network data from the thread stack and encoding it respectively to
 *             the attributeID received.
 *       (2) CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK:
 *           - Encode a NULL value for nullable attributes or 0 for the others.
 *           - Devices using the ot-br-posix dbus stack have not yet provided the API to fetch the needed thread informations.
 *       (3) None of the conditions above
 *           - returns CHIP_ERROR_NOT_IMPLEMENTED.
 */
CHIP_ERROR WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder)
{
    CHIP_ERROR err;
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK)
    otInstance * otInst = ThreadStackMgrImpl().OTInstance();

    if (!otDatasetIsCommissioned(otInst))
    {
        // For the following nullable attributes of the cluster, encodeNull since
        // thread instance cannot provide the relevant data when it is not currently configured.
        //
        // Note that RoutingRole is nullable but not listed here as thread provides
        // valid data even when disabled or detached
        switch (attributeId)
        {
        case Attributes::Channel::Id:
        case Attributes::NetworkName::Id:
        case Attributes::PanId::Id:
        case Attributes::ExtendedPanId::Id:
        case Attributes::MeshLocalPrefix::Id:
        case Attributes::PartitionId::Id:
        case Attributes::Weighting::Id:
        case Attributes::DataVersion::Id:
        case Attributes::StableDataVersion::Id:
        case Attributes::LeaderRouterId::Id:
        case Attributes::ActiveTimestamp::Id:
        case Attributes::PendingTimestamp::Id:
        case Attributes::Delay::Id:
        case Attributes::SecurityPolicy::Id:
        case Attributes::ChannelPage0Mask::Id:
        case Attributes::OperationalDatasetComponents::Id:
            return encoder.EncodeNull();
        }
    }

    switch (attributeId)
    {
    case Attributes::Channel::Id: {
        uint16_t channel = otLinkGetChannel(otInst);
        err              = encoder.Encode(channel);
    }
    break;

    case Attributes::RoutingRole::Id: {
        using ThreadNetworkDiagnostics::RoutingRoleEnum;
        RoutingRoleEnum routingRole;
        otDeviceRole otRole = otThreadGetDeviceRole(otInst);

        if (otRole == OT_DEVICE_ROLE_DISABLED)
        {
            routingRole = RoutingRoleEnum::kUnspecified;
        }
        else if (otRole == OT_DEVICE_ROLE_DETACHED)
        {
            routingRole = RoutingRoleEnum::kUnassigned;
        }
        else if (otRole == OT_DEVICE_ROLE_ROUTER)
        {
            routingRole = RoutingRoleEnum::kRouter;
        }
        else if (otRole == OT_DEVICE_ROLE_LEADER)
        {
            routingRole = RoutingRoleEnum::kLeader;
        }
        else if (otRole == OT_DEVICE_ROLE_CHILD)
        {
            otLinkModeConfig linkMode = otThreadGetLinkMode(otInst);

            if (linkMode.mRxOnWhenIdle)
            {
                routingRole = RoutingRoleEnum::kEndDevice;
#if CHIP_DEVICE_CONFIG_THREAD_FTD
                if (otThreadIsRouterEligible(otInst))
                {
                    routingRole = RoutingRoleEnum::kReed;
                }
#endif
            }
            else
            {
                routingRole = RoutingRoleEnum::kSleepyEndDevice;
            }
        }

        err = encoder.Encode(routingRole);
    }
    break;

    case Attributes::NetworkName::Id: {
        const char * networkName = otThreadGetNetworkName(otInst);
        err                      = encoder.Encode(CharSpan::fromCharString(networkName));
    }
    break;

    case Attributes::PanId::Id: {
        uint16_t panId = otLinkGetPanId(otInst);
        err            = encoder.Encode(panId);
    }
    break;

    case Attributes::ExtendedPanId::Id: {
        const otExtendedPanId * pExtendedPanid = otThreadGetExtendedPanId(otInst);
        err                                    = encoder.Encode(Encoding::BigEndian::Get64(pExtendedPanid->m8));
    }
    break;

    case Attributes::MeshLocalPrefix::Id: {
        uint8_t meshLocaPrefix[OT_MESH_LOCAL_PREFIX_SIZE + 1] = { 0 }; // + 1  to encode prefix Len in the octstr

        const otMeshLocalPrefix * pMeshLocalPrefix = otThreadGetMeshLocalPrefix(otInst);
        meshLocaPrefix[0]                          = OT_IP6_PREFIX_BITSIZE;

        memcpy(&meshLocaPrefix[1], pMeshLocalPrefix->m8, OT_MESH_LOCAL_PREFIX_SIZE);
        err = encoder.Encode(ByteSpan(meshLocaPrefix));
    }
    break;

    case Attributes::OverrunCount::Id: {
        uint64_t overrunCount = 0; // mOverrunCount;
        err                   = encoder.Encode(overrunCount);
    }
    break;

    case Attributes::NeighborTable::Id: {
        err = encoder.EncodeList([otInst](const auto & aEncoder) -> CHIP_ERROR {
            constexpr uint16_t kFrameErrorRate100Percent   = 0xffff;
            constexpr uint16_t kMessageErrorRate100Percent = 0xffff;

            otNeighborInfo neighInfo;
            otNeighborInfoIterator iterator = OT_NEIGHBOR_INFO_ITERATOR_INIT;

            while (otThreadGetNextNeighborInfo(otInst, &iterator, &neighInfo) == OT_ERROR_NONE)
            {
                Structs::NeighborTableStruct::Type neighborTable;
                app::DataModel::Nullable<int8_t> averageRssi;
                app::DataModel::Nullable<int8_t> lastRssi;

                if (neighInfo.mAverageRssi == OT_RADIO_RSSI_INVALID)
                {
                    averageRssi.SetNull();
                }
                else
                {
                    // Thread average calculation already restrict mAverageRssi to be between -128 and 0
                    averageRssi.SetNonNull(neighInfo.mAverageRssi);
                }

                if (neighInfo.mLastRssi == OT_RADIO_RSSI_INVALID)
                {
                    lastRssi.SetNull();
                }
                else
                {
                    lastRssi.SetNonNull(min(static_cast<int8_t>(0), neighInfo.mLastRssi));
                }

                neighborTable.averageRssi      = averageRssi;
                neighborTable.lastRssi         = lastRssi;
                neighborTable.extAddress       = Encoding::BigEndian::Get64(neighInfo.mExtAddress.m8);
                neighborTable.age              = neighInfo.mAge;
                neighborTable.rloc16           = neighInfo.mRloc16;
                neighborTable.linkFrameCounter = neighInfo.mLinkFrameCounter;
                neighborTable.mleFrameCounter  = neighInfo.mMleFrameCounter;
                neighborTable.lqi              = neighInfo.mLinkQualityIn;
                neighborTable.frameErrorRate =
                    static_cast<uint8_t>((static_cast<uint32_t>(neighInfo.mFrameErrorRate) * 100) / kFrameErrorRate100Percent);
                neighborTable.messageErrorRate =
                    static_cast<uint8_t>((static_cast<uint32_t>(neighInfo.mMessageErrorRate) * 100) / kMessageErrorRate100Percent);
                neighborTable.rxOnWhenIdle     = neighInfo.mRxOnWhenIdle;
                neighborTable.fullThreadDevice = neighInfo.mFullThreadDevice;
                neighborTable.fullNetworkData  = neighInfo.mFullNetworkData;
                neighborTable.isChild          = neighInfo.mIsChild;

                ReturnErrorOnFailure(aEncoder.Encode(neighborTable));
            }

            return CHIP_NO_ERROR;
        });
    }
    break;

    case Attributes::RouteTable::Id: {
        err = encoder.EncodeList([otInst](const auto & aEncoder) -> CHIP_ERROR {
            otRouterInfo routerInfo;

#if CHIP_DEVICE_CONFIG_THREAD_FTD
            uint8_t maxRouterId = otThreadGetMaxRouterId(otInst);
            CHIP_ERROR chipErr  = CHIP_ERROR_INCORRECT_STATE;

            for (uint8_t i = 0; i <= maxRouterId; i++)
            {
                if (otThreadGetRouterInfo(otInst, i, &routerInfo) == OT_ERROR_NONE)
                {
                    Structs::RouteTableStruct::Type routeTable;

                    routeTable.extAddress      = Encoding::BigEndian::Get64(routerInfo.mExtAddress.m8);
                    routeTable.rloc16          = routerInfo.mRloc16;
                    routeTable.routerId        = routerInfo.mRouterId;
                    routeTable.nextHop         = routerInfo.mNextHop;
                    routeTable.pathCost        = routerInfo.mPathCost;
                    routeTable.LQIIn           = routerInfo.mLinkQualityIn;
                    routeTable.LQIOut          = routerInfo.mLinkQualityOut;
                    routeTable.age             = routerInfo.mAge;
                    routeTable.allocated       = routerInfo.mAllocated;
                    routeTable.linkEstablished = routerInfo.mLinkEstablished;

                    ReturnErrorOnFailure(aEncoder.Encode(routeTable));
                    chipErr = CHIP_NO_ERROR;
                }
            }

            return chipErr;

#else // OPENTHREAD_MTD
            otError otErr = otThreadGetParentInfo(otInst, &routerInfo);
            ReturnErrorOnFailure(Internal::MapOpenThreadError(otErr));

            Structs::RouteTableStruct::Type routeTable;

            routeTable.extAddress      = Encoding::BigEndian::Get64(routerInfo.mExtAddress.m8);
            routeTable.rloc16          = routerInfo.mRloc16;
            routeTable.routerId        = routerInfo.mRouterId;
            routeTable.nextHop         = routerInfo.mNextHop;
            routeTable.pathCost        = routerInfo.mPathCost;
            routeTable.LQIIn           = routerInfo.mLinkQualityIn;
            routeTable.LQIOut          = routerInfo.mLinkQualityOut;
            routeTable.age             = routerInfo.mAge;
            routeTable.allocated       = routerInfo.mAllocated;
            routeTable.linkEstablished = routerInfo.mLinkEstablished;

            ReturnErrorOnFailure(aEncoder.Encode(routeTable));
            return CHIP_NO_ERROR;
#endif
        });
    }
    break;

    case Attributes::PartitionId::Id: {
        uint32_t partitionId = otThreadGetPartitionId(otInst);
        err                  = encoder.Encode(partitionId);
    }
    break;

    case Attributes::Weighting::Id: {
        uint8_t weight = otThreadGetLeaderWeight(otInst);
        err            = encoder.Encode(weight);
    }
    break;

    case Attributes::DataVersion::Id: {
        uint8_t dataVersion = otNetDataGetVersion(otInst);
        err                 = encoder.Encode(dataVersion);
    }
    break;

    case Attributes::StableDataVersion::Id: {
        uint8_t stableVersion = otNetDataGetStableVersion(otInst);
        err                   = encoder.Encode(stableVersion);
    }
    break;

    case Attributes::LeaderRouterId::Id: {
        uint8_t leaderRouterId = otThreadGetLeaderRouterId(otInst);
        err                    = encoder.Encode(leaderRouterId);
    }
    break;

    case Attributes::DetachedRoleCount::Id: {
        uint16_t detachedRole = otThreadGetMleCounters(otInst)->mDetachedRole;
        err                   = encoder.Encode(detachedRole);
    }
    break;

    case Attributes::ChildRoleCount::Id: {
        uint16_t childRole = otThreadGetMleCounters(otInst)->mChildRole;
        err                = encoder.Encode(childRole);
    }
    break;

    case Attributes::RouterRoleCount::Id: {
        uint16_t routerRole = otThreadGetMleCounters(otInst)->mRouterRole;
        err                 = encoder.Encode(routerRole);
    }
    break;

    case Attributes::LeaderRoleCount::Id: {
        uint16_t leaderRole = otThreadGetMleCounters(otInst)->mLeaderRole;
        err                 = encoder.Encode(leaderRole);
    }
    break;

    case Attributes::AttachAttemptCount::Id: {
        uint16_t attachAttempts = otThreadGetMleCounters(otInst)->mAttachAttempts;
        err                     = encoder.Encode(attachAttempts);
    }
    break;

    case Attributes::PartitionIdChangeCount::Id: {
        uint16_t partitionIdChanges = otThreadGetMleCounters(otInst)->mPartitionIdChanges;
        err                         = encoder.Encode(partitionIdChanges);
    }
    break;

    case Attributes::BetterPartitionAttachAttemptCount::Id: {
        uint16_t betterPartitionAttachAttempts = otThreadGetMleCounters(otInst)->mBetterPartitionAttachAttempts;
        err                                    = encoder.Encode(betterPartitionAttachAttempts);
    }
    break;

    case Attributes::ParentChangeCount::Id: {
        uint16_t parentChanges = otThreadGetMleCounters(otInst)->mParentChanges;
        err                    = encoder.Encode(parentChanges);
    }
    break;

    case Attributes::TxTotalCount::Id: {
        uint32_t txTotal = otLinkGetCounters(otInst)->mTxTotal;
        err              = encoder.Encode(txTotal);
    }
    break;

    case Attributes::TxUnicastCount::Id: {
        uint32_t txUnicast = otLinkGetCounters(otInst)->mTxUnicast;
        err                = encoder.Encode(txUnicast);
    }
    break;

    case Attributes::TxBroadcastCount::Id: {
        uint32_t txBroadcast = otLinkGetCounters(otInst)->mTxBroadcast;
        err                  = encoder.Encode(txBroadcast);
    }
    break;

    case Attributes::TxAckRequestedCount::Id: {
        uint32_t txAckRequested = otLinkGetCounters(otInst)->mTxAckRequested;
        err                     = encoder.Encode(txAckRequested);
    }
    break;

    case Attributes::TxAckedCount::Id: {
        uint32_t txAcked = otLinkGetCounters(otInst)->mTxAcked;
        err              = encoder.Encode(txAcked);
    }
    break;

    case Attributes::TxNoAckRequestedCount::Id: {
        uint32_t txNoAckRequested = otLinkGetCounters(otInst)->mTxNoAckRequested;
        err                       = encoder.Encode(txNoAckRequested);
    }
    break;

    case Attributes::TxDataCount::Id: {
        uint32_t txData = otLinkGetCounters(otInst)->mTxData;
        err             = encoder.Encode(txData);
    }
    break;

    case Attributes::TxDataPollCount::Id: {
        uint32_t txDataPoll = otLinkGetCounters(otInst)->mTxDataPoll;
        err                 = encoder.Encode(txDataPoll);
    }
    break;

    case Attributes::TxBeaconCount::Id: {
        uint32_t txBeacon = otLinkGetCounters(otInst)->mTxBeacon;
        err               = encoder.Encode(txBeacon);
    }
    break;

    case Attributes::TxBeaconRequestCount::Id: {
        uint32_t txBeaconRequest = otLinkGetCounters(otInst)->mTxBeaconRequest;
        err                      = encoder.Encode(txBeaconRequest);
    }
    break;

    case Attributes::TxOtherCount::Id: {
        uint32_t txOther = otLinkGetCounters(otInst)->mTxOther;
        err              = encoder.Encode(txOther);
    }
    break;

    case Attributes::TxRetryCount::Id: {
        uint32_t txRetry = otLinkGetCounters(otInst)->mTxRetry;
        err              = encoder.Encode(txRetry);
    }
    break;

    case Attributes::TxDirectMaxRetryExpiryCount::Id: {
        uint32_t txDirectMaxRetryExpiry = otLinkGetCounters(otInst)->mTxDirectMaxRetryExpiry;
        err                             = encoder.Encode(txDirectMaxRetryExpiry);
    }
    break;

    case Attributes::TxIndirectMaxRetryExpiryCount::Id: {
        uint32_t txIndirectMaxRetryExpiry = otLinkGetCounters(otInst)->mTxIndirectMaxRetryExpiry;
        err                               = encoder.Encode(txIndirectMaxRetryExpiry);
    }
    break;

    case Attributes::TxErrCcaCount::Id: {
        uint32_t txErrCca = otLinkGetCounters(otInst)->mTxErrCca;
        err               = encoder.Encode(txErrCca);
    }
    break;

    case Attributes::TxErrAbortCount::Id: {
        uint32_t TxErrAbort = otLinkGetCounters(otInst)->mTxErrAbort;
        err                 = encoder.Encode(TxErrAbort);
    }
    break;

    case Attributes::TxErrBusyChannelCount::Id: {
        uint32_t TxErrBusyChannel = otLinkGetCounters(otInst)->mTxErrBusyChannel;
        err                       = encoder.Encode(TxErrBusyChannel);
    }
    break;

    case Attributes::RxTotalCount::Id: {
        uint32_t rxTotal = otLinkGetCounters(otInst)->mRxTotal;
        err              = encoder.Encode(rxTotal);
    }
    break;

    case Attributes::RxUnicastCount::Id: {
        uint32_t rxUnicast = otLinkGetCounters(otInst)->mRxUnicast;
        err                = encoder.Encode(rxUnicast);
    }
    break;

    case Attributes::RxBroadcastCount::Id: {
        uint32_t rxBroadcast = otLinkGetCounters(otInst)->mRxBroadcast;
        err                  = encoder.Encode(rxBroadcast);
    }
    break;

    case Attributes::RxDataCount::Id: {
        uint32_t rxData = otLinkGetCounters(otInst)->mRxData;
        err             = encoder.Encode(rxData);
    }
    break;

    case Attributes::RxDataPollCount::Id: {
        uint32_t rxDataPoll = otLinkGetCounters(otInst)->mRxDataPoll;
        err                 = encoder.Encode(rxDataPoll);
    }
    break;

    case Attributes::RxBeaconCount::Id: {
        uint32_t rxBeacon = otLinkGetCounters(otInst)->mRxBeacon;
        err               = encoder.Encode(rxBeacon);
    }
    break;

    case Attributes::RxBeaconRequestCount::Id: {
        uint32_t rxBeaconRequest = otLinkGetCounters(otInst)->mRxBeaconRequest;
        err                      = encoder.Encode(rxBeaconRequest);
    }
    break;

    case Attributes::RxOtherCount::Id: {
        uint32_t rxOther = otLinkGetCounters(otInst)->mRxOther;
        err              = encoder.Encode(rxOther);
    }
    break;

    case Attributes::RxAddressFilteredCount::Id: {
        uint32_t rxAddressFiltered = otLinkGetCounters(otInst)->mRxAddressFiltered;
        err                        = encoder.Encode(rxAddressFiltered);
    }
    break;

    case Attributes::RxDestAddrFilteredCount::Id: {
        uint32_t rxDestAddrFiltered = otLinkGetCounters(otInst)->mRxDestAddrFiltered;
        err                         = encoder.Encode(rxDestAddrFiltered);
    }
    break;

    case Attributes::RxDuplicatedCount::Id: {
        uint32_t rxDuplicated = otLinkGetCounters(otInst)->mRxDuplicated;
        err                   = encoder.Encode(rxDuplicated);
    }
    break;

    case Attributes::RxErrNoFrameCount::Id: {
        uint32_t rxErrNoFrame = otLinkGetCounters(otInst)->mRxErrNoFrame;
        err                   = encoder.Encode(rxErrNoFrame);
    }
    break;

    case Attributes::RxErrUnknownNeighborCount::Id: {
        uint32_t rxErrUnknownNeighbor = otLinkGetCounters(otInst)->mRxErrUnknownNeighbor;
        err                           = encoder.Encode(rxErrUnknownNeighbor);
    }
    break;

    case Attributes::RxErrInvalidSrcAddrCount::Id: {
        uint32_t rxErrInvalidSrcAddr = otLinkGetCounters(otInst)->mRxErrInvalidSrcAddr;
        err                          = encoder.Encode(rxErrInvalidSrcAddr);
    }
    break;

    case Attributes::RxErrSecCount::Id: {
        uint32_t rxErrSec = otLinkGetCounters(otInst)->mRxErrSec;
        err               = encoder.Encode(rxErrSec);
    }
    break;

    case Attributes::RxErrFcsCount::Id: {
        uint32_t rxErrFcs = otLinkGetCounters(otInst)->mRxErrFcs;
        err               = encoder.Encode(rxErrFcs);
    }
    break;

    case Attributes::RxErrOtherCount::Id: {
        uint32_t rxErrOther = otLinkGetCounters(otInst)->mRxErrOther;
        err                 = encoder.Encode(rxErrOther);
    }
    break;

    case Attributes::ActiveTimestamp::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));
        uint64_t activeTimestamp = (activeDataset.mActiveTimestamp.mSeconds << 16) | (activeDataset.mActiveTimestamp.mTicks << 1) |
            activeDataset.mActiveTimestamp.mAuthoritative;
        err = encoder.Encode(activeTimestamp);
    }
    break;

    case Attributes::PendingTimestamp::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));
        uint64_t pendingTimestamp = (activeDataset.mPendingTimestamp.mSeconds << 16) |
            (activeDataset.mPendingTimestamp.mTicks << 1) | activeDataset.mPendingTimestamp.mAuthoritative;
        err = encoder.Encode(pendingTimestamp);
    }
    break;

    case Attributes::Delay::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));
        uint32_t delay = activeDataset.mDelay;
        err            = encoder.Encode(delay);
    }
    break;

    case Attributes::SecurityPolicy::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));

        Structs::SecurityPolicy::Type securityPolicy;
        static_assert(sizeof(securityPolicy) == sizeof(activeDataset.mSecurityPolicy),
                      "securityPolicy Struct do not match otSecurityPolicy");
        uint16_t policyAsInts[2];
        static_assert(sizeof(policyAsInts) == sizeof(activeDataset.mSecurityPolicy),
                      "We're missing some members of otSecurityPolicy?");
        memcpy(&policyAsInts, &activeDataset.mSecurityPolicy, sizeof(policyAsInts));
        securityPolicy.rotationTime = policyAsInts[0];
        securityPolicy.flags        = policyAsInts[1];
        err                         = encoder.Encode(securityPolicy);
    }
    break;

    case Attributes::ChannelPage0Mask::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));

        // In the resultant Octet string, the most significant bit of the left-most byte indicates channel 0
        // We have to bitswap the entire uint32_t before converting to octet string
        uint32_t bitSwappedChannelMask = 0;
        for (int i = 0, j = 31; i < 32; i++, j--)
        {
            bitSwappedChannelMask |= ((activeDataset.mChannelMask >> j) & 1) << i;
        }

        uint8_t buffer[sizeof(uint32_t)] = { 0 };
        Encoding::BigEndian::Put32(buffer, bitSwappedChannelMask);
        err = encoder.Encode(ByteSpan(buffer));
    }
    break;

    case Attributes::OperationalDatasetComponents::Id: {
        otOperationalDataset activeDataset;
        otError otErr = otDatasetGetActive(otInst, &activeDataset);
        VerifyOrReturnError(otErr == OT_ERROR_NONE, Internal::MapOpenThreadError(otErr));
        Structs::OperationalDatasetComponents::Type OpDatasetComponents;

        OpDatasetComponents.activeTimestampPresent  = activeDataset.mComponents.mIsActiveTimestampPresent;
        OpDatasetComponents.pendingTimestampPresent = activeDataset.mComponents.mIsPendingTimestampPresent;
        OpDatasetComponents.masterKeyPresent        = activeDataset.mComponents.mIsNetworkKeyPresent;
        OpDatasetComponents.networkNamePresent      = activeDataset.mComponents.mIsNetworkNamePresent;
        OpDatasetComponents.extendedPanIdPresent    = activeDataset.mComponents.mIsExtendedPanIdPresent;
        OpDatasetComponents.meshLocalPrefixPresent  = activeDataset.mComponents.mIsMeshLocalPrefixPresent;
        OpDatasetComponents.delayPresent            = activeDataset.mComponents.mIsDelayPresent;
        OpDatasetComponents.panIdPresent            = activeDataset.mComponents.mIsPanIdPresent;
        OpDatasetComponents.channelPresent          = activeDataset.mComponents.mIsChannelPresent;
        OpDatasetComponents.pskcPresent             = activeDataset.mComponents.mIsPskcPresent;
        OpDatasetComponents.securityPolicyPresent   = activeDataset.mComponents.mIsSecurityPolicyPresent;
        OpDatasetComponents.channelMaskPresent      = activeDataset.mComponents.mIsChannelMaskPresent;

        err = encoder.Encode(OpDatasetComponents);
    }
    break;

    case Attributes::ActiveNetworkFaultsList::Id: {
        // activeNetworkFaults are not tracked by the thread stack nor the ThreadStackManager.
        // Encode an emptyList to indicate there are currently no active faults.
        err = encoder.EncodeEmptyList();
    }
    break;

    default: {
        err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    break;
    }

#elif (CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK)
    switch (attributeId)
    {
    case Attributes::NeighborTable::Id:
    case Attributes::RouteTable::Id:
    case Attributes::ActiveNetworkFaultsList::Id:
        err = encoder.EncodeEmptyList();
        break;
    case Attributes::Channel::Id:
    case Attributes::RoutingRole::Id:
    case Attributes::NetworkName::Id:
    case Attributes::PanId::Id:
    case Attributes::ExtendedPanId::Id:
    case Attributes::MeshLocalPrefix::Id:
    case Attributes::PartitionId::Id:
    case Attributes::Weighting::Id:
    case Attributes::DataVersion::Id:
    case Attributes::StableDataVersion::Id:
    case Attributes::LeaderRouterId::Id:
    case Attributes::ActiveTimestamp::Id:
    case Attributes::PendingTimestamp::Id:
    case Attributes::Delay::Id:
    case Attributes::ChannelPage0Mask::Id:
    case Attributes::SecurityPolicy::Id:
    case Attributes::OperationalDatasetComponents::Id:
        err = encoder.EncodeNull();
        break;
    case Attributes::OverrunCount::Id:
        err = encoder.Encode(static_cast<uint64_t>(0));
        break;
    case Attributes::DetachedRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::ChildRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::RouterRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::LeaderRoleCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::AttachAttemptCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::PartitionIdChangeCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::BetterPartitionAttachAttemptCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::ParentChangeCount::Id:
        err = encoder.Encode(static_cast<uint16_t>(0));
        break;
    case Attributes::TxTotalCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxUnicastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxBroadcastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxAckRequestedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxAckedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxNoAckRequestedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxDataCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxDataPollCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxBeaconCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxBeaconRequestCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxRetryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxDirectMaxRetryExpiryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxIndirectMaxRetryExpiryCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxErrCcaCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxErrAbortCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::TxErrBusyChannelCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxTotalCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxUnicastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxBroadcastCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxDataCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxDataPollCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxBeaconCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxBeaconRequestCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxAddressFilteredCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxDestAddrFilteredCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxDuplicatedCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrNoFrameCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrUnknownNeighborCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrInvalidSrcAddrCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrSecCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrFcsCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    case Attributes::RxErrOtherCount::Id:
        err = encoder.Encode(static_cast<uint32_t>(0));
        break;
    default:
        err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        break;
    }
#else
    err = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // (CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_USES_OTBR_POSIX_DBUS_STACK)
    return err;
}

} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
