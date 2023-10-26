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

class PumpConfigurationAndControlCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 512u
  }

  interface MaxPressureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxSpeedAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxFlowAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MinConstPressureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxConstPressureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MinCompPressureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxCompPressureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MinConstSpeedAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxConstSpeedAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MinConstFlowAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxConstFlowAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MinConstTempAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface MaxConstTempAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CapacityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SpeedAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LifetimeRunningHoursAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface PowerAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LifetimeEnergyConsumedAttributeCallback {
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

  fun readMaxPressureAttribute(callback: MaxPressureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxPressureAttribute(
    callback: MaxPressureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxSpeedAttribute(callback: MaxSpeedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxSpeedAttribute(
    callback: MaxSpeedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxFlowAttribute(callback: MaxFlowAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxFlowAttribute(
    callback: MaxFlowAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinConstPressureAttribute(callback: MinConstPressureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinConstPressureAttribute(
    callback: MinConstPressureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxConstPressureAttribute(callback: MaxConstPressureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxConstPressureAttribute(
    callback: MaxConstPressureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinCompPressureAttribute(callback: MinCompPressureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinCompPressureAttribute(
    callback: MinCompPressureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxCompPressureAttribute(callback: MaxCompPressureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxCompPressureAttribute(
    callback: MaxCompPressureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinConstSpeedAttribute(callback: MinConstSpeedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinConstSpeedAttribute(
    callback: MinConstSpeedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxConstSpeedAttribute(callback: MaxConstSpeedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxConstSpeedAttribute(
    callback: MaxConstSpeedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinConstFlowAttribute(callback: MinConstFlowAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinConstFlowAttribute(
    callback: MinConstFlowAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxConstFlowAttribute(callback: MaxConstFlowAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxConstFlowAttribute(
    callback: MaxConstFlowAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinConstTempAttribute(callback: MinConstTempAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinConstTempAttribute(
    callback: MinConstTempAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxConstTempAttribute(callback: MaxConstTempAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxConstTempAttribute(
    callback: MaxConstTempAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPumpStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePumpStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEffectiveOperationModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEffectiveOperationModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEffectiveControlModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEffectiveControlModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCapacityAttribute(callback: CapacityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCapacityAttribute(
    callback: CapacityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSpeedAttribute(callback: SpeedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSpeedAttribute(
    callback: SpeedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLifetimeRunningHoursAttribute(callback: LifetimeRunningHoursAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLifetimeRunningHoursAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeLifetimeRunningHoursAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLifetimeRunningHoursAttribute(
    callback: LifetimeRunningHoursAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPowerAttribute(callback: PowerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePowerAttribute(
    callback: PowerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLifetimeEnergyConsumedAttribute(callback: LifetimeEnergyConsumedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLifetimeEnergyConsumedAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeLifetimeEnergyConsumedAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLifetimeEnergyConsumedAttribute(
    callback: LifetimeEnergyConsumedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOperationModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOperationModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOperationModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readControlModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeControlModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeControlModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeControlModeAttribute(
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
