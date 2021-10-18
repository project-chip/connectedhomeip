/*
 *   Copyright (c) 2021 Project CHIP Authors
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

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

public class NsdManagerServiceResolver implements ServiceResolver {
  private static final String TAG = NsdManagerServiceResolver.class.getSimpleName();
  private static final long RESOLVE_SERVICE_TIMEOUT = 30000;
  private final NsdManager nsdManager;
  private MulticastLock multicastLock;
  private Handler mainThreadHandler;

  public NsdManagerServiceResolver(Context context) {
    this.nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);
    this.mainThreadHandler = new Handler(Looper.getMainLooper());

    this.multicastLock =
        ((WifiManager) context.getSystemService(Context.WIFI_SERVICE))
            .createMulticastLock("chipMulticastLock");
    this.multicastLock.setReferenceCounted(true);
  }

  @Override
  public void resolve(
      final String instanceName,
      final String serviceType,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    multicastLock.acquire();

    NsdServiceInfo serviceInfo = new NsdServiceInfo();
    serviceInfo.setServiceName(instanceName);
    serviceInfo.setServiceType(serviceType);
    Log.d(TAG, "Starting service resolution for '" + instanceName + "'");

    Runnable timeoutRunnable =
        new Runnable() {
          @Override
          public void run() {
            // Ensure we always release the multicast lock. It's possible that we release the
            // multicast lock here before ResolveListener returns, but since NsdManager has no API
            // to cancel service resolution, there's not much we can do here.
            if (multicastLock.isHeld()) {
              multicastLock.release();
            }
          }
        };

    this.nsdManager.resolveService(
        serviceInfo,
        new NsdManager.ResolveListener() {
          @Override
          public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
            Log.w(
                TAG,
                "Failed to resolve service '" + serviceInfo.getServiceName() + "': " + errorCode);
            chipMdnsCallback.handleServiceResolve(
                instanceName, serviceType, null, 0, null, callbackHandle, contextHandle, errorCode);

            if (multicastLock.isHeld()) {
              multicastLock.release();
            }
            mainThreadHandler.removeCallbacks(timeoutRunnable);
          }

          @Override
          public void onServiceResolved(NsdServiceInfo serviceInfo) {
            Log.i(
                TAG,
                "Resolved service '"
                    + serviceInfo.getServiceName()
                    + "' to "
                    + serviceInfo.getHost());
            // TODO: Find out if DNS-SD results for Android should contain interface ID
            chipMdnsCallback.handleServiceResolve(
                instanceName,
                serviceType,
                serviceInfo.getHost().getHostAddress(),
                serviceInfo.getPort(),
                serviceInfo.getAttributes(),
                callbackHandle,
                contextHandle,
                0);

            if (multicastLock.isHeld()) {
              multicastLock.release();
            }
            mainThreadHandler.removeCallbacks(timeoutRunnable);
          }
        });
    mainThreadHandler.postDelayed(timeoutRunnable, RESOLVE_SERVICE_TIMEOUT);
  }
}
