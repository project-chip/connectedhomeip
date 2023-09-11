/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CommissionerDiscoveryDelegateImpl_h
#define CommissionerDiscoveryDelegateImpl_h

#import "ConversionUtils.hpp"
#include <controller/DeviceDiscoveryDelegate.h>

class CommissionerDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate {
public:
    void SetUp(dispatch_queue_t _Nonnull clientQueue,
        void (^_Nonnull objCDiscoveredCommissionerHandler)(DiscoveredNodeData * _Nonnull),
        TargetVideoPlayerInfo * _Nullable cachedTargetVideoPlayerInfos)
    {
        mClientQueue = clientQueue;
        mObjCDiscoveredCommissionerHandler = objCDiscoveredCommissionerHandler;
        mCachedTargetVideoPlayerInfos = cachedTargetVideoPlayerInfos;
    }

    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
    {
        ChipLogProgress(AppServer, "CommissionerDiscoveryDelegateImpl().OnDiscoveredDevice() called");
        __block const chip::Dnssd::DiscoveredNodeData cppNodeData = nodeData;
        dispatch_async(mClientQueue, ^{
            DiscoveredNodeData * objCDiscoveredNodeData = [ConversionUtils convertToObjCDiscoveredNodeDataFrom:&cppNodeData];

            // set associated connectable video player from cache, if any
            if (mCachedTargetVideoPlayerInfos != nullptr) {
                for (size_t i = 0; i < kMaxCachedVideoPlayers && mCachedTargetVideoPlayerInfos[i].IsInitialized(); i++) {
                    if (mCachedTargetVideoPlayerInfos[i].IsSameAs(&cppNodeData)) {
                        VideoPlayer * connectableVideoPlayer =
                            [ConversionUtils convertToObjCVideoPlayerFrom:&mCachedTargetVideoPlayerInfos[i]];
                        [objCDiscoveredNodeData setConnectableVideoPlayer:connectableVideoPlayer];
                    }
                }
            }

            // make the callback
            mObjCDiscoveredCommissionerHandler(objCDiscoveredNodeData);
        });
    }

private:
    void (^_Nonnull mObjCDiscoveredCommissionerHandler)(DiscoveredNodeData * _Nonnull);
    dispatch_queue_t _Nonnull mClientQueue;
    TargetVideoPlayerInfo * _Nullable mCachedTargetVideoPlayerInfos;
};

#endif /* CommissionerDiscoveryDelegateImpl_h */
