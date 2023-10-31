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

import java.util.ArrayList

class BarrierControlCluster(private val endpointId: UShort) {
  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun barrierControlGoToPercent(percentOpen: UByte, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun barrierControlStop(timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readBarrierMovingStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierMovingStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBarrierSafetyStatusAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierSafetyStatusAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierCapabilitiesAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierCapabilitiesAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBarrierOpenEventsAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierOpenEventsAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierOpenEventsAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierOpenEventsAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierCloseEventsAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCloseEventsAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCloseEventsAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierCloseEventsAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierCommandOpenEventsAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCommandOpenEventsAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCommandOpenEventsAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierCommandOpenEventsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierCommandCloseEventsAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCommandCloseEventsAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierCommandCloseEventsAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierCommandCloseEventsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierOpenPeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierOpenPeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierOpenPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierOpenPeriodAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierClosePeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierClosePeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeBarrierClosePeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierClosePeriodAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBarrierPositionAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBarrierPositionAttribute(minInterval: Int, maxInterval: Int): UByte {
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
    const val CLUSTER_ID: UInt = 259u
  }
}
