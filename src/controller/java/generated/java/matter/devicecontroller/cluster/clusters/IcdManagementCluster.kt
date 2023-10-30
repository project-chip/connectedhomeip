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

class IcdManagementCluster(private val endpointId: UShort) {
  class RegisterClientResponse(val ICDCounter: UInt)

  class RegisteredClientsAttribute(
    val value: ArrayList<ChipStructs.IcdManagementClusterMonitoringRegistrationStruct>?
  )

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun registerClient(
    checkInNodeID: ULong,
    monitoredSubject: ULong,
    key: ByteArray,
    verificationKey: ByteArray?
  ): RegisterClientResponse {
    // Implementation needs to be added here
  }

  suspend fun registerClient(
    checkInNodeID: ULong,
    monitoredSubject: ULong,
    key: ByteArray,
    verificationKey: ByteArray?,
    timedInvokeTimeoutMs: Int
  ): RegisterClientResponse {
    // Implementation needs to be added here
  }

  suspend fun unregisterClient(checkInNodeID: ULong, verificationKey: ByteArray?) {
    // Implementation needs to be added here
  }

  suspend fun unregisterClient(
    checkInNodeID: ULong,
    verificationKey: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun stayActiveRequest() {
    // Implementation needs to be added here
  }

  suspend fun stayActiveRequest(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun readIdleModeDurationAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeIdleModeDurationAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readActiveModeDurationAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveModeDurationAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readActiveModeThresholdAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveModeThresholdAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRegisteredClientsAttribute(): RegisteredClientsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRegisteredClientsAttributeWithFabricFilter(
    isFabricFiltered: Boolean
  ): RegisteredClientsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeRegisteredClientsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): RegisteredClientsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readICDCounterAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeICDCounterAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClientsSupportedPerFabricAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClientsSupportedPerFabricAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readUserActiveModeTriggerHintAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeUserActiveModeTriggerHintAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readUserActiveModeTriggerInstructionAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeUserActiveModeTriggerInstructionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
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

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 70u
  }
}
