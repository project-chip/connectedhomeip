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

class OperationalCredentialsCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 62u
  }

  fun attestationRequest(callback: AttestationResponseCallback, attestationNonce: ByteArray) {
    // Implementation needs to be added here
  }

  fun attestationRequest(
    callback: AttestationResponseCallback,
    attestationNonce: ByteArray,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun certificateChainRequest(
    callback: CertificateChainResponseCallback,
    certificateType: Integer
  ) {
    // Implementation needs to be added here
  }

  fun certificateChainRequest(
    callback: CertificateChainResponseCallback,
    certificateType: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun CSRRequest(callback: CSRResponseCallback, CSRNonce: ByteArray, isForUpdateNOC: Boolean?) {
    // Implementation needs to be added here
  }

  fun CSRRequest(
    callback: CSRResponseCallback,
    CSRNonce: ByteArray,
    isForUpdateNOC: Boolean?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun addNOC(
    callback: NOCResponseCallback,
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    IPKValue: ByteArray,
    caseAdminSubject: Long,
    adminVendorId: Integer
  ) {
    // Implementation needs to be added here
  }

  fun addNOC(
    callback: NOCResponseCallback,
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    IPKValue: ByteArray,
    caseAdminSubject: Long,
    adminVendorId: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun updateNOC(callback: NOCResponseCallback, NOCValue: ByteArray, ICACValue: ByteArray?) {
    // Implementation needs to be added here
  }

  fun updateNOC(
    callback: NOCResponseCallback,
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun updateFabricLabel(callback: NOCResponseCallback, label: String) {
    // Implementation needs to be added here
  }

  fun updateFabricLabel(callback: NOCResponseCallback, label: String, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun removeFabric(callback: NOCResponseCallback, fabricIndex: Integer) {
    // Implementation needs to be added here
  }

  fun removeFabric(callback: NOCResponseCallback, fabricIndex: Integer, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun addTrustedRootCertificate(callback: DefaultClusterCallback, rootCACertificate: ByteArray) {
    // Implementation needs to be added here
  }

  fun addTrustedRootCertificate(
    callback: DefaultClusterCallback,
    rootCACertificate: ByteArray,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface AttestationResponseCallback {
    fun onSuccess(attestationElements: ByteArray, attestationSignature: ByteArray)

    fun onError(error: Exception)
  }

  interface CertificateChainResponseCallback {
    fun onSuccess(certificate: ByteArray)

    fun onError(error: Exception)
  }

  interface CSRResponseCallback {
    fun onSuccess(NOCSRElements: ByteArray, attestationSignature: ByteArray)

    fun onError(error: Exception)
  }

  interface NOCResponseCallback {
    fun onSuccess(statusCode: Integer, fabricIndex: Integer?, debugText: String?)

    fun onError(error: Exception)
  }

  interface NOCsAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.OperationalCredentialsClusterNOCStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface FabricsAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.OperationalCredentialsClusterFabricDescriptorStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TrustedRootCertificatesAttributeCallback {
    fun onSuccess(value: ArrayList<ByteArray>)

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

  fun readNOCsAttribute(callback: NOCsAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readNOCsAttributeWithFabricFilter(
    callback: NOCsAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNOCsAttribute(callback: NOCsAttributeCallback, minInterval: Int, maxInterval: Int) {
    // Implementation needs to be added here
  }

  fun readFabricsAttribute(callback: FabricsAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readFabricsAttributeWithFabricFilter(
    callback: FabricsAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun subscribeFabricsAttribute(
    callback: FabricsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportedFabricsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportedFabricsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCommissionedFabricsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCommissionedFabricsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTrustedRootCertificatesAttribute(callback: TrustedRootCertificatesAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTrustedRootCertificatesAttribute(
    callback: TrustedRootCertificatesAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentFabricIndexAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentFabricIndexAttribute(
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
