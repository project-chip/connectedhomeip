/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

import chip.devicecontroller.model.AttributeWriteRequest;
import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.DataVersionFilter;
import chip.devicecontroller.model.InvokeElement;
import java.util.List;
import javax.annotation.Nullable;

public class ChipInteractionClient {
  static native void subscribe(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      List<DataVersionFilter> dataVersionFilters,
      int minInterval,
      int maxInterval,
      boolean keepSubscriptions,
      boolean isFabricFiltered,
      int imTimeoutMs,
      @Nullable Long eventMin,
      boolean isPeerICD);

  static native void read(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      List<DataVersionFilter> dataVersionFilters,
      boolean isFabricFiltered,
      int imTimeoutMs,
      @Nullable Long eventMin);

  static native void write(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<AttributeWriteRequest> attributeList,
      int timedRequestTimeoutMs,
      int imTimeoutMs);

  static native void invoke(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      InvokeElement invokeElement,
      int timedRequestTimeoutMs,
      int imTimeoutMs);

  static native void extendableInvoke(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<InvokeElement> invokeElementList,
      int timedRequestTimeoutMs,
      int imTimeoutMs);

  static native void shutdownSubscriptions(
      long deviceControllerPtr,
      @Nullable Integer fabricIndex,
      @Nullable Long peerNodeId,
      @Nullable Long subscriptionId);

  static native long getRemoteDeviceId(long devicePtr);

  static native int getFabricIndex(long devicePtr);
}
