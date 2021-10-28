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
import android.os.Message;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;

public class NsdManagerServiceBrowse implements ServiceBrowse {
  private static final String TAG = NsdManagerServiceBrowse.class.getSimpleName();
  private static final long DISCOVER_SERVICE_TIMEOUT = 3000;
  private final NsdManager nsdManager;
  private MulticastLock multicastLock;

  private HashMap<Long, NsdManagerDiscovery> callbackMap = new HashMap<>();

  public NsdManagerServiceBrowse(Context context) {
    this.nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);

    this.multicastLock = ((WifiManager) context.getSystemService(Context.WIFI_SERVICE))
        .createMulticastLock("chipDiscoverMulticastLock");
    this.multicastLock.setReferenceCounted(true);
  }

  @Override
  public void browse(final String serviceType, final long callbackHandle, final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    startDiscover(serviceType, callbackHandle, contextHandle, chipMdnsCallback);

    Message msg = new Message();
    msg.what = 0;
    msg.obj = callbackHandle;
    handler.sendMessageDelayed(msg, DISCOVER_SERVICE_TIMEOUT);
  }

  public void startDiscover(final String serviceType, final long callbackHandle, final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
    if (callbackMap.containsKey(callbackHandle)) {
      Log.d(TAG, "Invalid callbackHandle");
      return;
    }

    NsdManagerDiscovery discovery = new NsdManagerDiscovery(serviceType, callbackHandle, contextHandle,
        chipMdnsCallback);
    multicastLock.acquire();

    Log.d(TAG, "Starting service discovering for '" + serviceType + "'");

    this.nsdManager.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discovery);
    callbackMap.put(callbackHandle, discovery);
  }

  public void stopDiscover(final long callbackHandle) {
    if (!callbackMap.containsKey(callbackHandle)) {
      Log.d(TAG, "Invalid callbackHandle");
      return;
    }

    NsdManagerDiscovery discovery = callbackMap.remove(callbackHandle);
    if (multicastLock.isHeld()) {
      multicastLock.release();
    }

    this.nsdManager.stopServiceDiscovery(discovery);
    discovery.sendCallback();
  }

  private Handler handler = new Handler() {
    @Override
    public void handleMessage(Message msg) {
      long handle = ((Long) msg.obj).longValue();
      stopDiscover(handle);
      super.handleMessage(msg);
    }
  };

  public class NsdManagerDiscovery implements NsdManager.DiscoveryListener {
    private String serviceType;
    private long callbackHandle;
    private long contextHandle;
    private ChipMdnsCallback chipMdnsCallback;
    private ArrayList<String> serviceNameList = new ArrayList<>();

    public NsdManagerDiscovery(String serviceType, long callbackHandle, long contextHandle,
        ChipMdnsCallback chipMdnsCallback) {
      this.serviceType = serviceType;
      this.callbackHandle = callbackHandle;
      this.contextHandle = contextHandle;
      this.chipMdnsCallback = chipMdnsCallback;
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

    public void sendCallback() {
      chipMdnsCallback.handleServiceBrowse(serviceNameList.toArray(new String[serviceNameList.size()]), serviceType,
          callbackHandle, contextHandle);
    }
  }
}
