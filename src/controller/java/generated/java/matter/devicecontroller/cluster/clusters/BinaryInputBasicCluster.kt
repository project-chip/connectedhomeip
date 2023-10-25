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

class BinaryInputBasicCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 15u
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

  fun readActiveTextAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeActiveTextAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeActiveTextAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeActiveTextAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDescriptionAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeDescriptionAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeDescriptionAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeDescriptionAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInactiveTextAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInactiveTextAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeInactiveTextAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInactiveTextAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOutOfServiceAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOutOfServiceAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeOutOfServiceAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOutOfServiceAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPolarityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePolarityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPresentValueAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePresentValueAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writePresentValueAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePresentValueAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readReliabilityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeReliabilityAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeReliabilityAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeReliabilityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStatusFlagsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeStatusFlagsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readApplicationTypeAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeApplicationTypeAttribute(
    callback: LongAttributeCallback,
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
