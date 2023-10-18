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

class ChannelCluster(private val controller: MatterController, private val endpointId: UShort) {
  class ChangeChannelResponse(val status: UByte, val data: String?)

  class ProgramGuideResponse(
    val channelPagingStruct: Short,
    val programList: List<ChannelClusterProgramStruct>
  )

  class ChannelListAttribute(val value: List<ChannelClusterChannelInfoStruct>?)

  class LineupAttribute(val value: ChannelClusterLineupInfoStruct?)

  class CurrentChannelAttribute(val value: ChannelClusterChannelInfoStruct?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun changeChannel(
    match: String,
    timedInvokeTimeoutMs: Int? = null
  ): ChangeChannelResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_MATCH_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_MATCH_REQ), match)
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

    val TAG_DATA: Int = 1
    val data_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
        tlvReader.getString(ContextSpecificTag(TAG_DATA))
      } else {
        null
      }
    tlvReader.exitContainer()

    return ChangeChannelResponse(status_decoded, data_decoded)
  }

  suspend fun changeChannelByNumber(
    majorNumber: UShort,
    minorNumber: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_MAJOR_NUMBER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_MAJOR_NUMBER_REQ), majorNumber)

    val TAG_MINOR_NUMBER_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_MINOR_NUMBER_REQ), minorNumber)
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

  suspend fun skipChannel(count: Short, timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 3u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_COUNT_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_COUNT_REQ), count)
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

  suspend fun getProgramGuide(
    startTime: UInt?,
    endTime: UInt?,
    channelList: List<ChannelClusterChannelInfoStruct>?,
    pageToken: ChannelClusterPageTokenStruct?,
    recordingFlag: UInt?,
    externalIDList: List<ChannelClusterAdditionalInfoStruct>?,
    data: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): ProgramGuideResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_START_TIME_REQ: Int = 0
    startTime?.let { tlvWriter.put(ContextSpecificTag(TAG_START_TIME_REQ), startTime) }

    val TAG_END_TIME_REQ: Int = 1
    endTime?.let { tlvWriter.put(ContextSpecificTag(TAG_END_TIME_REQ), endTime) }

    val TAG_CHANNEL_LIST_REQ: Int = 2
    channelList?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_CHANNEL_LIST_REQ))
      for (item in channelList.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
    }

    val TAG_PAGE_TOKEN_REQ: Int = 3
    pageToken?.let { pageToken.toTlv(ContextSpecificTag(TAG_PAGE_TOKEN_REQ), tlvWriter) }

    val TAG_RECORDING_FLAG_REQ: Int = 4
    recordingFlag?.let { tlvWriter.put(ContextSpecificTag(TAG_RECORDING_FLAG_REQ), recordingFlag) }

    val TAG_EXTERNAL_I_D_LIST_REQ: Int = 5
    externalIDList?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST_REQ))
      for (item in externalIDList.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
    }

    val TAG_DATA_REQ: Int = 6
    data?.let { tlvWriter.put(ContextSpecificTag(TAG_DATA_REQ), data) }
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
    val TAG_CHANNEL_PAGING_STRUCT: Int = 0
    val channelPagingStruct_decoded =
      tlvReader.getShort(ContextSpecificTag(TAG_CHANNEL_PAGING_STRUCT))

    val TAG_PROGRAM_LIST: Int = 1
    val programList_decoded =
      buildList<ChannelClusterProgramStruct> {
        tlvReader.enterArray(ContextSpecificTag(TAG_PROGRAM_LIST))
        while (!tlvReader.isEndOfContainer()) {
          add(ChannelClusterProgramStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }
    tlvReader.exitContainer()

    return ProgramGuideResponse(channelPagingStruct_decoded, programList_decoded)
  }

  suspend fun recordProgram(
    programIdentifier: String,
    shouldRecordSeries: Boolean,
    externalIDList: List<ChannelClusterAdditionalInfoStruct>,
    data: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_PROGRAM_IDENTIFIER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_PROGRAM_IDENTIFIER_REQ), programIdentifier)

    val TAG_SHOULD_RECORD_SERIES_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SHOULD_RECORD_SERIES_REQ), shouldRecordSeries)

    val TAG_EXTERNAL_I_D_LIST_REQ: Int = 2
    tlvWriter.startArray(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST_REQ))
    for (item in externalIDList.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val TAG_DATA_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_DATA_REQ), data)
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

  suspend fun cancelRecordProgram(
    programIdentifier: String,
    shouldRecordSeries: Boolean,
    externalIDList: List<ChannelClusterAdditionalInfoStruct>,
    data: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 7u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_PROGRAM_IDENTIFIER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_PROGRAM_IDENTIFIER_REQ), programIdentifier)

    val TAG_SHOULD_RECORD_SERIES_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SHOULD_RECORD_SERIES_REQ), shouldRecordSeries)

    val TAG_EXTERNAL_I_D_LIST_REQ: Int = 2
    tlvWriter.startArray(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST_REQ))
    for (item in externalIDList.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val TAG_DATA_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_DATA_REQ), data)
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

  suspend fun readChannelListAttribute(): ChannelListAttribute {
    val ATTRIBUTE_ID_CHANNELLIST: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CHANNELLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CHANNELLIST
        }

      requireNotNull(attributeData) { "Channellist attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<ChannelClusterChannelInfoStruct>? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          buildList<ChannelClusterChannelInfoStruct> {
            tlvReader.enterArray(AnonymousTag)
            while (!tlvReader.isEndOfContainer()) {
              add(ChannelClusterChannelInfoStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          null
        }

      return ChannelListAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readLineupAttribute(): LineupAttribute {
    val ATTRIBUTE_ID_LINEUP: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LINEUP
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LINEUP
        }

      requireNotNull(attributeData) { "Lineup attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ChannelClusterLineupInfoStruct? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            ChannelClusterLineupInfoStruct.fromTlv(AnonymousTag, tlvReader)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return LineupAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCurrentChannelAttribute(): CurrentChannelAttribute {
    val ATTRIBUTE_ID_CURRENTCHANNEL: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTCHANNEL
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTCHANNEL
        }

      requireNotNull(attributeData) { "Currentchannel attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ChannelClusterChannelInfoStruct? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            ChannelClusterChannelInfoStruct.fromTlv(AnonymousTag, tlvReader)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return CurrentChannelAttribute(decodedValue)
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
    private val logger = Logger.getLogger(ChannelCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1284u
  }
}
