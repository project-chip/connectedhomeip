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
import android.util.Log;
import androidx.annotation.Nullable;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class NsdManagerServiceResolver implements ServiceResolver {
  private static final String TAG = NsdManagerServiceResolver.class.getSimpleName();
  private static final long RESOLVE_SERVICE_TIMEOUT = 30000;
  private final NsdManager nsdManager;
  private MulticastLock multicastLock;
  private MulticastLock publishMulticastLock;
  private List<NsdManager.RegistrationListener> registrationListeners = new ArrayList<>();
  private final CopyOnWriteArrayList<String> mMFServiceName = new CopyOnWriteArrayList<>();
  @Nullable private final NsdManagerResolverAvailState nsdManagerResolverAvailState;
  private final long timeout;

  private ExecutorService mResolveExecutorService;

  /**
   * @param context application context
   * @param nsdManagerResolverAvailState Passing NsdManagerResolverAvailState allows
   *     NsdManagerServiceResolver to synchronize on the usage of NsdManager's resolveService() API
   * @param timeout Timeout value in case there is no response after calling resolve
   */
  public NsdManagerServiceResolver(
      Context context,
      @Nullable NsdManagerResolverAvailState nsdManagerResolverAvailState,
      long timeout) {
    this.nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);

    this.multicastLock =
        ((WifiManager) context.getSystemService(Context.WIFI_SERVICE))
            .createMulticastLock("chipMulticastLock");
    this.multicastLock.setReferenceCounted(true);

    this.publishMulticastLock =
        ((WifiManager) context.getSystemService(Context.WIFI_SERVICE))
            .createMulticastLock("chipPublishMulticastLock");
    this.publishMulticastLock.setReferenceCounted(true);

    this.nsdManagerResolverAvailState = nsdManagerResolverAvailState;
    this.timeout = timeout;

    mResolveExecutorService = Executors.newSingleThreadExecutor();
  }

  public NsdManagerServiceResolver(Context context) {
    this(context, null, RESOLVE_SERVICE_TIMEOUT);
  }

  public NsdManagerServiceResolver(
      Context context, @Nullable NsdManagerResolverAvailState nsdManagerResolverAvailState) {
    this(context, nsdManagerResolverAvailState, RESOLVE_SERVICE_TIMEOUT);
  }

  @Override
  public void resolve(
      final String instanceName,
      final String serviceType,
      final long callbackHandle,
      final long contextHandle,
      final ChipMdnsCallback chipMdnsCallback) {
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

            if (nsdManagerResolverAvailState != null) {
              nsdManagerResolverAvailState.signalFree();
            }
          }
        };

    mResolveExecutorService.execute(
        () -> {
          if (nsdManagerResolverAvailState != null) {
            nsdManagerResolverAvailState.acquireResolver();
          }

          ScheduledFuture<?> resolveTimeoutExecutor =
              Executors.newSingleThreadScheduledExecutor()
                  .schedule(timeoutRunnable, timeout, TimeUnit.MILLISECONDS);

          NsdServiceFinderAndResolver serviceFinderResolver =
              new NsdServiceFinderAndResolver(
                  this.nsdManager,
                  serviceInfo,
                  callbackHandle,
                  contextHandle,
                  chipMdnsCallback,
                  multicastLock,
                  resolveTimeoutExecutor,
                  nsdManagerResolverAvailState);
          serviceFinderResolver.start();
        });
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
    /**
     * Note, MF's NSDService will be repeatedly registered until it exceeds the OS's
     * maximum(http://androidxref.com/9.0.0_r3/xref/frameworks/base/services/core/java/com/android/server/NsdService.java#MAX_LIMIT)
     * limit at which time the registration will fail.
     */
    if (serviceName.contains("-") && mMFServiceName.contains(serviceName)) {
      Log.w(TAG, "publish: duplicate MF nsdService");
      return;
    }
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
    if (registrationListeners.size() == 0) {
      publishMulticastLock.acquire();
    }
    registrationListeners.add(registrationListener);
    mMFServiceName.add(serviceName);

    nsdManager.registerService(serviceInfo, NsdManager.PROTOCOL_DNS_SD, registrationListener);
    Log.d(TAG, "publish " + registrationListener + " count = " + registrationListeners.size());
  }

  @Override
  public void removeServices() {
    Log.d(TAG, "removeServices: ");
    if (registrationListeners.size() > 0 && publishMulticastLock.isHeld()) {
      publishMulticastLock.release();
    }
    for (NsdManager.RegistrationListener l : registrationListeners) {
      Log.i(TAG, "Remove " + l);
      nsdManager.unregisterService(l);
    }
    registrationListeners.clear();
    mMFServiceName.clear();
  }

  /**
   * The Android NsdManager calls back on the NsdManager.ResolveListener with a
   * FAILURE_ALREADY_ACTIVE(3) if any application code calls resolveService() on it while the
   * resolve operation is already active (from another call made previously). An object of
   * NsdManagerResolverAvailState allows NsdManagerServiceResolver to synchronize on the usage of
   * NsdManager's resolveService() API
   */
  public static class NsdManagerResolverAvailState {
    private static final String TAG = NsdManagerResolverAvailState.class.getSimpleName();

    private Lock lock = new ReentrantLock();
    private Condition condition = lock.newCondition();
    private boolean busy = false;

    /**
     * Waits if the NsdManager is already busy with resolving a service. Otherwise, it marks it as
     * busy and returns
     */
    public void acquireResolver() {
      lock.lock();
      try {
        while (busy) {
          Log.d(TAG, "Found NsdManager Resolver busy, waiting");
          condition.await();
        }
        Log.d(TAG, "Found NsdManager Resolver free, using it and marking it as busy");
        busy = true;
      } catch (InterruptedException e) {
        Log.e(TAG, "Failure while waiting for condition: " + e);
      } finally {
        lock.unlock();
      }
    }

    /** Signals the NsdManager resolver as free */
    public void signalFree() {
      lock.lock();
      try {
        Log.d(TAG, "Signaling NsdManager Resolver as free");
        busy = false;
        condition.signal();
      } finally {
        lock.unlock();
      }
    }
  }
}
