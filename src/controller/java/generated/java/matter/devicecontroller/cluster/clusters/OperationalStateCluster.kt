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

class OperationalStateCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 96u
  }

  fun pause(callback: OperationalCommandResponseCallback) {
    // Implementation needs to be added here
  }

  fun pause(callback: OperationalCommandResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun stop(callback: OperationalCommandResponseCallback) {
    // Implementation needs to be added here
  }

  fun stop(callback: OperationalCommandResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun start(callback: OperationalCommandResponseCallback) {
    // Implementation needs to be added here
  }

  fun start(callback: OperationalCommandResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun resume(callback: OperationalCommandResponseCallback) {
    // Implementation needs to be added here
  }

  fun resume(callback: OperationalCommandResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface OperationalCommandResponseCallback {
    fun onSuccess(commandResponseState: ChipStructs.OperationalStateClusterErrorStateStruct)

    fun onError(error: Exception)
  }

  interface PhaseListAttributeCallback {
    fun onSuccess(value: ArrayList<String>?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPhaseAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CountdownTimeAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OperationalStateListAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.OperationalStateClusterOperationalStateStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OperationalErrorAttributeCallback {
    fun onSuccess(value: ChipStructs.OperationalStateClusterErrorStateStruct)

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

  fun readPhaseListAttribute(callback: PhaseListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePhaseListAttribute(
    callback: PhaseListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPhaseAttribute(callback: CurrentPhaseAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPhaseAttribute(
    callback: CurrentPhaseAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCountdownTimeAttribute(callback: CountdownTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCountdownTimeAttribute(
    callback: CountdownTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationalStateListAttribute(callback: OperationalStateListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOperationalStateListAttribute(
    callback: OperationalStateListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationalStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOperationalStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationalErrorAttribute(callback: OperationalErrorAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOperationalErrorAttribute(
    callback: OperationalErrorAttributeCallback,
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
