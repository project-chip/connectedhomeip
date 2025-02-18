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
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.transform
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WebRTCTransportProviderCluster(
  private val controller: MatterController,
  private val endpointId: UShort,
) {
  class SolicitOfferResponse(
    val webRTCSessionID: UShort,
    val deferredOffer: Boolean,
    val videoStreamID: UShort?,
    val audioStreamID: UShort?,
  )

  class ProvideOfferResponse(
    val webRTCSessionID: UShort,
    val videoStreamID: UShort?,
    val audioStreamID: UShort?,
  )

  class CurrentSessionsAttribute(
    val value: List<WebRTCTransportProviderClusterWebRTCSessionStruct>
  )

  sealed class CurrentSessionsAttributeSubscriptionState {
    data class Success(val value: List<WebRTCTransportProviderClusterWebRTCSessionStruct>) :
      CurrentSessionsAttributeSubscriptionState()

    data class Error(val exception: Exception) : CurrentSessionsAttributeSubscriptionState()

    object SubscriptionEstablished : CurrentSessionsAttributeSubscriptionState()
  }

  class GeneratedCommandListAttribute(val value: List<UInt>)

  sealed class GeneratedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : GeneratedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : GeneratedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : GeneratedCommandListAttributeSubscriptionState()
  }

  class AcceptedCommandListAttribute(val value: List<UInt>)

  sealed class AcceptedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AcceptedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AcceptedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : AcceptedCommandListAttributeSubscriptionState()
  }

  class EventListAttribute(val value: List<UInt>)

  sealed class EventListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : EventListAttributeSubscriptionState()

    data class Error(val exception: Exception) : EventListAttributeSubscriptionState()

    object SubscriptionEstablished : EventListAttributeSubscriptionState()
  }

  class AttributeListAttribute(val value: List<UInt>)

  sealed class AttributeListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AttributeListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AttributeListAttributeSubscriptionState()

    object SubscriptionEstablished : AttributeListAttributeSubscriptionState()
  }

  suspend fun solicitOffer(
    streamUsage: UByte,
    videoStreamID: UShort?,
    audioStreamID: UShort?,
    ICEServers: List<WebRTCTransportProviderClusterICEServerStruct>?,
    ICETransportPolicy: String?,
    metadataOptions: UByte?,
    timedInvokeTimeout: Duration? = null,
  ): SolicitOfferResponse {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_STREAM_USAGE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_STREAM_USAGE_REQ), streamUsage)

    val TAG_VIDEO_STREAM_ID_REQ: Int = 1
    videoStreamID?.let { tlvWriter.put(ContextSpecificTag(TAG_VIDEO_STREAM_ID_REQ), videoStreamID) }

    val TAG_AUDIO_STREAM_ID_REQ: Int = 2
    audioStreamID?.let { tlvWriter.put(ContextSpecificTag(TAG_AUDIO_STREAM_ID_REQ), audioStreamID) }

    val TAG_ICE_SERVERS_REQ: Int = 3
    ICEServers?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_ICE_SERVERS_REQ))
      for (item in ICEServers.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
    }

    val TAG_ICE_TRANSPORT_POLICY_REQ: Int = 4
    ICETransportPolicy?.let {
      tlvWriter.put(ContextSpecificTag(TAG_ICE_TRANSPORT_POLICY_REQ), ICETransportPolicy)
    }

    val TAG_METADATA_OPTIONS_REQ: Int = 5
    metadataOptions?.let {
      tlvWriter.put(ContextSpecificTag(TAG_METADATA_OPTIONS_REQ), metadataOptions)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WEB_RTC_SESSION_ID: Int = 0
    var webRTCSessionID_decoded: UShort? = null

    val TAG_DEFERRED_OFFER: Int = 1
    var deferredOffer_decoded: Boolean? = null

    val TAG_VIDEO_STREAM_ID: Int = 2
    var videoStreamID_decoded: UShort? = null

    val TAG_AUDIO_STREAM_ID: Int = 3
    var audioStreamID_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_WEB_RTC_SESSION_ID)) {
        webRTCSessionID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEFERRED_OFFER)) {
        deferredOffer_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_VIDEO_STREAM_ID)) {
        videoStreamID_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(tag)) {
                tlvReader.getUShort(tag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_AUDIO_STREAM_ID)) {
        audioStreamID_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(tag)) {
                tlvReader.getUShort(tag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (webRTCSessionID_decoded == null) {
      throw IllegalStateException("webRTCSessionID not found in TLV")
    }

    if (deferredOffer_decoded == null) {
      throw IllegalStateException("deferredOffer not found in TLV")
    }

    tlvReader.exitContainer()

    return SolicitOfferResponse(
      webRTCSessionID_decoded,
      deferredOffer_decoded,
      videoStreamID_decoded,
      audioStreamID_decoded,
    )
  }

  suspend fun provideOffer(
    webRTCSessionID: UShort?,
    sdp: String,
    streamUsage: UByte,
    videoStreamID: UShort?,
    audioStreamID: UShort?,
    ICEServers: List<WebRTCTransportProviderClusterICEServerStruct>?,
    ICETransportPolicy: String?,
    metadataOptions: UByte?,
    timedInvokeTimeout: Duration? = null,
  ): ProvideOfferResponse {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEB_RTC_SESSION_ID_REQ: Int = 0
    webRTCSessionID?.let {
      tlvWriter.put(ContextSpecificTag(TAG_WEB_RTC_SESSION_ID_REQ), webRTCSessionID)
    }

    val TAG_SDP_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SDP_REQ), sdp)

    val TAG_STREAM_USAGE_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_STREAM_USAGE_REQ), streamUsage)

    val TAG_VIDEO_STREAM_ID_REQ: Int = 3
    videoStreamID?.let { tlvWriter.put(ContextSpecificTag(TAG_VIDEO_STREAM_ID_REQ), videoStreamID) }

    val TAG_AUDIO_STREAM_ID_REQ: Int = 4
    audioStreamID?.let { tlvWriter.put(ContextSpecificTag(TAG_AUDIO_STREAM_ID_REQ), audioStreamID) }

    val TAG_ICE_SERVERS_REQ: Int = 5
    ICEServers?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_ICE_SERVERS_REQ))
      for (item in ICEServers.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
    }

    val TAG_ICE_TRANSPORT_POLICY_REQ: Int = 6
    ICETransportPolicy?.let {
      tlvWriter.put(ContextSpecificTag(TAG_ICE_TRANSPORT_POLICY_REQ), ICETransportPolicy)
    }

    val TAG_METADATA_OPTIONS_REQ: Int = 7
    metadataOptions?.let {
      tlvWriter.put(ContextSpecificTag(TAG_METADATA_OPTIONS_REQ), metadataOptions)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WEB_RTC_SESSION_ID: Int = 0
    var webRTCSessionID_decoded: UShort? = null

    val TAG_VIDEO_STREAM_ID: Int = 1
    var videoStreamID_decoded: UShort? = null

    val TAG_AUDIO_STREAM_ID: Int = 2
    var audioStreamID_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_WEB_RTC_SESSION_ID)) {
        webRTCSessionID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_VIDEO_STREAM_ID)) {
        videoStreamID_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(tag)) {
                tlvReader.getUShort(tag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_AUDIO_STREAM_ID)) {
        audioStreamID_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(tag)) {
                tlvReader.getUShort(tag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (webRTCSessionID_decoded == null) {
      throw IllegalStateException("webRTCSessionID not found in TLV")
    }

    tlvReader.exitContainer()

    return ProvideOfferResponse(
      webRTCSessionID_decoded,
      videoStreamID_decoded,
      audioStreamID_decoded,
    )
  }

  suspend fun provideAnswer(
    webRTCSessionID: UShort,
    sdp: String,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEB_RTC_SESSION_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEB_RTC_SESSION_ID_REQ), webRTCSessionID)

    val TAG_SDP_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SDP_REQ), sdp)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun provideICECandidates(
    webRTCSessionID: UShort,
    ICECandidates: List<String>,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEB_RTC_SESSION_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEB_RTC_SESSION_ID_REQ), webRTCSessionID)

    val TAG_ICE_CANDIDATES_REQ: Int = 1
    tlvWriter.startArray(ContextSpecificTag(TAG_ICE_CANDIDATES_REQ))
    for (item in ICECandidates.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun endSession(
    webRTCSessionID: UShort,
    reason: UByte,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEB_RTC_SESSION_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEB_RTC_SESSION_ID_REQ), webRTCSessionID)

    val TAG_REASON_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_REASON_REQ), reason)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun readCurrentSessionsAttribute(): CurrentSessionsAttribute {
    val ATTRIBUTE_ID: UInt = 0u

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

    requireNotNull(attributeData) { "Currentsessions attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<WebRTCTransportProviderClusterWebRTCSessionStruct> =
      buildList<WebRTCTransportProviderClusterWebRTCSessionStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(WebRTCTransportProviderClusterWebRTCSessionStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return CurrentSessionsAttribute(decodedValue)
  }

  suspend fun subscribeCurrentSessionsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<CurrentSessionsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 0u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            CurrentSessionsAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Currentsessions attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<WebRTCTransportProviderClusterWebRTCSessionStruct> =
            buildList<WebRTCTransportProviderClusterWebRTCSessionStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  WebRTCTransportProviderClusterWebRTCSessionStruct.fromTlv(AnonymousTag, tlvReader)
                )
              }
              tlvReader.exitContainer()
            }

          emit(CurrentSessionsAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(CurrentSessionsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<GeneratedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65528u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            GeneratedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Generatedcommandlist attribute not found in Node State update"
          }

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

          emit(GeneratedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(GeneratedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AcceptedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65529u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AcceptedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Acceptedcommandlist attribute not found in Node State update"
          }

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

          emit(AcceptedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AcceptedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeEventListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<EventListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65530u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            EventListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Eventlist attribute not found in Node State update" }

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

          emit(EventListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EventListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AttributeListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65531u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AttributeListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Attributelist attribute not found in Node State update" }

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

          emit(AttributeListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AttributeListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeFeatureMapAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UIntSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65532u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UIntSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Featuremap attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

          emit(UIntSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UIntSubscriptionState.SubscriptionEstablished)
        }
      }
    }
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

  suspend fun subscribeClusterRevisionAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65533u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UShortSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Clusterrevision attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

          emit(UShortSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  companion object {
    private val logger = Logger.getLogger(WebRTCTransportProviderCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1363u
  }
}
