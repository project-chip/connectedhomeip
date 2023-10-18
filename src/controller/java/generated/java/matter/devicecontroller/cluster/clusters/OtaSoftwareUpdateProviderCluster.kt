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

class OtaSoftwareUpdateProviderCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class QueryImageResponse(
    val status: UByte,
    val delayedActionTime: UInt?,
    val imageURI: String?,
    val softwareVersion: UInt?,
    val softwareVersionString: String?,
    val updateToken: ByteArray?,
    val userConsentNeeded: Boolean?,
    val metadataForRequestor: ByteArray?
  )

  class ApplyUpdateResponse(val action: UByte, val delayedActionTime: UInt)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun queryImage(
    vendorID: UShort,
    productID: UShort,
    softwareVersion: UInt,
    protocolsSupported: List<UByte>,
    hardwareVersion: UShort?,
    location: String?,
    requestorCanConsent: Boolean?,
    metadataForProvider: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): QueryImageResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_VENDOR_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_VENDOR_I_D_REQ), vendorID)

    val TAG_PRODUCT_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_PRODUCT_I_D_REQ), productID)

    val TAG_SOFTWARE_VERSION_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_SOFTWARE_VERSION_REQ), softwareVersion)

    val TAG_PROTOCOLS_SUPPORTED_REQ: Int = 3
    tlvWriter.startArray(ContextSpecificTag(TAG_PROTOCOLS_SUPPORTED_REQ))
    for (item in protocolsSupported.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val TAG_HARDWARE_VERSION_REQ: Int = 4
    hardwareVersion?.let {
      tlvWriter.put(ContextSpecificTag(TAG_HARDWARE_VERSION_REQ), hardwareVersion)
    }

    val TAG_LOCATION_REQ: Int = 5
    location?.let { tlvWriter.put(ContextSpecificTag(TAG_LOCATION_REQ), location) }

    val TAG_REQUESTOR_CAN_CONSENT_REQ: Int = 6
    requestorCanConsent?.let {
      tlvWriter.put(ContextSpecificTag(TAG_REQUESTOR_CAN_CONSENT_REQ), requestorCanConsent)
    }

    val TAG_METADATA_FOR_PROVIDER_REQ: Int = 7
    metadataForProvider?.let {
      tlvWriter.put(ContextSpecificTag(TAG_METADATA_FOR_PROVIDER_REQ), metadataForProvider)
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
    val TAG_STATUS: Int = 0
    val status_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))

    val TAG_DELAYED_ACTION_TIME: Int = 1
    val delayedActionTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DELAYED_ACTION_TIME))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_DELAYED_ACTION_TIME))
      } else {
        null
      }

    val TAG_IMAGE_U_R_I: Int = 2
    val imageURI_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_IMAGE_U_R_I))) {
        tlvReader.getString(ContextSpecificTag(TAG_IMAGE_U_R_I))
      } else {
        null
      }

    val TAG_SOFTWARE_VERSION: Int = 3
    val softwareVersion_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOFTWARE_VERSION))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_SOFTWARE_VERSION))
      } else {
        null
      }

    val TAG_SOFTWARE_VERSION_STRING: Int = 4
    val softwareVersionString_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOFTWARE_VERSION_STRING))) {
        tlvReader.getString(ContextSpecificTag(TAG_SOFTWARE_VERSION_STRING))
      } else {
        null
      }

    val TAG_UPDATE_TOKEN: Int = 5
    val updateToken_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_UPDATE_TOKEN))) {
        tlvReader.getByteArray(ContextSpecificTag(TAG_UPDATE_TOKEN))
      } else {
        null
      }

    val TAG_USER_CONSENT_NEEDED: Int = 6
    val userConsentNeeded_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_CONSENT_NEEDED))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_USER_CONSENT_NEEDED))
      } else {
        null
      }

    val TAG_METADATA_FOR_REQUESTOR: Int = 7
    val metadataForRequestor_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_METADATA_FOR_REQUESTOR))) {
        tlvReader.getByteArray(ContextSpecificTag(TAG_METADATA_FOR_REQUESTOR))
      } else {
        null
      }
    tlvReader.exitContainer()

    return QueryImageResponse(
      status_decoded,
      delayedActionTime_decoded,
      imageURI_decoded,
      softwareVersion_decoded,
      softwareVersionString_decoded,
      updateToken_decoded,
      userConsentNeeded_decoded,
      metadataForRequestor_decoded
    )
  }

  suspend fun applyUpdateRequest(
    updateToken: ByteArray,
    newVersion: UInt,
    timedInvokeTimeoutMs: Int? = null
  ): ApplyUpdateResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_UPDATE_TOKEN_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_UPDATE_TOKEN_REQ), updateToken)

    val TAG_NEW_VERSION_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_NEW_VERSION_REQ), newVersion)
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
    val TAG_ACTION: Int = 0
    val action_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_ACTION))

    val TAG_DELAYED_ACTION_TIME: Int = 1
    val delayedActionTime_decoded = tlvReader.getUInt(ContextSpecificTag(TAG_DELAYED_ACTION_TIME))
    tlvReader.exitContainer()

    return ApplyUpdateResponse(action_decoded, delayedActionTime_decoded)
  }

  suspend fun notifyUpdateApplied(
    updateToken: ByteArray,
    softwareVersion: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_UPDATE_TOKEN_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_UPDATE_TOKEN_REQ), updateToken)

    val TAG_SOFTWARE_VERSION_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SOFTWARE_VERSION_REQ), softwareVersion)
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
    private val logger = Logger.getLogger(OtaSoftwareUpdateProviderCluster::class.java.name)
    const val CLUSTER_ID: UInt = 41u
  }
}
