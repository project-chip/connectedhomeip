/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.chip.casting;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;
import chip.platform.NsdManagerServiceResolver;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class NsdResolveListener implements NsdManager.ResolveListener {

  private static final String TAG = NsdResolveListener.class.getSimpleName();

  private static final int MAX_RESOLUTION_ATTEMPTS = 5;
  private static final int RESOLUTION_ATTEMPT_DELAY_SECS = 1;

  private final NsdManager nsdManager;
  private final List<Long> deviceTypeFilter;
  private final List<VideoPlayer> preCommissionedVideoPlayers;
  private final SuccessCallback<DiscoveredNodeData> successCallback;
  private final FailureCallback failureCallback;
  private final NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState;
  private final int resolutionAttemptNumber;

  public NsdResolveListener(
      NsdManager nsdManager,
      List<Long> deviceTypeFilter,
      List<VideoPlayer> preCommissionedVideoPlayers,
      SuccessCallback<DiscoveredNodeData> successCallback,
      FailureCallback failureCallback,
      NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState,
      int resolutionAttemptNumber) {
    this.nsdManager = nsdManager;
    this.deviceTypeFilter = deviceTypeFilter;
    this.preCommissionedVideoPlayers = preCommissionedVideoPlayers;
    if (preCommissionedVideoPlayers != null) {
      for (VideoPlayer videoPlayer : preCommissionedVideoPlayers) {
        Log.d(TAG, "Precommissioned video player: " + videoPlayer);
      }
    }
    this.successCallback = successCallback;
    this.failureCallback = failureCallback;
    this.nsdManagerResolverAvailState = nsdManagerResolverAvailState;
    this.resolutionAttemptNumber = resolutionAttemptNumber;
  }

  @Override
  public void onServiceResolved(NsdServiceInfo serviceInfo) {
    DiscoveredNodeData discoveredNodeData = new DiscoveredNodeData(serviceInfo);
    Log.d(TAG, "DiscoveredNodeData resolved: " + discoveredNodeData);

    if (nsdManagerResolverAvailState != null) {
      nsdManagerResolverAvailState.signalFree();
    }

    if (isPassingDeviceTypeFilter(discoveredNodeData)) {
      addCommissioningInfo(discoveredNodeData);
      successCallback.handle(discoveredNodeData);
    } else {
      Log.d(
          TAG,
          "DiscoveredNodeData ignored because it did not pass the device type filter  "
              + discoveredNodeData);
    }
  }

  @Override
  public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
    if (nsdManagerResolverAvailState != null) {
      if (errorCode != NsdManager.FAILURE_ALREADY_ACTIVE
          || resolutionAttemptNumber >= MAX_RESOLUTION_ATTEMPTS) {
        nsdManagerResolverAvailState.signalFree();
      }
    }

    switch (errorCode) {
      case NsdManager.FAILURE_ALREADY_ACTIVE:
        Log.e(TAG, "NsdResolveListener FAILURE_ALREADY_ACTIVE - Service: " + serviceInfo);
        if (resolutionAttemptNumber < MAX_RESOLUTION_ATTEMPTS) {
          Log.d(TAG, "NsdResolveListener Scheduling a retry to resolve service " + serviceInfo);
          Executors.newSingleThreadScheduledExecutor()
              .schedule(
                  new Runnable() {
                    @Override
                    public void run() {
                      nsdManager.resolveService(
                          serviceInfo,
                          new NsdResolveListener(
                              nsdManager,
                              deviceTypeFilter,
                              preCommissionedVideoPlayers,
                              successCallback,
                              failureCallback,
                              nsdManagerResolverAvailState,
                              resolutionAttemptNumber + 1));
                    }
                  },
                  RESOLUTION_ATTEMPT_DELAY_SECS,
                  TimeUnit.SECONDS);
        } else { // giving up
          failureCallback.handle(
              new MatterError(
                  3, "NsdResolveListener FAILURE_ALREADY_ACTIVE - Service: " + serviceInfo));
        }
        break;
      case NsdManager.FAILURE_INTERNAL_ERROR:
        Log.e(TAG, "NsdResolveListener FAILURE_INTERNAL_ERROR - Service: " + serviceInfo);
        failureCallback.handle(
            new MatterError(
                3, "NsdResolveListener FAILURE_INTERNAL_ERROR - Service: " + serviceInfo));
        break;
      case NsdManager.FAILURE_MAX_LIMIT:
        Log.e(TAG, "NsdResolveListener FAILURE_MAX_LIMIT - Service: " + serviceInfo);
        failureCallback.handle(
            new MatterError(19, "NsdResolveListener FAILURE_MAX_LIMIT - Service: " + serviceInfo));
        break;
    }
  }

  private boolean isPassingDeviceTypeFilter(DiscoveredNodeData discoveredNodeData) {
    return deviceTypeFilter == null
        || deviceTypeFilter.isEmpty()
        || deviceTypeFilter.contains(discoveredNodeData.getDeviceType());
  }

  private void addCommissioningInfo(DiscoveredNodeData discoveredNodeData) {
    if (preCommissionedVideoPlayers != null) {
      long currentUnixTimeMS = System.currentTimeMillis();
      for (VideoPlayer videoPlayer : preCommissionedVideoPlayers) {
        if (videoPlayer.isSameAs(discoveredNodeData)) {
          Log.d(
              TAG,
              "Matching Video Player with the following information found for DiscoveredNodeData"
                  + videoPlayer);
          Log.d(TAG, "Updating discovery timestamp for VideoPlayer to " + currentUnixTimeMS);
          videoPlayer.setLastDiscoveredMs(currentUnixTimeMS);
          discoveredNodeData.setConnectableVideoPlayer(videoPlayer);
          return;
        }
      }
    }
    Log.d(
        TAG,
        "No matching VideoPlayers found from the cache for new DiscoveredNodeData: "
            + discoveredNodeData);
  }
}
