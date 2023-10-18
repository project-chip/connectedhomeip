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

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.devicecontroller.cluster.structs.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OperationalCredentialsCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class AttestationResponse(
    val attestationElements: ByteArray,
    val attestationSignature: ByteArray
  )

  class CertificateChainResponse(val certificate: ByteArray)

  class CSRResponse(val NOCSRElements: ByteArray, val attestationSignature: ByteArray)

  class NOCResponse(val statusCode: UByte, val fabricIndex: UByte?, val debugText: String?)

  class NOCsAttribute(val value: List<OperationalCredentialsClusterNOCStruct>)

  class FabricsAttribute(val value: List<OperationalCredentialsClusterFabricDescriptorStruct>)

  class TrustedRootCertificatesAttribute(val value: List<ByteArray>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun attestationRequest(
    attestationNonce: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ): AttestationResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ATTESTATION_NONCE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ATTESTATION_NONCE_REQ), attestationNonce)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ATTESTATION_ELEMENTS: Int = 0
    val attestationElements_decoded =
      tlvReader.getByteArray(ContextSpecificTag(TAG_ATTESTATION_ELEMENTS))

    val TAG_ATTESTATION_SIGNATURE: Int = 1
    val attestationSignature_decoded =
      tlvReader.getByteArray(ContextSpecificTag(TAG_ATTESTATION_SIGNATURE))
    tlvReader.exitContainer()

    return AttestationResponse(attestationElements_decoded, attestationSignature_decoded)
  }

  suspend fun certificateChainRequest(
    certificateType: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): CertificateChainResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CERTIFICATE_TYPE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_CERTIFICATE_TYPE_REQ), certificateType)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_CERTIFICATE: Int = 0
    val certificate_decoded = tlvReader.getByteArray(ContextSpecificTag(TAG_CERTIFICATE))
    tlvReader.exitContainer()

    return CertificateChainResponse(certificate_decoded)
  }

  suspend fun CSRRequest(
    CSRNonce: ByteArray,
    isForUpdateNOC: Boolean?,
    timedInvokeTimeoutMs: Int? = null
  ): CSRResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_C_S_R_NONCE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_C_S_R_NONCE_REQ), CSRNonce)

    val TAG_IS_FOR_UPDATE_N_O_C_REQ: Int = 1
    isForUpdateNOC?.let {
      tlvWriter.put(ContextSpecificTag(TAG_IS_FOR_UPDATE_N_O_C_REQ), isForUpdateNOC)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_N_O_C_S_R_ELEMENTS: Int = 0
    val NOCSRElements_decoded = tlvReader.getByteArray(ContextSpecificTag(TAG_N_O_C_S_R_ELEMENTS))

    val TAG_ATTESTATION_SIGNATURE: Int = 1
    val attestationSignature_decoded =
      tlvReader.getByteArray(ContextSpecificTag(TAG_ATTESTATION_SIGNATURE))
    tlvReader.exitContainer()

    return CSRResponse(NOCSRElements_decoded, attestationSignature_decoded)
  }

  suspend fun addNOC(
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    IPKValue: ByteArray,
    caseAdminSubject: ULong,
    adminVendorId: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): NOCResponse {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_N_O_C_VALUE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_N_O_C_VALUE_REQ), NOCValue)

    val TAG_I_C_A_C_VALUE_REQ: Int = 1
    ICACValue?.let { tlvWriter.put(ContextSpecificTag(TAG_I_C_A_C_VALUE_REQ), ICACValue) }

    val TAG_I_P_K_VALUE_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_I_P_K_VALUE_REQ), IPKValue)

    val TAG_CASE_ADMIN_SUBJECT_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_CASE_ADMIN_SUBJECT_REQ), caseAdminSubject)

    val TAG_ADMIN_VENDOR_ID_REQ: Int = 4
    tlvWriter.put(ContextSpecificTag(TAG_ADMIN_VENDOR_ID_REQ), adminVendorId)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS_CODE: Int = 0
    val statusCode_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE))

    val TAG_FABRIC_INDEX: Int = 1
    val fabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_FABRIC_INDEX))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))
      } else {
        null
      }

    val TAG_DEBUG_TEXT: Int = 2
    val debugText_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEBUG_TEXT))) {
        tlvReader.getString(ContextSpecificTag(TAG_DEBUG_TEXT))
      } else {
        null
      }
    tlvReader.exitContainer()

    return NOCResponse(statusCode_decoded, fabricIndex_decoded, debugText_decoded)
  }

  suspend fun updateNOC(
    NOCValue: ByteArray,
    ICACValue: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): NOCResponse {
    val commandId: UInt = 7u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_N_O_C_VALUE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_N_O_C_VALUE_REQ), NOCValue)

    val TAG_I_C_A_C_VALUE_REQ: Int = 1
    ICACValue?.let { tlvWriter.put(ContextSpecificTag(TAG_I_C_A_C_VALUE_REQ), ICACValue) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS_CODE: Int = 0
    val statusCode_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE))

    val TAG_FABRIC_INDEX: Int = 1
    val fabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_FABRIC_INDEX))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))
      } else {
        null
      }

    val TAG_DEBUG_TEXT: Int = 2
    val debugText_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEBUG_TEXT))) {
        tlvReader.getString(ContextSpecificTag(TAG_DEBUG_TEXT))
      } else {
        null
      }
    tlvReader.exitContainer()

    return NOCResponse(statusCode_decoded, fabricIndex_decoded, debugText_decoded)
  }

  suspend fun updateFabricLabel(label: String, timedInvokeTimeoutMs: Int? = null): NOCResponse {
    val commandId: UInt = 9u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_LABEL_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_LABEL_REQ), label)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS_CODE: Int = 0
    val statusCode_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE))

    val TAG_FABRIC_INDEX: Int = 1
    val fabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_FABRIC_INDEX))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))
      } else {
        null
      }

    val TAG_DEBUG_TEXT: Int = 2
    val debugText_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEBUG_TEXT))) {
        tlvReader.getString(ContextSpecificTag(TAG_DEBUG_TEXT))
      } else {
        null
      }
    tlvReader.exitContainer()

    return NOCResponse(statusCode_decoded, fabricIndex_decoded, debugText_decoded)
  }

  suspend fun removeFabric(fabricIndex: UByte, timedInvokeTimeoutMs: Int? = null): NOCResponse {
    val commandId: UInt = 10u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_FABRIC_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX_REQ), fabricIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS_CODE: Int = 0
    val statusCode_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE))

    val TAG_FABRIC_INDEX: Int = 1
    val fabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_FABRIC_INDEX))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))
      } else {
        null
      }

    val TAG_DEBUG_TEXT: Int = 2
    val debugText_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEBUG_TEXT))) {
        tlvReader.getString(ContextSpecificTag(TAG_DEBUG_TEXT))
      } else {
        null
      }
    tlvReader.exitContainer()

    return NOCResponse(statusCode_decoded, fabricIndex_decoded, debugText_decoded)
  }

  suspend fun addTrustedRootCertificate(
    rootCACertificate: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 11u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ROOT_C_A_CERTIFICATE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ROOT_C_A_CERTIFICATE_REQ), rootCACertificate)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun readNOCsAttribute(): NOCsAttribute {
    val ATTRIBUTE_ID_NOCS: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NOCS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NOCS
        }

      requireNotNull(attributeData) { "Nocs attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<OperationalCredentialsClusterNOCStruct> =
        buildList<OperationalCredentialsClusterNOCStruct> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(OperationalCredentialsClusterNOCStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      return NOCsAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readFabricsAttribute(): FabricsAttribute {
    val ATTRIBUTE_ID_FABRICS: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FABRICS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FABRICS
        }

      requireNotNull(attributeData) { "Fabrics attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<OperationalCredentialsClusterFabricDescriptorStruct> =
        buildList<OperationalCredentialsClusterFabricDescriptorStruct> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(
              OperationalCredentialsClusterFabricDescriptorStruct.fromTlv(AnonymousTag, tlvReader)
            )
          }
          tlvReader.exitContainer()
        }

      return FabricsAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSupportedFabricsAttribute(): UByte {
    val ATTRIBUTE_ID_SUPPORTEDFABRICS: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SUPPORTEDFABRICS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SUPPORTEDFABRICS
        }

      requireNotNull(attributeData) { "Supportedfabrics attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCommissionedFabricsAttribute(): UByte {
    val ATTRIBUTE_ID_COMMISSIONEDFABRICS: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_COMMISSIONEDFABRICS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_COMMISSIONEDFABRICS
        }

      requireNotNull(attributeData) { "Commissionedfabrics attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readTrustedRootCertificatesAttribute(): TrustedRootCertificatesAttribute {
    val ATTRIBUTE_ID_TRUSTEDROOTCERTIFICATES: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TRUSTEDROOTCERTIFICATES
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TRUSTEDROOTCERTIFICATES
        }

      requireNotNull(attributeData) { "Trustedrootcertificates attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<ByteArray> =
        buildList<ByteArray> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      return TrustedRootCertificatesAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCurrentFabricIndexAttribute(): UByte {
    val ATTRIBUTE_ID_CURRENTFABRICINDEX: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTFABRICINDEX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTFABRICINDEX
        }

      requireNotNull(attributeData) { "Currentfabricindex attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID_GENERATEDCOMMANDLIST: UInt = 65528u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_GENERATEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_GENERATEDCOMMANDLIST
        }

      requireNotNull(attributeData) { "Generatedcommandlist attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<UInt> =
        buildList<UInt> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      return GeneratedCommandListAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST: UInt = 65529u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
        }

      requireNotNull(attributeData) { "Acceptedcommandlist attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<UInt> =
        buildList<UInt> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      return AcceptedCommandListAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID_EVENTLIST: UInt = 65530u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EVENTLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EVENTLIST
        }

      requireNotNull(attributeData) { "Eventlist attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<UInt> =
        buildList<UInt> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      return EventListAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID_ATTRIBUTELIST: UInt = 65531u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ATTRIBUTELIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ATTRIBUTELIST
        }

      requireNotNull(attributeData) { "Attributelist attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<UInt> =
        buildList<UInt> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      return AttributeListAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID_FEATUREMAP: UInt = 65532u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FEATUREMAP
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FEATUREMAP
        }

      requireNotNull(attributeData) { "Featuremap attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID_CLUSTERREVISION: UInt = 65533u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CLUSTERREVISION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CLUSTERREVISION
        }

      requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  companion object {
    private val logger = Logger.getLogger(OperationalCredentialsCluster::class.java.name)
    const val CLUSTER_ID: UInt = 62u
  }
}
