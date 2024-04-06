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

import chip.devicecontroller.model.AttributeState;
import chip.devicecontroller.model.AttributeWriteRequest;
import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.ClusterState;
import chip.devicecontroller.model.EndpointState;
import chip.devicecontroller.model.InvokeElement;
import chip.devicecontroller.model.NodeState;
import chip.devicecontroller.model.Status;

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

import static chip.devicecontroller.ChipTLVType.*;

public class ChipClusters {

  public interface BaseClusterCallback {
    void onError(Exception error);
  }

  public interface DefaultClusterCallback extends BaseClusterCallback {
    void onSuccess();
  }

  public interface BaseAttributeCallback {
    void onError(Exception error);
    default void onSubscriptionEstablished(long subscriptionId) {}
  }

  public interface CharStringAttributeCallback extends BaseAttributeCallback {
    /** Indicates a successful read for a CHAR_STRING attribute. */
    void onSuccess(String value);
  }

  public interface OctetStringAttributeCallback extends BaseAttributeCallback {
    /** Indicates a successful read for an OCTET_STRING attribute. */
    void onSuccess(byte[] value);
  }

  public interface IntegerAttributeCallback extends BaseAttributeCallback {
    void onSuccess(int value);
  }

  public interface LongAttributeCallback extends BaseAttributeCallback {
    void onSuccess(long value);
  }

  public interface BooleanAttributeCallback extends BaseAttributeCallback {
    void onSuccess(boolean value);
  }

  public interface FloatAttributeCallback extends BaseAttributeCallback {
    void onSuccess(float value);
  }

  public interface DoubleAttributeCallback extends BaseAttributeCallback {
    void onSuccess(double value);
  }

  public static abstract class BaseChipCluster {
    protected long chipClusterPtr;

    protected long devicePtr;
    protected int endpointId;
    protected long clusterId;

    private Optional<Long> timeoutMillis = Optional.empty();

    public BaseChipCluster(long devicePtr, int endpointId, long clusterId) {
      this.devicePtr = devicePtr;
      this.endpointId = endpointId;
      this.clusterId = clusterId;
    }

    /**
     * Sets the timeout, in milliseconds, after which commands sent through this cluster will fail
     * with a timeout (regardless of whether or not a response has been received). If set to an
     * empty optional, the default timeout will be used.
     */
    public void setCommandTimeout(Optional<Long> timeoutMillis) {
      this.timeoutMillis = timeoutMillis;
    }

    /** Returns the current timeout (in milliseconds) for commands sent through this cluster. */
    public Optional<Long> getCommandTimeout() {
      return timeoutMillis == null ? Optional.empty() : timeoutMillis;
    }

    @Deprecated
    public abstract long initWithDevice(long devicePtr, int endpointId);

    protected void readAttribute(
        ReportCallbackImpl callback,
        long attributeId,
        boolean isFabricFiltered) {
      ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, attributeId);
      ChipDeviceController.read(0, jniCallback.getCallbackHandle(), devicePtr, Arrays.asList(path), null, null, isFabricFiltered, timeoutMillis.orElse(0L).intValue(), null);
    }

    protected void writeAttribute(
        WriteAttributesCallbackImpl callback,
        long attributeId,
        BaseTLVType value,
        int timedRequestTimeoutMs) {
      WriteAttributesCallbackJni jniCallback = new WriteAttributesCallbackJni(callback);
      byte[] tlv = encodeToTlv(value);
      AttributeWriteRequest writeRequest = AttributeWriteRequest.newInstance(endpointId, clusterId, attributeId, tlv);
      ChipDeviceController.write(0, jniCallback.getCallbackHandle(), devicePtr, Arrays.asList(writeRequest), timedRequestTimeoutMs, timeoutMillis.orElse(0L).intValue());
    }

