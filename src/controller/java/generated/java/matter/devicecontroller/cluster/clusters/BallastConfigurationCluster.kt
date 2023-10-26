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

class BallastConfigurationCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 769u
  }

  interface IntrinsicBallastFactorAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface BallastFactorAdjustmentAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LampRatedHoursAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LampBurnHoursAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LampBurnHoursTripPointAttributeCallback {
    fun onSuccess(value: Long?)

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

  fun readPhysicalMinLevelAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalMinLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalMaxLevelAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalMaxLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBallastStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBallastStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinLevelAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMinLevelAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMinLevelAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMinLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxLevelAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMaxLevelAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMaxLevelAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMaxLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readIntrinsicBallastFactorAttribute(callback: IntrinsicBallastFactorAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeIntrinsicBallastFactorAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeIntrinsicBallastFactorAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeIntrinsicBallastFactorAttribute(
    callback: IntrinsicBallastFactorAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBallastFactorAdjustmentAttribute(callback: BallastFactorAdjustmentAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBallastFactorAdjustmentAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBallastFactorAdjustmentAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBallastFactorAdjustmentAttribute(
    callback: BallastFactorAdjustmentAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampQuantityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLampQuantityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampTypeAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampTypeAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeLampTypeAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampTypeAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampManufacturerAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampManufacturerAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeLampManufacturerAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampManufacturerAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampRatedHoursAttribute(callback: LampRatedHoursAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampRatedHoursAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeLampRatedHoursAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampRatedHoursAttribute(
    callback: LampRatedHoursAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampBurnHoursAttribute(callback: LampBurnHoursAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampBurnHoursAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeLampBurnHoursAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampBurnHoursAttribute(
    callback: LampBurnHoursAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampAlarmModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampAlarmModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeLampAlarmModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampAlarmModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLampBurnHoursTripPointAttribute(callback: LampBurnHoursTripPointAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLampBurnHoursTripPointAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeLampBurnHoursTripPointAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLampBurnHoursTripPointAttribute(
    callback: LampBurnHoursTripPointAttributeCallback,
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
