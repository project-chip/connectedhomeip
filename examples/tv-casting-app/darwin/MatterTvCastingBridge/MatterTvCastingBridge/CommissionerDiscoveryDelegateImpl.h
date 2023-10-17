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
#include <system/SystemClock.h>

class CommissionerDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate {
public:
    void SetUp(dispatch_queue_t _Nonnull clientQueue,
        void (^_Nonnull objCDiscoveredCommissionerHandler)(DiscoveredNodeData * _Nonnull),
        TargetVideoPlayerInfo * _Nullable cachedTargetVideoPlayerInfos)
    {
        mClientQueue = clientQueue;
        mObjCDiscoveredCommissionerHandler = objCDiscoveredCommissionerHandler;
        mCachedTargetVideoPlayerInfos = cachedTargetVideoPlayerInfos;
        mDiscoveredCommissioners.clear();

        /**
         * Surface players (as DiscoveredNodeData objects on discoverySuccessCallback) that we previously
         * connected to and received their WakeOnLAN MACAddress, but could not discover over DNS-SD this time in
         * CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC. This API will also ensure that the reported players
         * were previously discoverable within CHIP_DEVICE_CONFIG_STR_CACHE_LAST_DISCOVERED_HOURS.
         *
         * The DiscoveredNodeData object for such players will have the IsAsleep attribute set to true,
         * which can optionally be used for any special UX treatment when displaying them.
         *
         * Surfacing such players as discovered will allow displaying them to the user, who may want to
         * cast to them. In such a case, the VerifyOrEstablishConnection API will turn them on over
         * WakeOnLan.
         */
        chip::DeviceLayer::SystemLayer().CancelTimer(
            ReportSleepingCommissioners, this); // cancel preexisting timer for ReportSleepingCommissioners, if any
        if (mCachedTargetVideoPlayerInfos != nullptr && mCachedTargetVideoPlayerInfos[0].IsInitialized()) {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(
#ifdef CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC
                                                            CHIP_DEVICE_CONFIG_STR_DISCOVERY_DELAY_SEC
#else
                                                            0
#endif
                                                            * 1000),
                ReportSleepingCommissioners, this);
        }
    }

    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
    {
        ChipLogProgress(AppServer, "CommissionerDiscoveryDelegateImpl().OnDiscoveredDevice() called");
        __block const chip::Dnssd::DiscoveredNodeData cppNodeData = nodeData;
        dispatch_async(mClientQueue, ^{
            DiscoveredNodeData * objCDiscoveredNodeData = [ConversionUtils convertToObjCDiscoveredNodeDataFrom:&cppNodeData];
            mDiscoveredCommissioners.push_back(objCDiscoveredNodeData); // add to the list of discovered commissioners

            // set associated connectable video player from cache, if any
            if (mCachedTargetVideoPlayerInfos != nullptr) {
                for (size_t i = 0; i < kMaxCachedVideoPlayers && mCachedTargetVideoPlayerInfos[i].IsInitialized(); i++) {
                    if (mCachedTargetVideoPlayerInfos[i].IsSameAs(&cppNodeData)) {
                        chip::System::Clock::Timestamp currentUnixTimeMS = chip::System::Clock::kZero;
                        chip::System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);
                        ChipLogProgress(AppServer, "Updating discovery timestamp for VideoPlayer %lu",
                            static_cast<unsigned long>(currentUnixTimeMS.count()));
                        mCachedTargetVideoPlayerInfos[i].SetLastDiscovered(currentUnixTimeMS); // add discovery timestamp
                        CastingServer::GetInstance()->AddVideoPlayer(
                            &mCachedTargetVideoPlayerInfos[i]); // write updated video player to cache

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
    static void ReportSleepingCommissioners(chip::System::Layer * _Nonnull aSystemLayer, void * _Nullable context)
    {
        ChipLogProgress(AppServer, "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() called");
        CommissionerDiscoveryDelegateImpl * thiz = (CommissionerDiscoveryDelegateImpl *) context;
        if (thiz == nullptr || thiz->mCachedTargetVideoPlayerInfos == nullptr) {
            ChipLogProgress(
                AppServer, "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() found no cached video players");
            return;
        }
        for (size_t i = 0; i < kMaxCachedVideoPlayers && thiz->mCachedTargetVideoPlayerInfos[i].IsInitialized(); i++) {
            // do NOT surface this cached Player if we don't have its MACAddress
            if (thiz->mCachedTargetVideoPlayerInfos[i].GetMACAddress() == nullptr
                && thiz->mCachedTargetVideoPlayerInfos[i].GetMACAddress()->size() == 0) {
                ChipLogProgress(NotSpecified,
                    "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() Skipping Player with hostName %s but no "
                    "MACAddress",
                    thiz->mCachedTargetVideoPlayerInfos[i].GetHostName());
                continue;
            }

            // do NOT surface this cached Player if it has not been discoverable recently
            if (!thiz->mCachedTargetVideoPlayerInfos[i].WasRecentlyDiscoverable()) {
                ChipLogProgress(NotSpecified,
                    "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() Skipping Player with hostName %s that "
                    "has not been discovered recently",
                    thiz->mCachedTargetVideoPlayerInfos[i].GetHostName());
                continue;
            }

            // do NOT surface this cached Player if it was just discovered right now (in this discovery call)
            bool justDiscovered = false;
            for (DiscoveredNodeData * discoveredCommissioner : thiz->mDiscoveredCommissioners) {
                if (strcmp(
                        (char *) [discoveredCommissioner.hostName UTF8String], thiz->mCachedTargetVideoPlayerInfos[i].GetHostName())
                    == 0) {
                    justDiscovered = true;
                    break;
                }
            }
            if (justDiscovered) {
                ChipLogProgress(NotSpecified,
                    "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() Skipping Player with hostName %s that "
                    "was just discovered",
                    thiz->mCachedTargetVideoPlayerInfos[i].GetHostName());
                continue;
            }

            // DO surface this cached Player (as asleep)
            DiscoveredNodeData * objCDiscoveredNodeData =
                [ConversionUtils convertToDiscoveredNodeDataFrom:&thiz->mCachedTargetVideoPlayerInfos[i]];
            objCDiscoveredNodeData.getConnectableVideoPlayer.isAsleep = true;
            ChipLogProgress(AppServer, "CommissionerDiscoveryDelegateImpl().ReportSleepingCommissioners() with hostName %s",
                thiz->mCachedTargetVideoPlayerInfos[i].GetHostName());
            thiz->mObjCDiscoveredCommissionerHandler(objCDiscoveredNodeData);
        }
    }

    std::vector<DiscoveredNodeData *> mDiscoveredCommissioners;
    void (^_Nonnull mObjCDiscoveredCommissionerHandler)(DiscoveredNodeData * _Nonnull);
    dispatch_queue_t _Nonnull mClientQueue;
    TargetVideoPlayerInfo * _Nullable mCachedTargetVideoPlayerInfos;
};

#endif /* CommissionerDiscoveryDelegateImpl_h */