    protected void subscribeAttribute(
        ReportCallbackImpl callback,
        long attributeId,
        int minInterval,
        int maxInterval) {
      ReportCallbackJni jniCallback = new ReportCallbackJni(callback, callback, null);
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, attributeId);
      ChipDeviceController.subscribe(0, jniCallback.getCallbackHandle(), devicePtr, Arrays.asList(path), null, null, minInterval, maxInterval, false, true, timeoutMillis.orElse(0L).intValue(), null);
    }

    protected void invoke(
        InvokeCallbackImpl callback,
        long commandId,
        BaseTLVType value,
        int timedRequestTimeoutMs) {
      InvokeCallbackJni jniCallback = new InvokeCallbackJni(callback);
      byte[] tlv = encodeToTlv(value);
      InvokeElement element = InvokeElement.newInstance(endpointId, clusterId, commandId, tlv, null);
      ChipDeviceController.invoke(0, jniCallback.getCallbackHandle(), devicePtr, element, timedRequestTimeoutMs, timeoutMillis.orElse(0L).intValue());
    }

    private static native byte[] encodeToTlv(BaseTLVType value);

    static native BaseTLVType decodeFromTlv(byte[] tlv);

    @Deprecated
    public void deleteCluster(long chipClusterPtr) {}
    @SuppressWarnings("deprecation")
    protected void finalize() throws Throwable {
      super.finalize();

      if (chipClusterPtr != 0) {
        deleteCluster(chipClusterPtr);
        chipClusterPtr = 0;
      }
    }
  }

  abstract static class ReportCallbackImpl implements ReportCallback, SubscriptionEstablishedCallback {
    private BaseAttributeCallback callback;
    private ChipAttributePath path;

    private static final long CHIP_ERROR_UNSUPPORTED_ATTRIBUTE = 0x86;

    ReportCallbackImpl(BaseAttributeCallback callback, ChipAttributePath path) {
      this.callback = callback;
      this.path = path;
    }

    @Override
    public void onError(
        @Nullable ChipAttributePath attributePath,
        @Nullable ChipEventPath eventPath,
        @Nonnull Exception e) {
      callback.onError(e);
    }

    @Override
    public void onReport(NodeState nodeState) {
      if (nodeState == null) {
        callback.onError(new ChipClusterException());
        return;
      }

      EndpointState endpointState = nodeState.getEndpointState((int)path.getEndpointId().getId());
      if (endpointState == null) {
        callback.onError(new ChipClusterException(CHIP_ERROR_UNSUPPORTED_ATTRIBUTE));
        return;
      }

      ClusterState clusterState = endpointState.getClusterState(path.getClusterId().getId());
      if (clusterState == null) {
        callback.onError(new ChipClusterException(CHIP_ERROR_UNSUPPORTED_ATTRIBUTE));
        return;
      }

      AttributeState attributeState = clusterState.getAttributeState(path.getAttributeId().getId());
      if (attributeState == null) {
        callback.onError(new ChipClusterException(CHIP_ERROR_UNSUPPORTED_ATTRIBUTE));
        return;
      }

      byte[] tlv = attributeState.getTlv();
      if (tlv == null) {
          callback.onError(new ChipClusterException(CHIP_ERROR_UNSUPPORTED_ATTRIBUTE));
          return;
      }

      onSuccess(tlv);
    }

    @Override
    public void onSubscriptionEstablished(long subscriptionId) {
      callback.onSubscriptionEstablished(subscriptionId);
    }

    public abstract void onSuccess(byte[] tlv);
  }

  static class WriteAttributesCallbackImpl implements WriteAttributesCallback {
    private DefaultClusterCallback callback;

    WriteAttributesCallbackImpl(DefaultClusterCallback callback) {
      this.callback = callback;
    }

    @Override
    public void onResponse(ChipAttributePath attributePath, Status status) {
      if (status.getStatus() == Status.Code.Success)
      {
        callback.onSuccess();
      }
      else
      {
        callback.onError(new StatusException(status.getStatus()));
      }
    }

    @Override
    public void onError(@Nullable ChipAttributePath attributePath, Exception e) {
      callback.onError(e);
    }
  }

  abstract static class InvokeCallbackImpl implements InvokeCallback {
    private BaseClusterCallback callback;

    private static final long CHIP_ERROR_UNSUPPORTED_COMMAND = 0x81;

    InvokeCallbackImpl(BaseClusterCallback callback) {
      this.callback = callback;
    }

    public void onError(Exception e) {
      callback.onError(e);
    }

    public void onResponse(InvokeElement invokeElement, long successCode) {
      byte[] tlv = invokeElement.getTlvByteArray();
      if (tlv == null) {
        onResponse(null);
        return;
      }
      BaseTLVType value = BaseChipCluster.decodeFromTlv(tlv);
      if (value == null || value.type() != TLVType.Struct) {
        callback.onError(new ChipClusterException(CHIP_ERROR_UNSUPPORTED_COMMAND));
        return;
      }
      onResponse((StructType)value);
    }

    public abstract void onResponse(StructType value);
  }

  public static class FirstCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 1L;

    private static final long SOME_INTEGER_ATTRIBUTE_ID = 1L;

    public FirstCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId, CLUSTER_ID);
    }

    @Override
    @Deprecated
    public long initWithDevice(long devicePtr, int endpointId) {
      return 0L;
    }

    public void readSomeIntegerAttribute(
        IntegerAttributeCallback callback) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_INTEGER_ATTRIBUTE_ID);

      readAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
            callback.onSuccess(value);
          }
        }, SOME_INTEGER_ATTRIBUTE_ID, true);
    }

    public void writeSomeIntegerAttribute(DefaultClusterCallback callback, Integer value) {
      writeSomeIntegerAttribute(callback, value, 0);
    }

    public void writeSomeIntegerAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
      BaseTLVType tlvValue = new UIntType(value);
      writeAttribute(new WriteAttributesCallbackImpl(callback), SOME_INTEGER_ATTRIBUTE_ID, tlvValue, timedWriteTimeoutMs);
    }

    public void subscribeSomeIntegerAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_INTEGER_ATTRIBUTE_ID);

      subscribeAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
          }
        }, SOME_INTEGER_ATTRIBUTE_ID, minInterval, maxInterval);
    }
  }

  public static class SecondCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 2L;

    private static final long FABRICS_ATTRIBUTE_ID = 0L;
    private static final long SOME_BYTES_ATTRIBUTE_ID = 123L;

    public SecondCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId, CLUSTER_ID);
    }

    @Override
    @Deprecated
    public long initWithDevice(long devicePtr, int endpointId) {
      return 0L;
    }

    public interface FabricsAttributeCallback extends BaseAttributeCallback {
      void onSuccess(List<ChipStructs.SecondClusterFabricDescriptorStruct> value);
    }

    public void readFabricsAttribute(
        FabricsAttributeCallback callback) {
      readFabricsAttributeWithFabricFilter(callback, true);
    }

    public void readFabricsAttributeWithFabricFilter(
        FabricsAttributeCallback callback, boolean isFabricFiltered) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, FABRICS_ATTRIBUTE_ID);

      readAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            List<ChipStructs.SecondClusterFabricDescriptorStruct> value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
            callback.onSuccess(value);
          }
        }, FABRICS_ATTRIBUTE_ID, isFabricFiltered);
    }

    public void subscribeFabricsAttribute(
        FabricsAttributeCallback callback, int minInterval, int maxInterval) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, FABRICS_ATTRIBUTE_ID);

      subscribeAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            List<ChipStructs.SecondClusterFabricDescriptorStruct> value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
          }
        }, FABRICS_ATTRIBUTE_ID, minInterval, maxInterval);
    }

    public void readSomeBytesAttribute(
        OctetStringAttributeCallback callback) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_BYTES_ATTRIBUTE_ID);

      readAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            byte[] value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
            callback.onSuccess(value);
          }
        }, SOME_BYTES_ATTRIBUTE_ID, true);
    }

    public void subscribeSomeBytesAttribute(
        OctetStringAttributeCallback callback, int minInterval, int maxInterval) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_BYTES_ATTRIBUTE_ID);

      subscribeAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            byte[] value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
          }
        }, SOME_BYTES_ATTRIBUTE_ID, minInterval, maxInterval);
    }
  }

  public static class ThirdCluster extends BaseChipCluster {
    public static final long CLUSTER_ID = 3L;

    private static final long SOME_ENUM_ATTRIBUTE_ID = 10L;
    private static final long OPTIONS_ATTRIBUTE_ID = 20L;

    public ThirdCluster(long devicePtr, int endpointId) {
      super(devicePtr, endpointId, CLUSTER_ID);
    }

    @Override
    @Deprecated
    public long initWithDevice(long devicePtr, int endpointId) {
      return 0L;
    }

    public void readSomeEnumAttribute(
        IntegerAttributeCallback callback) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_ENUM_ATTRIBUTE_ID);

      readAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
            callback.onSuccess(value);
          }
        }, SOME_ENUM_ATTRIBUTE_ID, true);
    }

    public void writeSomeEnumAttribute(DefaultClusterCallback callback, Integer value) {
      writeSomeEnumAttribute(callback, value, 0);
    }

    public void writeSomeEnumAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
      BaseTLVType tlvValue = new UIntType(value);
      writeAttribute(new WriteAttributesCallbackImpl(callback), SOME_ENUM_ATTRIBUTE_ID, tlvValue, timedWriteTimeoutMs);
    }

    public void subscribeSomeEnumAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, SOME_ENUM_ATTRIBUTE_ID);

      subscribeAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
          }
        }, SOME_ENUM_ATTRIBUTE_ID, minInterval, maxInterval);
    }

    public void readOptionsAttribute(
        IntegerAttributeCallback callback) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, OPTIONS_ATTRIBUTE_ID);

      readAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
            callback.onSuccess(value);
          }
        }, OPTIONS_ATTRIBUTE_ID, true);
    }

    public void writeOptionsAttribute(DefaultClusterCallback callback, Integer value) {
      writeOptionsAttribute(callback, value, 0);
    }

    public void writeOptionsAttribute(DefaultClusterCallback callback, Integer value, int timedWriteTimeoutMs) {
      BaseTLVType tlvValue = new UIntType(value);
      writeAttribute(new WriteAttributesCallbackImpl(callback), OPTIONS_ATTRIBUTE_ID, tlvValue, timedWriteTimeoutMs);
    }

    public void subscribeOptionsAttribute(
        IntegerAttributeCallback callback, int minInterval, int maxInterval) {
      ChipAttributePath path = ChipAttributePath.newInstance(endpointId, clusterId, OPTIONS_ATTRIBUTE_ID);

      subscribeAttribute(new ReportCallbackImpl(callback, path) {
          @Override
          public void onSuccess(byte[] tlv) {
            Integer value = ChipTLVValueDecoder.decodeAttributeValue(path, tlv);
          }
        }, OPTIONS_ATTRIBUTE_ID, minInterval, maxInterval);
    }
  }
}
