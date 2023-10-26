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

class LevelControlCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 8u
  }

  fun moveToLevel(
    callback: DefaultClusterCallback,
    level: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToLevel(
    callback: DefaultClusterCallback,
    level: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun move(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun move(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun step(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun step(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stop(callback: DefaultClusterCallback, optionsMask: Integer, optionsOverride: Integer) {
    // Implementation needs to be added here
  }

  fun stop(
    callback: DefaultClusterCallback,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToLevelWithOnOff(
    callback: DefaultClusterCallback,
    level: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToLevelWithOnOff(
    callback: DefaultClusterCallback,
    level: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveWithOnOff(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveWithOnOff(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stepWithOnOff(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stepWithOnOff(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer?,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stopWithOnOff(
    callback: DefaultClusterCallback,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stopWithOnOff(
    callback: DefaultClusterCallback,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToClosestFrequency(callback: DefaultClusterCallback, frequency: Integer) {
    // Implementation needs to be added here
  }

  fun moveToClosestFrequency(
    callback: DefaultClusterCallback,
    frequency: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface CurrentLevelAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OnLevelAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OnTransitionTimeAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OffTransitionTimeAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DefaultMoveRateAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface StartUpCurrentLevelAttributeCallback {
    fun onSuccess(value: Integer?)

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

  fun readCurrentLevelAttribute(callback: CurrentLevelAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentLevelAttribute(
    callback: CurrentLevelAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRemainingTimeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRemainingTimeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinLevelAttribute(callback: IntegerAttributeCallback) {
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

  fun subscribeMaxLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentFrequencyAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentFrequencyAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinFrequencyAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinFrequencyAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxFrequencyAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxFrequencyAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOptionsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOptionsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOptionsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOptionsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOnOffTransitionTimeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOnOffTransitionTimeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOnOffTransitionTimeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOnOffTransitionTimeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOnLevelAttribute(callback: OnLevelAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOnLevelAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOnLevelAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOnLevelAttribute(
    callback: OnLevelAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOnTransitionTimeAttribute(callback: OnTransitionTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOnTransitionTimeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOnTransitionTimeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOnTransitionTimeAttribute(
    callback: OnTransitionTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOffTransitionTimeAttribute(callback: OffTransitionTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOffTransitionTimeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOffTransitionTimeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOffTransitionTimeAttribute(
    callback: OffTransitionTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDefaultMoveRateAttribute(callback: DefaultMoveRateAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeDefaultMoveRateAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeDefaultMoveRateAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeDefaultMoveRateAttribute(
    callback: DefaultMoveRateAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStartUpCurrentLevelAttribute(callback: StartUpCurrentLevelAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeStartUpCurrentLevelAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeStartUpCurrentLevelAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeStartUpCurrentLevelAttribute(
    callback: StartUpCurrentLevelAttributeCallback,
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
