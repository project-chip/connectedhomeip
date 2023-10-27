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

class WindowCoveringCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 258u
  }

  fun upOrOpen(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun upOrOpen(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun downOrClose(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun downOrClose(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun stopMotion(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun stopMotion(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun goToLiftValue(callback: DefaultClusterCallback, liftValue: Integer) {
    // Implementation needs to be added here
  }

  fun goToLiftValue(
    callback: DefaultClusterCallback,
    liftValue: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun goToLiftPercentage(callback: DefaultClusterCallback, liftPercent100thsValue: Integer) {
    // Implementation needs to be added here
  }

  fun goToLiftPercentage(
    callback: DefaultClusterCallback,
    liftPercent100thsValue: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun goToTiltValue(callback: DefaultClusterCallback, tiltValue: Integer) {
    // Implementation needs to be added here
  }

  fun goToTiltValue(
    callback: DefaultClusterCallback,
    tiltValue: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun goToTiltPercentage(callback: DefaultClusterCallback, tiltPercent100thsValue: Integer) {
    // Implementation needs to be added here
  }

  fun goToTiltPercentage(
    callback: DefaultClusterCallback,
    tiltPercent100thsValue: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface CurrentPositionLiftAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPositionTiltAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPositionLiftPercentageAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPositionTiltPercentageAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TargetPositionLiftPercent100thsAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TargetPositionTiltPercent100thsAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPositionLiftPercent100thsAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface CurrentPositionTiltPercent100thsAttributeCallback {
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

  fun readTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalClosedLimitLiftAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalClosedLimitLiftAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPhysicalClosedLimitTiltAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePhysicalClosedLimitTiltAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionLiftAttribute(callback: CurrentPositionLiftAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionLiftAttribute(
    callback: CurrentPositionLiftAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionTiltAttribute(callback: CurrentPositionTiltAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionTiltAttribute(
    callback: CurrentPositionTiltAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfActuationsLiftAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfActuationsLiftAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfActuationsTiltAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfActuationsTiltAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readConfigStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeConfigStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionLiftPercentageAttribute(
    callback: CurrentPositionLiftPercentageAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionLiftPercentageAttribute(
    callback: CurrentPositionLiftPercentageAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionTiltPercentageAttribute(
    callback: CurrentPositionTiltPercentageAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionTiltPercentageAttribute(
    callback: CurrentPositionTiltPercentageAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperationalStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOperationalStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTargetPositionLiftPercent100thsAttribute(
    callback: TargetPositionLiftPercent100thsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeTargetPositionLiftPercent100thsAttribute(
    callback: TargetPositionLiftPercent100thsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTargetPositionTiltPercent100thsAttribute(
    callback: TargetPositionTiltPercent100thsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeTargetPositionTiltPercent100thsAttribute(
    callback: TargetPositionTiltPercent100thsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEndProductTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEndProductTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionLiftPercent100thsAttribute(
    callback: CurrentPositionLiftPercent100thsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionLiftPercent100thsAttribute(
    callback: CurrentPositionLiftPercent100thsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentPositionTiltPercent100thsAttribute(
    callback: CurrentPositionTiltPercent100thsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentPositionTiltPercent100thsAttribute(
    callback: CurrentPositionTiltPercent100thsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInstalledOpenLimitLiftAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInstalledOpenLimitLiftAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInstalledClosedLimitLiftAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInstalledClosedLimitLiftAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInstalledOpenLimitTiltAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInstalledOpenLimitTiltAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInstalledClosedLimitTiltAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeInstalledClosedLimitTiltAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSafetyStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSafetyStatusAttribute(
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
