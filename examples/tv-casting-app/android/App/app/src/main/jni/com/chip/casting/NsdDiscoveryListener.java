/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
  private final List<VideoPlayer> preCommissionedVideoPlayers;
  private final SuccessCallback<DiscoveredNodeData> successCallback;
  private final FailureCallback failureCallback;

  public NsdDiscoveryListener(
      NsdManager nsdManager,
      String targetServiceType,
      List<Long> deviceTypeFilter,
      List<VideoPlayer> preCommissionedVideoPlayers,
      SuccessCallback<DiscoveredNodeData> successCallback,
      FailureCallback failureCallback) {
    this.nsdManager = nsdManager;
    this.targetServiceType = targetServiceType;
    this.deviceTypeFilter = deviceTypeFilter;
    this.preCommissionedVideoPlayers = preCommissionedVideoPlayers;
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
          new NsdResolveListener(
              nsdManager,
              deviceTypeFilter,
              preCommissionedVideoPlayers,
              successCallback,
              failureCallback));
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
