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
  class CurrentLevelAttribute(val value: UByte?)

  class OnLevelAttribute(val value: UByte?)

  class OnTransitionTimeAttribute(val value: UShort?)

  class OffTransitionTimeAttribute(val value: UShort?)

  class DefaultMoveRateAttribute(val value: UByte?)

  class StartUpCurrentLevelAttribute(val value: UByte?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun moveToLevel(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun moveToLevel(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun move(moveMode: UInt, rate: UByte?, optionsMask: UInt, optionsOverride: UInt) {
    // Implementation needs to be added here
  }

  suspend fun move(
    moveMode: UInt,
    rate: UByte?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun step(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun step(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun stop(optionsMask: UInt, optionsOverride: UInt) {
    // Implementation needs to be added here
  }

  suspend fun stop(optionsMask: UInt, optionsOverride: UInt, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun moveToLevelWithOnOff(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun moveToLevelWithOnOff(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun moveWithOnOff(
    moveMode: UInt,
    rate: UByte?,
    optionsMask: UInt,
    optionsOverride: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun moveWithOnOff(
    moveMode: UInt,
    rate: UByte?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun stepWithOnOff(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun stepWithOnOff(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun stopWithOnOff(optionsMask: UInt, optionsOverride: UInt) {
    // Implementation needs to be added here
  }

  suspend fun stopWithOnOff(optionsMask: UInt, optionsOverride: UInt, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun moveToClosestFrequency(frequency: UShort) {
    // Implementation needs to be added here
  }

  suspend fun moveToClosestFrequency(frequency: UShort, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun readCurrentLevelAttribute(): CurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentLevelAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRemainingTimeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemainingTimeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMinLevelAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinLevelAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMaxLevelAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxLevelAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readCurrentFrequencyAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentFrequencyAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMinFrequencyAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinFrequencyAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMaxFrequencyAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxFrequencyAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOptionsAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOptionsAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeOptionsAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOptionsAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOnOffTransitionTimeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOnOffTransitionTimeAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOnOffTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnOffTransitionTimeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOnLevelAttribute(): OnLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOnLevelAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeOnLevelAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnLevelAttribute(minInterval: Int, maxInterval: Int): OnLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOnTransitionTimeAttribute(): OnTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOnTransitionTimeAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOnTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnTransitionTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OnTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOffTransitionTimeAttribute(): OffTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOffTransitionTimeAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOffTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOffTransitionTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OffTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDefaultMoveRateAttribute(): DefaultMoveRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeDefaultMoveRateAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeDefaultMoveRateAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeDefaultMoveRateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): DefaultMoveRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStartUpCurrentLevelAttribute(): StartUpCurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpCurrentLevelAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpCurrentLevelAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeStartUpCurrentLevelAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StartUpCurrentLevelAttribute {
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
    const val CLUSTER_ID: UInt = 8u
  }
}
