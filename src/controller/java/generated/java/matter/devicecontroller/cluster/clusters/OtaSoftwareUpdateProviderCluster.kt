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

class OtaSoftwareUpdateProviderCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 41u
  }

  fun queryImage(
    callback: QueryImageResponseCallback,
    vendorID: Integer,
    productID: Integer,
    softwareVersion: Long,
    protocolsSupported: ArrayList<Integer>,
    hardwareVersion: Integer?,
    location: String?,
    requestorCanConsent: Boolean?,
    metadataForProvider: ByteArray?
  ) {
    // Implementation needs to be added here
  }

  fun queryImage(
    callback: QueryImageResponseCallback,
    vendorID: Integer,
    productID: Integer,
    softwareVersion: Long,
    protocolsSupported: ArrayList<Integer>,
    hardwareVersion: Integer?,
    location: String?,
    requestorCanConsent: Boolean?,
    metadataForProvider: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun applyUpdateRequest(
    callback: ApplyUpdateResponseCallback,
    updateToken: ByteArray,
    newVersion: Long
  ) {
    // Implementation needs to be added here
  }

  fun applyUpdateRequest(
    callback: ApplyUpdateResponseCallback,
    updateToken: ByteArray,
    newVersion: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun notifyUpdateApplied(
    callback: DefaultClusterCallback,
    updateToken: ByteArray,
    softwareVersion: Long
  ) {
    // Implementation needs to be added here
  }

  fun notifyUpdateApplied(
    callback: DefaultClusterCallback,
    updateToken: ByteArray,
    softwareVersion: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface QueryImageResponseCallback {
    fun onSuccess(
      status: Integer,
      delayedActionTime: Long?,
      imageURI: String?,
      softwareVersion: Long?,
      softwareVersionString: String?,
      updateToken: ByteArray?,
      userConsentNeeded: Boolean?,
      metadataForRequestor: ByteArray?
    )

    fun onError(error: Exception)
  }

  interface ApplyUpdateResponseCallback {
    fun onSuccess(action: Integer, delayedActionTime: Long)

    fun onError(error: Exception)
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
