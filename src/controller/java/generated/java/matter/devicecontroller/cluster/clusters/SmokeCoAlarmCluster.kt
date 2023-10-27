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

class SmokeCoAlarmCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 92u
  }

  fun selfTestRequest(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun selfTestRequest(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
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

  fun readExpressedStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeExpressedStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSmokeStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSmokeStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCOStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCOStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBatteryAlertAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBatteryAlertAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDeviceMutedAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDeviceMutedAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTestInProgressAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTestInProgressAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readHardwareFaultAlertAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeHardwareFaultAlertAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEndOfServiceAlertAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEndOfServiceAlertAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInterconnectSmokeAlarmAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInterconnectSmokeAlarmAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInterconnectCOAlarmAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInterconnectCOAlarmAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readContaminationStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeContaminationStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSmokeSensitivityLevelAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeSmokeSensitivityLevelAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeSmokeSensitivityLevelAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeSmokeSensitivityLevelAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readExpiryDateAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeExpiryDateAttribute(
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
