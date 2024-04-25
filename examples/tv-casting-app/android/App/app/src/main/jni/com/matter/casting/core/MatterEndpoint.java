/*
 *   Copyright (c) 2024 Project CHIP Authors
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
 */
package com.matter.casting.core;

import android.util.Log;
import chip.devicecontroller.ChipClusters;
import com.matter.casting.support.DeviceTypeStruct;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class MatterEndpoint implements Endpoint {
  private static final String TAG = MatterEndpoint.class.getSimpleName();
  private static final long MAX_WAIT_FOR_DEVICE_PROXY_MS = 5000;
  protected long _cppEndpoint;

  @Override
  public native int getId();

  @Override
  public native int getVendorId();

  @Override
  public native int getProductId();

  @Override
  public native List<DeviceTypeStruct> getDeviceTypeList();

  @Override
  public <T extends ChipClusters.BaseChipCluster> T getCluster(Class<T> clusterClass) {
    try {
      Constructor<T> constructor = clusterClass.getDeclaredConstructor(long.class, int.class);
      Long deviceProxy = getDeviceProxy();
      if (deviceProxy == null) {
        Log.e(TAG, "Could not get DeviceProxy while constructing cluster object");
        return null;
      }
      return constructor.newInstance(deviceProxy, getId());
    } catch (InstantiationException
        | IllegalAccessException
        | InvocationTargetException
        | NoSuchMethodException e) {
      Log.e(
          TAG,
          "Could not create cluster object for " + clusterClass.getSimpleName() + " exc: " + e);
      return null;
    }
  }

  @Override
  public native CastingPlayer getCastingPlayer();

  @Override
  public String toString() {
    return "MatterEndpoint{" + "id=" + getId() + '}';
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterEndpoint that = (MatterEndpoint) o;
    return getId() == that.getId();
  }

  @Override
  public int hashCode() {
    return Objects.hash(getId());
  }

  private Long getDeviceProxy() {
    CompletableFuture<Long> deviceProxyFuture = new CompletableFuture<>();
    getDeviceProxy(
        new MatterCallback<Long>() {
          @Override
          public void handle(Long deviceProxy) {
            deviceProxyFuture.complete(deviceProxy);
          }
        },
        new MatterCallback<MatterError>() {
          @Override
          public void handle(MatterError response) {
            deviceProxyFuture.completeExceptionally(
                new RuntimeException("Failed on getDeviceProxy: " + response));
          }
        });

    try {
      return deviceProxyFuture.get(MAX_WAIT_FOR_DEVICE_PROXY_MS, TimeUnit.MILLISECONDS);
    } catch (ExecutionException | InterruptedException | TimeoutException e) {
      Log.e(TAG, "Exception while waiting on getDeviceProxy future: " + e);
      return null;
    }
  }

  protected native void getDeviceProxy(
      MatterCallback<Long> successCallback, MatterCallback<MatterError> failureCallback);
}
