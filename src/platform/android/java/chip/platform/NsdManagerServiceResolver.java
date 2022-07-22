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
import java.util.List;

public class NsdManagerServiceResolver implements ServiceResolver {
  private static final String TAG = NsdManagerServiceResolver.class.getSimpleName();
  private static final long RESOLVE_SERVICE_TIMEOUT = 30000;
  private final NsdManager nsdManager;
  private MulticastLock multicastLock;
  private Handler mainThreadHandler;
  private List<NsdManager.RegistrationListener> registrationListeners = new ArrayList<>();

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
    Log.d(
        TAG,
        "resolve: Starting service resolution for '"
            + instanceName
            + "' type '"
            + serviceType
            + "'");

    Runnable timeoutRunnable =
        new Runnable() {
          @Override
          public void run() {
            // Ensure we always release the multicast lock. It's possible that we release the
            // multicast lock here before ResolveListener returns, but since NsdManager has no API
            // to cancel service resolution, there's not much we can do here.
            Log.d(TAG, "resolve: Timing out");
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
                instanceName, serviceType, null, null, 0, null, callbackHandle, contextHandle);

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
                serviceInfo.getHost().getHostName(),
                serviceInfo.getHost().getHostAddress(),
                serviceInfo.getPort(),
                serviceInfo.getAttributes(),
                callbackHandle,
                contextHandle);

            if (multicastLock.isHeld()) {
              multicastLock.release();
            }
            mainThreadHandler.removeCallbacks(timeoutRunnable);
          }
        });
    mainThreadHandler.postDelayed(timeoutRunnable, RESOLVE_SERVICE_TIMEOUT);
  }

  @Override
  public void publish(
      String serviceName,
      String hostName,
      String type,
      int port,
      String[] textEntriesKeys,
      byte[][] textEntriesDatas,
      String[] subTypes) {
    NsdServiceInfo serviceInfo = new NsdServiceInfo();
    serviceInfo.setServiceName(serviceName);

    /**
     * Note, subtypes registration is using an undocumented feature of android dns-sd
     * service/mDNSResponder which MAY STOP WORKING in future Android versions. Here, set type =
     * "${type},${subtypes1},${subtypes2},...", then subtypes1, subtypes2 etc are all registered to
     * this dns-sd server, we can usd `dns-sd -B ${type},${subtypes}` or avahi-browse
     * ${subtypes}._sub.${type} -r to browser it
     */
    StringBuilder sb = new StringBuilder(type);
    for (String subType : subTypes) {
      sb.append(",").append(subType);
    }
    serviceInfo.setServiceType(sb.toString());

    serviceInfo.setPort(port);
    Log.i(TAG, "publish serviceName=" + serviceName + " type=" + sb.toString() + " port=" + port);
    int cnt = Math.min(textEntriesDatas.length, textEntriesKeys.length);
    for (int i = 0; i < cnt; i++) {
      String value = new String(textEntriesDatas[i]);
      serviceInfo.setAttribute(textEntriesKeys[i], value);
      Log.i(TAG, "     " + textEntriesKeys[i] + "=" + value);
    }

    NsdManager.RegistrationListener registrationListener =
        new NsdManager.RegistrationListener() {
          @Override
          public void onRegistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
            Log.w(
                TAG,
                "service " + serviceInfo.getServiceName() + " onRegistrationFailed:" + errorCode);
          }

          @Override
          public void onUnregistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
            Log.w(
                TAG,
                "service " + serviceInfo.getServiceName() + " onUnregistrationFailed:" + errorCode);
          }

          @Override
          public void onServiceRegistered(NsdServiceInfo serviceInfo) {
            Log.i(
                TAG,
                "service " + serviceInfo.getServiceName() + "(" + this + ") onServiceRegistered");
          }

          @Override
          public void onServiceUnregistered(NsdServiceInfo serviceInfo) {
            Log.i(
                TAG,
                "service " + serviceInfo.getServiceName() + "(" + this + ") onServiceUnregistered");
          }
        };
    registrationListeners.add(registrationListener);

    nsdManager.registerService(serviceInfo, NsdManager.PROTOCOL_DNS_SD, registrationListener);
    Log.d(TAG, "publish " + registrationListener + " count = " + registrationListeners.size());
  }

  @Override
  public void removeServices() {
    Log.d(TAG, "removeServices: ");
    for (NsdManager.RegistrationListener l : registrationListeners) {
      Log.i(TAG, "Remove " + l);
      nsdManager.unregisterService(l);
    }
    registrationListeners.clear();
  }
}
