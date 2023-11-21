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

package matter.devicecontroller.cluster.clusters

import matter.controller.MatterController
import matter.devicecontroller.cluster.structs.*

class SmokeCoAlarmCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun selfTestRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readExpressedStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeExpressedStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSmokeStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeSmokeStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCOStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCOStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBatteryAlertAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatteryAlertAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readDeviceMutedAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeDeviceMutedAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readTestInProgressAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeTestInProgressAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readHardwareFaultAlertAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeHardwareFaultAlertAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readEndOfServiceAlertAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeEndOfServiceAlertAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readInterconnectSmokeAlarmAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeInterconnectSmokeAlarmAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readInterconnectCOAlarmAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeInterconnectCOAlarmAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readContaminationStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeContaminationStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSmokeSensitivityLevelAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeSmokeSensitivityLevelAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeSmokeSensitivityLevelAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readExpiryDateAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeExpiryDateAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 92u
  }
}
