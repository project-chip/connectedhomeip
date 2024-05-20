/*
 *   Copyright (c) 2023 Project CHIP Authors
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

package chip.platform;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.net.wifi.WifiManager.MulticastLock;
import android.util.Log;
import androidx.annotation.Nullable;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

class NsdServiceFinderAndResolver implements NsdManager.DiscoveryListener {
  private static final String TAG = NsdServiceFinderAndResolver.class.getSimpleName();

  private static final long BROWSE_SERVICE_TIMEOUT_MS = 5000L;

  private final NsdManager nsdManager;
  private final NsdServiceInfo targetServiceInfo;
  private final long callbackHandle;
  private final long contextHandle;
  private final ChipMdnsCallback chipMdnsCallback;
  private final MulticastLock multicastLock;
  private final ScheduledFuture<?> resolveTimeoutExecutor;

  @Nullable
  private final NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState;

  private ScheduledFuture<?> stopDiscoveryRunnable;

  public NsdServiceFinderAndResolver(
      final NsdManager nsdManager,
      final NsdServiceInfo targetServiceInfo,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback,
      final MulticastLock multicastLock,
      final ScheduledFuture<?> resolveTimeoutExecutor,
      final NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState) {
    this.nsdManager = nsdManager;
    this.targetServiceInfo = targetServiceInfo;
    this.callbackHandle = callbackHandle;
    this.contextHandle = contextHandle;
    this.chipMdnsCallback = chipMdnsCallback;
    this.multicastLock = multicastLock;
    this.resolveTimeoutExecutor = resolveTimeoutExecutor;
    this.nsdManagerResolverAvailState = nsdManagerResolverAvailState;
  }

  public void start() {
    multicastLock.acquire();

    NsdServiceFinderAndResolver serviceFinderResolver = this;
    this.stopDiscoveryRunnable =
        Executors.newSingleThreadScheduledExecutor()
            .schedule(
                new Runnable() {
                  @Override
                  public void run() {
                    Log.d(
                        TAG,
                        "Service discovery timed out after " + BROWSE_SERVICE_TIMEOUT_MS + " ms");
                    nsdManager.stopServiceDiscovery(serviceFinderResolver);
                    if (multicastLock.isHeld()) {
                      multicastLock.release();
                    }
                  }
                },
                BROWSE_SERVICE_TIMEOUT_MS,
                TimeUnit.MILLISECONDS);

    this.nsdManager.discoverServices(
        targetServiceInfo.getServiceType(), NsdManager.PROTOCOL_DNS_SD, this);
  }

  @Override
  public void onServiceFound(NsdServiceInfo service) {
    if (targetServiceInfo.getServiceName().equals(service.getServiceName())) {
      Log.d(TAG, "onServiceFound: found target service " + service);

      if (stopDiscoveryRunnable.cancel(false)) {
        nsdManager.stopServiceDiscovery(this);
      }

      resolveService(service, callbackHandle, contextHandle, chipMdnsCallback);
    } else {
      Log.d(TAG, "onServiceFound: found service not a target for resolution, ignoring " + service);
    }
  }

  private void resolveService(
      NsdServiceInfo serviceInfo,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    this.nsdManager.resolveService(
        serviceInfo,
        new NsdManager.ResolveListener() {
          @Override
          public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
            Log.w(
                TAG,
                "Failed to resolve service '" + serviceInfo.getServiceName() + "': " + errorCode);
            chipMdnsCallback.handleServiceResolve(
                serviceInfo.getServiceName(),
                // Use the target service info since the resolved service info sometimes appends a
                // "." at the front likely because it is trying to strip the service name out of it
                // and something is missed.
                // The target service info service type should be effectively the same as the
                // resolved service info.
                NsdServiceFinderAndResolver.this.targetServiceInfo.getServiceType(),
                null,
                null,
                0,
                null,
                callbackHandle,
                contextHandle);

            if (multicastLock.isHeld()) {
              multicastLock.release();

              if (nsdManagerResolverAvailState != null) {
                nsdManagerResolverAvailState.signalFree();
              }
            }
            resolveTimeoutExecutor.cancel(false);
          }

          @Override
          public void onServiceResolved(NsdServiceInfo serviceInfo) {
            Log.i(
                TAG,
                "Resolved service '"
                    + serviceInfo.getServiceName()
                    + "' to "
                    + serviceInfo.getHost()
                    + ", type : "
                    + serviceInfo.getServiceType());
            // TODO: Find out if DNS-SD results for Android should contain interface ID
            chipMdnsCallback.handleServiceResolve(
                serviceInfo.getServiceName(),
                // Use the target service info since the resolved service info sometimes appends a
                // "." at the front likely because it is trying to strip the service name out of it
                // and something is missed.
                // The target service info service type should be effectively the same as the
                // resolved service info.
                NsdServiceFinderAndResolver.this.targetServiceInfo.getServiceType(),
                serviceInfo.getHost().getHostName(),
                serviceInfo.getHost().getHostAddress(),
                serviceInfo.getPort(),
                serviceInfo.getAttributes(),
                callbackHandle,
                contextHandle);

            if (multicastLock.isHeld()) {
              multicastLock.release();

              if (nsdManagerResolverAvailState != null) {
                nsdManagerResolverAvailState.signalFree();
              }
            }
            resolveTimeoutExecutor.cancel(false);
          }
        });
  }

  @Override
  public void onDiscoveryStarted(String regType) {
    Log.d(TAG, "Service discovery started. regType: " + regType);
  }

  @Override
  public void onServiceLost(NsdServiceInfo service) {
    Log.e(TAG, "Service lost: " + service);
  }

  @Override
  public void onDiscoveryStopped(String serviceType) {
    Log.i(TAG, "Discovery stopped: " + serviceType);
  }

  @Override
  public void onStartDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to start: Error code: " + errorCode);
  }

  @Override
  public void onStopDiscoveryFailed(String serviceType, int errorCode) {
    Log.e(TAG, "Discovery failed to stop: Error code: " + errorCode);
  }
}
