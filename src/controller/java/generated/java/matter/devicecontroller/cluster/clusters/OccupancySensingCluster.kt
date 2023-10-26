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

class OccupancySensingCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 1030u
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

  fun readOccupancyAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupancyAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupancySensorTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupancySensorTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupancySensorTypeBitmapAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupancySensorTypeBitmapAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPIROccupiedToUnoccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePIROccupiedToUnoccupiedDelayAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writePIROccupiedToUnoccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePIROccupiedToUnoccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPIRUnoccupiedToOccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePIRUnoccupiedToOccupiedDelayAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writePIRUnoccupiedToOccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePIRUnoccupiedToOccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPIRUnoccupiedToOccupiedThresholdAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePIRUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writePIRUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePIRUnoccupiedToOccupiedThresholdAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUltrasonicOccupiedToUnoccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicOccupiedToUnoccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicOccupiedToUnoccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUltrasonicOccupiedToUnoccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUltrasonicUnoccupiedToOccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicUnoccupiedToOccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicUnoccupiedToOccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUltrasonicUnoccupiedToOccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUltrasonicUnoccupiedToOccupiedThresholdAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeUltrasonicUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUltrasonicUnoccupiedToOccupiedThresholdAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalContactOccupiedToUnoccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactOccupiedToUnoccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactOccupiedToUnoccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalContactOccupiedToUnoccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalContactUnoccupiedToOccupiedDelayAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactUnoccupiedToOccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactUnoccupiedToOccupiedDelayAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalContactUnoccupiedToOccupiedDelayAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalContactUnoccupiedToOccupiedThresholdAttribute(
    callback: IntegerAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writePhysicalContactUnoccupiedToOccupiedThresholdAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalContactUnoccupiedToOccupiedThresholdAttribute(
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
