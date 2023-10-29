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
  class AttestationResponse(
    val attestationElements: ByteArray,
    val attestationSignature: ByteArray
  )

  class CertificateChainResponse(val certificate: ByteArray)

  class CSRResponse(val NOCSRElements: ByteArray, val attestationSignature: ByteArray)

  class NOCResponse(val statusCode: UInt, val fabricIndex: UByte?, val debugText: String?)

  class NOCsAttribute(val value: ArrayList<ChipStructs.OperationalCredentialsClusterNOCStruct>)

  class FabricsAttribute(
    val value: ArrayList<ChipStructs.OperationalCredentialsClusterFabricDescriptorStruct>
  )

  class TrustedRootCertificatesAttribute(val value: ArrayList<ByteArray>)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun attestationRequest(
    attestationNonce: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ): AttestationResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun certificateChainRequest(
    certificateType: UInt,
    timedInvokeTimeoutMs: Int? = null
  ): CertificateChainResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun CSRRequest(
    CSRNonce: ByteArray,
    isForUpdateNOC: Boolean?,
    timedInvokeTimeoutMs: Int? = null
  ): CSRResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addNOC(
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    IPKValue: ByteArray,
    caseAdminSubject: ULong,
    adminVendorId: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): NOCResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun updateNOC(
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): NOCResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun updateFabricLabel(label: String, timedInvokeTimeoutMs: Int? = null): NOCResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun removeFabric(fabricIndex: UByte, timedInvokeTimeoutMs: Int? = null): NOCResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addTrustedRootCertificate(
    rootCACertificate: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readNOCsAttribute(): NOCsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNOCsAttributeWithFabricFilter(isFabricFiltered: Boolean): NOCsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNOCsAttribute(minInterval: Int, maxInterval: Int): NOCsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFabricsAttribute(): FabricsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFabricsAttributeWithFabricFilter(isFabricFiltered: Boolean): FabricsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeFabricsAttribute(minInterval: Int, maxInterval: Int): FabricsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupportedFabricsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedFabricsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCommissionedFabricsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCommissionedFabricsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readTrustedRootCertificatesAttribute(): TrustedRootCertificatesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeTrustedRootCertificatesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TrustedRootCertificatesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentFabricIndexAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentFabricIndexAttribute(minInterval: Int, maxInterval: Int): UByte {
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

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 62u
  }
}
