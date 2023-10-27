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

class GeneralCommissioningCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 48u
  }

  fun armFailSafe(
    callback: ArmFailSafeResponseCallback,
    expiryLengthSeconds: Integer,
    breadcrumb: Long
  ) {
    // Implementation needs to be added here
  }

  fun armFailSafe(
    callback: ArmFailSafeResponseCallback,
    expiryLengthSeconds: Integer,
    breadcrumb: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setRegulatoryConfig(
    callback: SetRegulatoryConfigResponseCallback,
    newRegulatoryConfig: Integer,
    countryCode: String,
    breadcrumb: Long
  ) {
    // Implementation needs to be added here
  }

  fun setRegulatoryConfig(
    callback: SetRegulatoryConfigResponseCallback,
    newRegulatoryConfig: Integer,
    countryCode: String,
    breadcrumb: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun commissioningComplete(callback: CommissioningCompleteResponseCallback) {
    // Implementation needs to be added here
  }

  fun commissioningComplete(
    callback: CommissioningCompleteResponseCallback,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface ArmFailSafeResponseCallback {
    fun onSuccess(errorCode: Integer, debugText: String)

    fun onError(error: Exception)
  }

  interface SetRegulatoryConfigResponseCallback {
    fun onSuccess(errorCode: Integer, debugText: String)

    fun onError(error: Exception)
  }

  interface CommissioningCompleteResponseCallback {
    fun onSuccess(errorCode: Integer, debugText: String)

    fun onError(error: Exception)
  }

  interface BasicCommissioningInfoAttributeCallback {
    fun onSuccess(value: ChipStructs.GeneralCommissioningClusterBasicCommissioningInfo)

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

  fun readBreadcrumbAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBreadcrumbAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeBreadcrumbAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBreadcrumbAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBasicCommissioningInfoAttribute(callback: BasicCommissioningInfoAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeBasicCommissioningInfoAttribute(
    callback: BasicCommissioningInfoAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRegulatoryConfigAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRegulatoryConfigAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocationCapabilityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLocationCapabilityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportsConcurrentConnectionAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportsConcurrentConnectionAttribute(
    callback: BooleanAttributeCallback,
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
