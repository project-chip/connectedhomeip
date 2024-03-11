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
import java.util.ArrayList;
import java.util.HashMap;

public class NsdManagerServiceBrowser implements ServiceBrowser {
  private static final String TAG = NsdManagerServiceBrowser.class.getSimpleName();
  private static final long BROWSE_SERVICE_TIMEOUT = 5000;
  private final NsdManager nsdManager;
  private MulticastLock multicastLock;
  private Handler mainThreadHandler;
  private final long timeout;

  private HashMap<Long, NsdManagerDiscovery> callbackMap;

  public NsdManagerServiceBrowser(Context context) {
    this(context, BROWSE_SERVICE_TIMEOUT);
  }

  /**
   * @param context application context
   * @param timeout Timeout value in case there is no response after calling browse
   */
  public NsdManagerServiceBrowser(Context context, long timeout) {
    this.nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);
    this.mainThreadHandler = new Handler(Looper.getMainLooper());

    this.multicastLock =
        ((WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE))
            .createMulticastLock("chipBrowseMulticastLock");
    this.multicastLock.setReferenceCounted(true);
    callbackMap = new HashMap<>();
    this.timeout = timeout;
  }

  @Override
  public void browse(
      final String serviceType,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    Runnable timeoutRunnable =
        new Runnable() {
          @Override
          public void run() {
            stopDiscover(callbackHandle, chipMdnsCallback);
          }
        };
    startDiscover(serviceType, callbackHandle, contextHandle, chipMdnsCallback);
    mainThreadHandler.postDelayed(timeoutRunnable, timeout);
  }

  public void startDiscover(
      final String serviceType,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    if (callbackMap.containsKey(callbackHandle)) {
      Log.d(TAG, "Invalid callbackHandle");
      return;
    }

    NsdManagerDiscovery discovery =
        new NsdManagerDiscovery(serviceType, callbackHandle, contextHandle);
    multicastLock.acquire();

    Log.d(TAG, "Starting service discovering for '" + serviceType + "'");

    this.nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discovery);
    callbackMap.put(callbackHandle, discovery);
  }

  public void stopDiscover(final long callbackHandle, final ChipMdnsCallback chipMdnsCallback) {
    if (!callbackMap.containsKey(callbackHandle)) {
      Log.d(TAG, "Invalid callbackHandle");
      return;
    }

    NsdManagerDiscovery discovery = callbackMap.remove(callbackHandle);
    if (multicastLock.isHeld()) {
      multicastLock.release();
    }

    this.nsdManager.stopServiceDiscovery(discovery);
    discovery.handleServiceBrowse(chipMdnsCallback);
  }

  public class NsdManagerDiscovery implements NsdManager.DiscoveryListener {
    private String serviceType;
    private long callbackHandle;
    private long contextHandle;
    private ArrayList<String> serviceNameList = new ArrayList<>();

    public NsdManagerDiscovery(String serviceType, long callbackHandle, long contextHandle) {
      this.serviceType = serviceType;
      this.callbackHandle = callbackHandle;
      this.contextHandle = contextHandle;
    }

    @Override
    public void onStartDiscoveryFailed(String serviceType, int errorCode) {
      Log.w(TAG, "Failed to start discovery service '" + serviceType + "': " + errorCode);
    }

    @Override
    public void onDiscoveryStarted(String serviceType) {
      Log.i(TAG, "Started service '" + serviceType + "'");
    }

    @Override
    public void onServiceFound(NsdServiceInfo serviceInfo) {
      Log.i(TAG, "Found service '" + serviceInfo.getServiceName() + "'");
      serviceNameList.add(serviceInfo.getServiceName());
    }

    @Override
    public void onServiceLost(NsdServiceInfo serviceInfo) {
      Log.i(TAG, "Lost service '" + serviceType + "'");
    }

    @Override
    public void onStopDiscoveryFailed(String serviceType, int errorCode) {
      Log.w(TAG, "Failed to stop discovery service '" + serviceType + "': " + errorCode);
    }

    @Override
    public void onDiscoveryStopped(String serviceType) {
      Log.w(TAG, "Succeed to stop discovery service '" + serviceType);
    }

    public void handleServiceBrowse(ChipMdnsCallback chipMdnsCallback) {
      chipMdnsCallback.handleServiceBrowse(
          serviceNameList.toArray(new String[serviceNameList.size()]),
          serviceType,
          callbackHandle,
          contextHandle);
    }
  }
}
