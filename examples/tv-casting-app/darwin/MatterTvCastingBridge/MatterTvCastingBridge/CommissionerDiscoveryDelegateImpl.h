/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#ifndef CommissionerDiscoveryDelegateImpl_h
#define CommissionerDiscoveryDelegateImpl_h

#import "ConversionUtils.hpp"
#include <controller/DeviceDiscoveryDelegate.h>

class CommissionerDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate {
public:
    void SetUp(dispatch_queue_t _Nonnull clientQueue,
        void (^_Nonnull objCDiscoveredCommissionerHandler)(DiscoveredNodeData * _Nonnull),
        TargetVideoPlayerInfo * cachedTargetVideoPlayerInfos)
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
    TargetVideoPlayerInfo * mCachedTargetVideoPlayerInfos;
};

#endif /* CommissionerDiscoveryDelegateImpl_h */
