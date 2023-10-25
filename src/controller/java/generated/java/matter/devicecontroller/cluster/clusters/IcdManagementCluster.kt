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
  companion object {
    const val CLUSTER_ID: UInt = 70u
  }

  fun registerClient(
    callback: RegisterClientResponseCallback,
    checkInNodeID: Long,
    monitoredSubject: Long,
    key: ByteArray,
    verificationKey: ByteArray?
  ) {
    // Implementation needs to be added here
  }

  fun registerClient(
    callback: RegisterClientResponseCallback,
    checkInNodeID: Long,
    monitoredSubject: Long,
    key: ByteArray,
    verificationKey: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun unregisterClient(
    callback: DefaultClusterCallback,
    checkInNodeID: Long,
    verificationKey: ByteArray?
  ) {
    // Implementation needs to be added here
  }

  fun unregisterClient(
    callback: DefaultClusterCallback,
    checkInNodeID: Long,
    verificationKey: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stayActiveRequest(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun stayActiveRequest(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface RegisterClientResponseCallback {
    fun onSuccess(ICDCounter: Long)

    fun onError(error: Exception)
  }

  interface RegisteredClientsAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.IcdManagementClusterMonitoringRegistrationStruct>?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readIdleModeDurationAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeIdleModeDurationAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readActiveModeDurationAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActiveModeDurationAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readActiveModeThresholdAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActiveModeThresholdAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRegisteredClientsAttribute(callback: RegisteredClientsAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readRegisteredClientsAttributeWithFabricFilter(
    callback: RegisteredClientsAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRegisteredClientsAttribute(
    callback: RegisteredClientsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readICDCounterAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeICDCounterAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClientsSupportedPerFabricAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClientsSupportedPerFabricAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
