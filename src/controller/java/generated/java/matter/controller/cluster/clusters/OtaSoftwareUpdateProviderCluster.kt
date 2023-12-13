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

package matter.controller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
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
    timedInvokeTimeout: Duration? = null
  ): QueryImageResponse {
    val commandId: UInt = 0u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_DELAYED_ACTION_TIME: Int = 1
    var delayedActionTime_decoded: UInt? = null

    val TAG_IMAGE_U_R_I: Int = 2
    var imageURI_decoded: String? = null

    val TAG_SOFTWARE_VERSION: Int = 3
    var softwareVersion_decoded: UInt? = null

    val TAG_SOFTWARE_VERSION_STRING: Int = 4
    var softwareVersionString_decoded: String? = null

    val TAG_UPDATE_TOKEN: Int = 5
    var updateToken_decoded: ByteArray? = null

    val TAG_USER_CONSENT_NEEDED: Int = 6
    var userConsentNeeded_decoded: Boolean? = null

    val TAG_METADATA_FOR_REQUESTOR: Int = 7
    var metadataForRequestor_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DELAYED_ACTION_TIME)) {
        delayedActionTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_IMAGE_U_R_I)) {
        imageURI_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_SOFTWARE_VERSION)) {
        softwareVersion_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_SOFTWARE_VERSION_STRING)) {
        softwareVersionString_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_UPDATE_TOKEN)) {
        updateToken_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_USER_CONSENT_NEEDED)) {
        userConsentNeeded_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_METADATA_FOR_REQUESTOR)) {
        metadataForRequestor_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
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
    timedInvokeTimeout: Duration? = null
  ): ApplyUpdateResponse {
    val commandId: UInt = 2u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ACTION: Int = 0
    var action_decoded: UByte? = null

    val TAG_DELAYED_ACTION_TIME: Int = 1
    var delayedActionTime_decoded: UInt? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ACTION)) {
        action_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DELAYED_ACTION_TIME)) {
        delayedActionTime_decoded = tlvReader.getUInt(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (action_decoded == null) {
      throw IllegalStateException("action not found in TLV")
    }

    if (delayedActionTime_decoded == null) {
      throw IllegalStateException("delayedActionTime not found in TLV")
    }

    tlvReader.exitContainer()

    return ApplyUpdateResponse(action_decoded, delayedActionTime_decoded)
  }

  suspend fun notifyUpdateApplied(
    updateToken: ByteArray,
    softwareVersion: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 4u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65528u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
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
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65529u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
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
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID: UInt = 65530u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
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
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID: UInt = 65531u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
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
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 65532u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Featuremap attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 65533u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  companion object {
    private val logger = Logger.getLogger(OtaSoftwareUpdateProviderCluster::class.java.name)
    const val CLUSTER_ID: UInt = 41u
  }
}
