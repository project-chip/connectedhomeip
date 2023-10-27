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

class ApplicationBasicCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 1293u
  }

  interface ApplicationAttributeCallback {
    fun onSuccess(value: ChipStructs.ApplicationBasicClusterApplicationStruct)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AllowedVendorListAttributeCallback {
    fun onSuccess(value: ArrayList<Integer>)

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

  fun readVendorNameAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeVendorNameAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readVendorIDAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeVendorIDAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readApplicationNameAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeApplicationNameAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductIDAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductIDAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readApplicationAttribute(callback: ApplicationAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeApplicationAttribute(
    callback: ApplicationAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStatusAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeStatusAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readApplicationVersionAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeApplicationVersionAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAllowedVendorListAttribute(callback: AllowedVendorListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAllowedVendorListAttribute(
    callback: AllowedVendorListAttributeCallback,
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
