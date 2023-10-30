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

class OnOffCluster(private val endpointId: UShort) {
  class StartUpOnOffAttribute(val value: UInt?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun off() {
    // Implementation needs to be added here
  }

  suspend fun off(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun on() {
    // Implementation needs to be added here
  }

  suspend fun on(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun toggle() {
    // Implementation needs to be added here
  }

  suspend fun toggle(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun offWithEffect(effectIdentifier: UInt, effectVariant: UByte) {
    // Implementation needs to be added here
  }

  suspend fun offWithEffect(
    effectIdentifier: UInt,
    effectVariant: UByte,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun onWithRecallGlobalScene() {
    // Implementation needs to be added here
  }

  suspend fun onWithRecallGlobalScene(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun onWithTimedOff(onOffControl: UInt, onTime: UShort, offWaitTime: UShort) {
    // Implementation needs to be added here
  }

  suspend fun onWithTimedOff(
    onOffControl: UInt,
    onTime: UShort,
    offWaitTime: UShort,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun readOnOffAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnOffAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readGlobalSceneControlAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeGlobalSceneControlAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readOnTimeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOnTimeAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOnTimeAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnTimeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOffWaitTimeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOffWaitTimeAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOffWaitTimeAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOffWaitTimeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readStartUpOnOffAttribute(): StartUpOnOffAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpOnOffAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpOnOffAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeStartUpOnOffAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StartUpOnOffAttribute {
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
    const val CLUSTER_ID: UInt = 6u
  }
}
