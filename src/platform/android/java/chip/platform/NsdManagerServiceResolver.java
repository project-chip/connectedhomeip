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
import android.os.Build;
import android.util.Log;
import androidx.annotation.Nullable;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
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
  private final Object mRegistrationLock = new Object();
  private final Map<String, ActiveRegistration> mActiveRegistrations = new HashMap<>();
  private boolean mSetSubtypesUnavailableLogged = false;
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
    // Held whenever mActiveRegistrations is non-empty; tracked explicitly instead of via refcount.
    this.publishMulticastLock.setReferenceCounted(false);

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
    Map<String, String> txtAttributes = toTextEntriesMap(textEntriesKeys, textEntriesDatas);
    Set<String> subTypeSet = toSubTypesSet(subTypes);
    String mapKey = type + ":" + serviceName;

    synchronized (mRegistrationLock) {
      ActiveRegistration existing = mActiveRegistrations.get(mapKey);
      if (existing != null) {
        if (existing.matches(port, txtAttributes, subTypeSet)) {
          existing.pendingRemoval = false;
          Log.d(TAG, "publish: reusing active registration for " + mapKey);
          return;
        }
        Log.i(TAG, "publish: replacing changed registration for " + mapKey);
        unregisterServiceSafely(existing.listener, mapKey);
        mActiveRegistrations.remove(mapKey);
      }

      ActiveRegistration reg = null;
      try {
        NsdServiceInfo serviceInfo = new NsdServiceInfo();
        serviceInfo.setServiceName(serviceName);
        applyServiceTypeAndSubtypesLocked(serviceInfo, type, subTypeSet);
        serviceInfo.setPort(port);

        Log.i(
            TAG,
            "publish serviceName="
                + serviceName
                + " type="
                + serviceInfo.getServiceType()
                + " subTypes="
                + subTypeSet
                + " port="
                + port);
        for (Map.Entry<String, String> entry : txtAttributes.entrySet()) {
          serviceInfo.setAttribute(entry.getKey(), entry.getValue());
          Log.i(TAG, "     " + entry.getKey() + "=" + entry.getValue());
        }

        reg = new ActiveRegistration(mapKey, port, txtAttributes, subTypeSet);
        if (!publishMulticastLock.isHeld()) {
          publishMulticastLock.acquire();
        }
        mActiveRegistrations.put(mapKey, reg);

        nsdManager.registerService(serviceInfo, NsdManager.PROTOCOL_DNS_SD, reg.listener);
      } catch (RuntimeException e) {
        if (reg != null && mActiveRegistrations.get(mapKey) == reg) {
          mActiveRegistrations.remove(mapKey);
        }
        releasePublishLockIfEmptyLocked();
        throw e;
      }
      Log.d(TAG, "publish " + reg.listener + " count = " + mActiveRegistrations.size());
    }
  }

  private static Map<String, String> toTextEntriesMap(
      @Nullable String[] textEntriesKeys, @Nullable byte[][] textEntriesDatas) {
    Map<String, String> txtAttributes = new LinkedHashMap<>();
    if (textEntriesKeys == null && textEntriesDatas == null) {
      return txtAttributes;
    }
    if (textEntriesKeys == null
        || textEntriesDatas == null
        || textEntriesKeys.length != textEntriesDatas.length) {
      throw new IllegalArgumentException(
          "textEntriesKeys and textEntriesDatas must both be null or have equal length (got "
              + (textEntriesKeys == null ? "null" : textEntriesKeys.length)
              + " keys vs "
              + (textEntriesDatas == null ? "null" : textEntriesDatas.length)
              + " values)");
    }
    for (int i = 0; i < textEntriesKeys.length; i++) {
      if (textEntriesKeys[i] == null) {
        continue;
      }
      String value =
          textEntriesDatas[i] != null
              ? new String(textEntriesDatas[i], StandardCharsets.UTF_8)
              : "";
      txtAttributes.put(textEntriesKeys[i], value);
    }
    return txtAttributes;
  }

  private static Set<String> toSubTypesSet(@Nullable String[] subTypes) {
    Set<String> subTypeSet = new LinkedHashSet<>();
    if (subTypes != null) {
      for (String subType : subTypes) {
        if (subType != null && !subType.isEmpty()) {
          subTypeSet.add(subType);
        }
      }
    }
    return subTypeSet;
  }

  @Override
  public void removeServices() {
    synchronized (mRegistrationLock) {
      Log.d(
          TAG, "removeServices: marking " + mActiveRegistrations.size() + " services for removal");
      for (ActiveRegistration reg : mActiveRegistrations.values()) {
        reg.pendingRemoval = true;
      }
    }
  }

  @Override
  public void finalizeServiceUpdate() {
    synchronized (mRegistrationLock) {
      Log.d(TAG, "finalizeServiceUpdate: active count = " + mActiveRegistrations.size());
      Iterator<Map.Entry<String, ActiveRegistration>> it =
          mActiveRegistrations.entrySet().iterator();
      while (it.hasNext()) {
        Map.Entry<String, ActiveRegistration> entry = it.next();
        ActiveRegistration reg = entry.getValue();
        if (reg.pendingRemoval) {
          Log.i(TAG, "Remove " + reg.listener + " (" + entry.getKey() + ")");
          unregisterServiceSafely(reg.listener, entry.getKey());
          it.remove();
        }
      }
      releasePublishLockIfEmptyLocked();
    }
  }

  private void applyServiceTypeAndSubtypesLocked(
      NsdServiceInfo serviceInfo, String type, Set<String> subTypeSet) {
    if (Build.VERSION.SDK_INT >= 34) {
      serviceInfo.setServiceType(type);
      if (!subTypeSet.isEmpty()) {
        try {
          Method setSubtypesMethod = NsdServiceInfo.class.getMethod("setSubtypes", Set.class);
          setSubtypesMethod.invoke(serviceInfo, subTypeSet);
        } catch (NoSuchMethodException | IllegalAccessException e) {
          if (!mSetSubtypesUnavailableLogged) {
            mSetSubtypesUnavailableLogged = true;
            Log.w(
                TAG,
                "NsdServiceInfo.setSubtypes unavailable on SDK "
                    + Build.VERSION.SDK_INT
                    + ", publishing base service type only",
                e);
          }
        } catch (InvocationTargetException e) {
          Throwable cause = e.getCause() != null ? e.getCause() : e;
          Log.w(TAG, "NsdServiceInfo.setSubtypes failed for " + subTypeSet, cause);
        }
      }
    } else {
      // Note, on API < 34, subtypes registration uses an undocumented behavior of android dns-sd
      // service/mDNSResponder where setting type = "${type},${subtypes1},${subtypes2},..."
      // registers all subtypes.
      StringBuilder sb = new StringBuilder(type);
      for (String subType : subTypeSet) {
        sb.append(",").append(subType);
      }
      serviceInfo.setServiceType(sb.toString());
    }
  }

  private void unregisterServiceSafely(NsdManager.RegistrationListener listener, String mapKey) {
    try {
      nsdManager.unregisterService(listener);
    } catch (RuntimeException e) {
      Log.w(TAG, "unregisterService failed for " + mapKey, e);
    }
  }

  private void releasePublishLockIfEmptyLocked() {
    if (mActiveRegistrations.isEmpty() && publishMulticastLock.isHeld()) {
      publishMulticastLock.release();
    }
  }

  private void handleRegistrationFailed(String mapKey, ActiveRegistration reg) {
    synchronized (mRegistrationLock) {
      if (mActiveRegistrations.get(mapKey) == reg) {
        mActiveRegistrations.remove(mapKey);
        releasePublishLockIfEmptyLocked();
      }
    }
  }

  private class ActiveRegistration {
    final String mapKey;
    final int port;
    final Map<String, String> txtAttributes;
    final Set<String> subTypes;
    final NsdManager.RegistrationListener listener;
    boolean pendingRemoval = false;

    ActiveRegistration(
        String mapKey, int port, Map<String, String> txtAttributes, Set<String> subTypes) {
      this.mapKey = mapKey;
      this.port = port;
      this.txtAttributes = txtAttributes;
      this.subTypes = subTypes;
      this.listener =
          new NsdManager.RegistrationListener() {
            @Override
            public void onRegistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
              Log.w(
                  TAG,
                  "service "
                      + getLogServiceName(serviceInfo)
                      + " onRegistrationFailed:"
                      + errorCode);
              handleRegistrationFailed(mapKey, ActiveRegistration.this);
            }

            @Override
            public void onUnregistrationFailed(NsdServiceInfo serviceInfo, int errorCode) {
              Log.w(
                  TAG,
                  "service "
                      + getLogServiceName(serviceInfo)
                      + " onUnregistrationFailed:"
                      + errorCode);
            }

            @Override
            public void onServiceRegistered(NsdServiceInfo serviceInfo) {
              Log.i(
                  TAG,
                  "service "
                      + getLogServiceName(serviceInfo)
                      + "("
                      + this
                      + ") onServiceRegistered");
            }

            @Override
            public void onServiceUnregistered(NsdServiceInfo serviceInfo) {
              Log.i(
                  TAG,
                  "service "
                      + getLogServiceName(serviceInfo)
                      + "("
                      + this
                      + ") onServiceUnregistered");
            }
          };
    }

    private String getLogServiceName(NsdServiceInfo serviceInfo) {
      return serviceInfo != null ? serviceInfo.getServiceName() : mapKey;
    }

    boolean matches(int port, Map<String, String> txtAttributes, Set<String> subTypes) {
      return this.port == port
          && this.txtAttributes.equals(txtAttributes)
          && this.subTypes.equals(subTypes);
    }
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
