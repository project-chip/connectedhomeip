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
import java.util.List;

public class NsdDiscoveryListener implements NsdManager.DiscoveryListener {
  private static final String TAG = NsdDiscoveryListener.class.getSimpleName();

  private final NsdManager nsdManager;
  private final String targetServiceType;
  private final List<Long> deviceTypeFilter;
  private final SuccessCallback<DiscoveredNodeData> successCallback;
  private final FailureCallback failureCallback;

  public NsdDiscoveryListener(
      NsdManager nsdManager,
      String targetServiceType,
      List<Long> deviceTypeFilter,
      SuccessCallback<DiscoveredNodeData> successCallback,
      FailureCallback failureCallback) {
    this.nsdManager = nsdManager;
    this.targetServiceType = targetServiceType;
    this.deviceTypeFilter = deviceTypeFilter;
    this.successCallback = successCallback;
    this.failureCallback = failureCallback;
  }

  @Override
  public void onDiscoveryStarted(String regType) {
    Log.d(TAG, "Service discovery started. regType: " + regType);
  }

  @Override
  public void onServiceFound(NsdServiceInfo service) {
    Log.d(TAG, "Service discovery success. " + service);
    if (service.getServiceType().equals(targetServiceType)) {
      nsdManager.resolveService(
          service,
          new NsdResolveListener(nsdManager, deviceTypeFilter, successCallback, failureCallback));
    } else {
      Log.d(TAG, "Ignoring discovered service: " + service.toString());
    }
  }

  @Override
  public void onServiceLost(NsdServiceInfo service) {
    // When the network service is no longer available.
    // Internal bookkeeping code goes here.
    Log.e(TAG, "Service lost: " + service);
  }

  @Override
  public void onDiscoveryStopped(String serviceType) {
    Log.i(TAG, "Discovery stopped: " + serviceType);
  }

  @Override
  public void onStartDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to start: Error code:" + errorCode);
    failureCallback.handle(
        new MatterError(
            3, "NsdDiscoveryListener Discovery failed to start: Nsd Error code:" + errorCode));
    nsdManager.stopServiceDiscovery(this);
  }

  @Override
  public void onStopDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to stop: Error code:" + errorCode);
    failureCallback.handle(
        new MatterError(
            3, "NsdDiscoveryListener Discovery failed to stop: Nsd Error code:" + errorCode));
    nsdManager.stopServiceDiscovery(this);
  }
}
