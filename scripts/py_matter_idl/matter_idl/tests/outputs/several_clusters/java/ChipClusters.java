/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package chip.devicecontroller;

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

public class ChipClusters {

  public interface DefaultClusterCallback {
    void onSuccess();
    void onError(Exception error);
  }

  public interface CharStringAttributeCallback {
    /** Indicates a successful read for a CHAR_STRING attribute. */
    void onSuccess(String value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface OctetStringAttributeCallback {
    /** Indicates a successful read for an OCTET_STRING attribute. */
    void onSuccess(byte[] value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface IntegerAttributeCallback {
    void onSuccess(int value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface LongAttributeCallback {
    void onSuccess(long value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface BooleanAttributeCallback {
    void onSuccess(boolean value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface FloatAttributeCallback {
    void onSuccess(float value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface DoubleAttributeCallback {
    void onSuccess(double value);
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public static abstract class BaseChipCluster {
    protected long chipClusterPtr;

    public BaseChipCluster(long devicePtr, int endpointId) {
      chipClusterPtr = initWithDevice(devicePtr, endpointId);
    }

    /**
     * Sets the timeout, in milliseconds, after which commands sent through this cluster will fail
     * with a timeout (regardless of whether or not a response has been received). If set to an
     * empty optional, the default timeout will be used.
     */
    public void setCommandTimeout(Optional<Long> timeoutMillis) {
      setCommandTimeout(chipClusterPtr, timeoutMillis);
    }

    private native void setCommandTimeout(long clusterPtr, Optional<Long> timeoutMillis);

    /** Returns the current timeout (in milliseconds) for commands sent through this cluster. */
    public Optional<Long> getCommandTimeout() {
      Optional<Long> timeout = getCommandTimeout(chipClusterPtr);
      return timeout == null ? Optional.empty() : timeout;
    }

    private native Optional<Long> getCommandTimeout(long clusterPtr);

    public abstract long initWithDevice(long devicePtr, int endpointId);

    public native void deleteCluster(long chipClusterPtr);
    
    @SuppressWarnings("deprecation")
    protected void finalize() throws Throwable {
      super.finalize();

      if (chipClusterPtr != 0) {
        deleteCluster(chipClusterPtr);
        chipClusterPtr = 0;
      }
    }
  }

  public static class FirstCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 1L;

    public FirstCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId);
    }

    @Override
    public native long initWithDevice(long devicePtr, int endpointId);

    public void readSomeIntegerAttribute(
        IntegerAttributeCallback callback) {
        readSomeIntegerAttribute(chipClusterPtr, callback);
    }

    public void writeSomeIntegerAttribute(DefaultClusterCallback callback, Integer value) {
        writeSomeIntegerAttribute(chipClusterPtr, callback, value, null);
    }

    public void writeSomeIntegerAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
        writeSomeIntegerAttribute(chipClusterPtr, callback, value, timedWriteTimeoutMs);
    }

    public void subscribeSomeIntegerAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
        subscribeSomeIntegerAttribute(chipClusterPtr, callback, minInterval, maxInterval);
    }

    private native void readSomeIntegerAttribute(long chipClusterPtr, IntegerAttributeCallback callback);

    private native void writeSomeIntegerAttribute(long chipClusterPtr, DefaultClusterCallback callback, Integer value, @Nullable Integer timedWriteTimeoutMs);

    private native void subscribeSomeIntegerAttribute(long chipClusterPtr, IntegerAttributeCallback callback, int minInterval, int maxInterval);
  }

  public static class SecondCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 2L;

    public SecondCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId);
    }

    @Override
    public native long initWithDevice(long devicePtr, int endpointId);

    public interface FabricsAttributeCallback {
      void onSuccess(List<ChipStructs.SecondClusterFabricDescriptorStruct> value);
      void onError(Exception ex);
      default void onSubscriptionEstablished(long subscriptionId) {}
    }

    public void readFabricsAttribute(
        FabricsAttributeCallback callback) {
        readFabricsAttribute(chipClusterPtr, callback, true);
    }

    public void readFabricsAttributeWithFabricFilter(
        FabricsAttributeCallback callback, boolean isFabricFiltered) {
        readFabricsAttribute(chipClusterPtr, callback, isFabricFiltered);
    }


    public void subscribeFabricsAttribute(
        FabricsAttributeCallback callback, int minInterval, int maxInterval) {
        subscribeFabricsAttribute(chipClusterPtr, callback, minInterval, maxInterval);
    }

    public void readSomeBytesAttribute(
        OctetStringAttributeCallback callback) {
        readSomeBytesAttribute(chipClusterPtr, callback);
    }

    public void subscribeSomeBytesAttribute(
        OctetStringAttributeCallback callback, int minInterval, int maxInterval) {
        subscribeSomeBytesAttribute(chipClusterPtr, callback, minInterval, maxInterval);
    }

    private native void readFabricsAttribute(long chipClusterPtr, FabricsAttributeCallback callback, boolean isFabricFiltered);

    private native void subscribeFabricsAttribute(long chipClusterPtr, FabricsAttributeCallback callback, int minInterval, int maxInterval);

    private native void readSomeBytesAttribute(long chipClusterPtr, OctetStringAttributeCallback callback);

    private native void subscribeSomeBytesAttribute(long chipClusterPtr, OctetStringAttributeCallback callback, int minInterval, int maxInterval);
  }

  public static class ThirdCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 3L;

    public ThirdCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId);
    }

    @Override
    public native long initWithDevice(long devicePtr, int endpointId);

    public void readSomeEnumAttribute(
        IntegerAttributeCallback callback) {
        readSomeEnumAttribute(chipClusterPtr, callback);
    }

    public void writeSomeEnumAttribute(DefaultClusterCallback callback, Integer value) {
        writeSomeEnumAttribute(chipClusterPtr, callback, value, null);
    }

    public void writeSomeEnumAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
        writeSomeEnumAttribute(chipClusterPtr, callback, value, timedWriteTimeoutMs);
    }

    public void subscribeSomeEnumAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
        subscribeSomeEnumAttribute(chipClusterPtr, callback, minInterval, maxInterval);
    }

    public void readOptionsAttribute(
        IntegerAttributeCallback callback) {
        readOptionsAttribute(chipClusterPtr, callback);
    }

    public void writeOptionsAttribute(DefaultClusterCallback callback, Integer value) {
        writeOptionsAttribute(chipClusterPtr, callback, value, null);
    }

    public void writeOptionsAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
        writeOptionsAttribute(chipClusterPtr, callback, value, timedWriteTimeoutMs);
    }

    public void subscribeOptionsAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
        subscribeOptionsAttribute(chipClusterPtr, callback, minInterval, maxInterval);
    }

    private native void readSomeEnumAttribute(long chipClusterPtr, IntegerAttributeCallback callback);

    private native void writeSomeEnumAttribute(long chipClusterPtr, DefaultClusterCallback callback, Integer value, @Nullable Integer timedWriteTimeoutMs);

    private native void subscribeSomeEnumAttribute(long chipClusterPtr, IntegerAttributeCallback callback, int minInterval, int maxInterval);

    private native void readOptionsAttribute(long chipClusterPtr, IntegerAttributeCallback callback);

    private native void writeOptionsAttribute(long chipClusterPtr, DefaultClusterCallback callback, Integer value, @Nullable Integer timedWriteTimeoutMs);

    private native void subscribeOptionsAttribute(long chipClusterPtr, IntegerAttributeCallback callback, int minInterval, int maxInterval);
  }
}
