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

class MediaPlaybackCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class PlaybackResponse(val status: UByte, val data: String?)

  class StartTimeAttribute(val value: ULong?)

  class DurationAttribute(val value: ULong?)

  class SampledPositionAttribute(val value: MediaPlaybackClusterPlaybackPositionStruct?)

  class SeekRangeEndAttribute(val value: ULong?)

  class SeekRangeStartAttribute(val value: ULong?)

  class ActiveAudioTrackAttribute(val value: MediaPlaybackClusterTrackStruct?)

  class AvailableAudioTracksAttribute(val value: List<MediaPlaybackClusterTrackStruct>?)

  class ActiveTextTrackAttribute(val value: MediaPlaybackClusterTrackStruct?)

  class AvailableTextTracksAttribute(val value: List<MediaPlaybackClusterTrackStruct>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun play(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun pause(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 1u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun stop(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun startOver(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 3u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun previous(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun next(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 5u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun rewind(
    audioAdvanceUnmuted: Boolean?,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_AUDIO_ADVANCE_UNMUTED_REQ: Int = 0
    audioAdvanceUnmuted?.let {
      tlvWriter.put(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED_REQ), audioAdvanceUnmuted)
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

    val TAG_DATA: Int = 1
    val data_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
        tlvReader.getString(ContextSpecificTag(TAG_DATA))
      } else {
        null
      }
    tlvReader.exitContainer()

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun fastForward(
    audioAdvanceUnmuted: Boolean?,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId: UInt = 7u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_AUDIO_ADVANCE_UNMUTED_REQ: Int = 0
    audioAdvanceUnmuted?.let {
      tlvWriter.put(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED_REQ), audioAdvanceUnmuted)
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

    val TAG_DATA: Int = 1
    val data_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
        tlvReader.getString(ContextSpecificTag(TAG_DATA))
      } else {
        null
      }
    tlvReader.exitContainer()

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun skipForward(
    deltaPositionMilliseconds: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId: UInt = 8u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_DELTA_POSITION_MILLISECONDS_REQ: Int = 0
    tlvWriter.put(
      ContextSpecificTag(TAG_DELTA_POSITION_MILLISECONDS_REQ),
      deltaPositionMilliseconds
    )
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun skipBackward(
    deltaPositionMilliseconds: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId: UInt = 9u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_DELTA_POSITION_MILLISECONDS_REQ: Int = 0
    tlvWriter.put(
      ContextSpecificTag(TAG_DELTA_POSITION_MILLISECONDS_REQ),
      deltaPositionMilliseconds
    )
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun seek(position: ULong, timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId: UInt = 11u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_POSITION_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_POSITION_REQ), position)
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

    return PlaybackResponse(status_decoded, data_decoded)
  }

  suspend fun activateAudioTrack(
    trackID: String,
    audioOutputIndex: UByte,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 12u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_TRACK_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_TRACK_I_D_REQ), trackID)

    val TAG_AUDIO_OUTPUT_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_AUDIO_OUTPUT_INDEX_REQ), audioOutputIndex)
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

  suspend fun activateTextTrack(trackID: String, timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 13u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_TRACK_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_TRACK_I_D_REQ), trackID)
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

  suspend fun deactivateTextTrack(timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 14u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

  suspend fun readCurrentStateAttribute(): UByte {
    val ATTRIBUTE_ID_CURRENTSTATE: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTSTATE
        }

      requireNotNull(attributeData) { "Currentstate attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readStartTimeAttribute(): StartTimeAttribute {
    val ATTRIBUTE_ID_STARTTIME: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_STARTTIME
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_STARTTIME
        }

      requireNotNull(attributeData) { "Starttime attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            tlvReader.getULong(AnonymousTag)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return StartTimeAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDurationAttribute(): DurationAttribute {
    val ATTRIBUTE_ID_DURATION: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DURATION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DURATION
        }

      requireNotNull(attributeData) { "Duration attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            tlvReader.getULong(AnonymousTag)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return DurationAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSampledPositionAttribute(): SampledPositionAttribute {
    val ATTRIBUTE_ID_SAMPLEDPOSITION: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SAMPLEDPOSITION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SAMPLEDPOSITION
        }

      requireNotNull(attributeData) { "Sampledposition attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: MediaPlaybackClusterPlaybackPositionStruct? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            MediaPlaybackClusterPlaybackPositionStruct.fromTlv(AnonymousTag, tlvReader)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return SampledPositionAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readPlaybackSpeedAttribute(): Float? {
    val ATTRIBUTE_ID_PLAYBACKSPEED: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_PLAYBACKSPEED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_PLAYBACKSPEED
        }

      requireNotNull(attributeData) { "Playbackspeed attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Float? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getFloat(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSeekRangeEndAttribute(): SeekRangeEndAttribute {
    val ATTRIBUTE_ID_SEEKRANGEEND: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SEEKRANGEEND
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SEEKRANGEEND
        }

      requireNotNull(attributeData) { "Seekrangeend attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            tlvReader.getULong(AnonymousTag)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return SeekRangeEndAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSeekRangeStartAttribute(): SeekRangeStartAttribute {
    val ATTRIBUTE_ID_SEEKRANGESTART: UInt = 6u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SEEKRANGESTART
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SEEKRANGESTART
        }

      requireNotNull(attributeData) { "Seekrangestart attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            tlvReader.getULong(AnonymousTag)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return SeekRangeStartAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActiveAudioTrackAttribute(): ActiveAudioTrackAttribute {
    val ATTRIBUTE_ID_ACTIVEAUDIOTRACK: UInt = 7u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEAUDIOTRACK
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEAUDIOTRACK
        }

      requireNotNull(attributeData) { "Activeaudiotrack attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: MediaPlaybackClusterTrackStruct? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            MediaPlaybackClusterTrackStruct.fromTlv(AnonymousTag, tlvReader)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return ActiveAudioTrackAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAvailableAudioTracksAttribute(): AvailableAudioTracksAttribute {
    val ATTRIBUTE_ID_AVAILABLEAUDIOTRACKS: UInt = 8u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVAILABLEAUDIOTRACKS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVAILABLEAUDIOTRACKS
        }

      requireNotNull(attributeData) { "Availableaudiotracks attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<MediaPlaybackClusterTrackStruct>? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            buildList<MediaPlaybackClusterTrackStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(MediaPlaybackClusterTrackStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return AvailableAudioTracksAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActiveTextTrackAttribute(): ActiveTextTrackAttribute {
    val ATTRIBUTE_ID_ACTIVETEXTTRACK: UInt = 9u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVETEXTTRACK
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVETEXTTRACK
        }

      requireNotNull(attributeData) { "Activetexttrack attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: MediaPlaybackClusterTrackStruct? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            MediaPlaybackClusterTrackStruct.fromTlv(AnonymousTag, tlvReader)
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return ActiveTextTrackAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAvailableTextTracksAttribute(): AvailableTextTracksAttribute {
    val ATTRIBUTE_ID_AVAILABLETEXTTRACKS: UInt = 10u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVAILABLETEXTTRACKS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVAILABLETEXTTRACKS
        }

      requireNotNull(attributeData) { "Availabletexttracks attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<MediaPlaybackClusterTrackStruct>? =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(AnonymousTag)) {
            buildList<MediaPlaybackClusterTrackStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(MediaPlaybackClusterTrackStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          } else {
            null
          }
        } else {
          tlvReader.getNull(AnonymousTag)
          null
        }

      return AvailableTextTracksAttribute(decodedValue)
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
    private val logger = Logger.getLogger(MediaPlaybackCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1286u
  }
}
