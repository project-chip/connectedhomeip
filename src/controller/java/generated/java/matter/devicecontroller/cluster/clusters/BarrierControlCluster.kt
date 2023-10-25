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
  companion object {
    const val CLUSTER_ID: UInt = 259u
  }

  fun barrierControlGoToPercent(callback: DefaultClusterCallback, percentOpen: Integer) {
    // Implementation needs to be added here
  }

  fun barrierControlGoToPercent(
    callback: DefaultClusterCallback,
    percentOpen: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun barrierControlStop(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun barrierControlStop(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
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

  fun readBarrierMovingStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierMovingStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierSafetyStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierSafetyStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierCapabilitiesAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierCapabilitiesAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierOpenEventsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierOpenEventsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierOpenEventsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierOpenEventsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierCloseEventsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierCloseEventsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierCloseEventsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierCloseEventsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierCommandOpenEventsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierCommandOpenEventsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierCommandOpenEventsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierCommandOpenEventsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierCommandCloseEventsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierCommandCloseEventsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierCommandCloseEventsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierCommandCloseEventsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierOpenPeriodAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierOpenPeriodAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierOpenPeriodAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierOpenPeriodAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierClosePeriodAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBarrierClosePeriodAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBarrierClosePeriodAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierClosePeriodAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBarrierPositionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBarrierPositionAttribute(
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
