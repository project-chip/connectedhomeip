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
import matter.controller.BooleanSubscriptionState
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.UByteSubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisCluster(private val controller: MatterController, private val endpointId: UShort) {
  class EstablishAnalysisStreamResponse(val analysisStreamID: UShort)

  class SupportedAmbientContextsAttribute(val value: List<AvAnalysisClusterSemanticTagStruct>)

  sealed class SupportedAmbientContextsAttributeSubscriptionState {
    data class Success(val value: List<AvAnalysisClusterSemanticTagStruct>) :
      SupportedAmbientContextsAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      SupportedAmbientContextsAttributeSubscriptionState()

    object SubscriptionEstablished : SupportedAmbientContextsAttributeSubscriptionState()
  }

  class ActiveAmbientContextTriggersAttribute(
    val value: List<AvAnalysisClusterContextTriggerStruct>
  )

  sealed class ActiveAmbientContextTriggersAttributeSubscriptionState {
    data class Success(val value: List<AvAnalysisClusterContextTriggerStruct>) :
      ActiveAmbientContextTriggersAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      ActiveAmbientContextTriggersAttributeSubscriptionState()

    object SubscriptionEstablished : ActiveAmbientContextTriggersAttributeSubscriptionState()
  }

  class AnalysisStreamsAttribute(val value: List<AvAnalysisClusterAnalysisStreamStruct>?)

  sealed class AnalysisStreamsAttributeSubscriptionState {
    data class Success(val value: List<AvAnalysisClusterAnalysisStreamStruct>?) :
      AnalysisStreamsAttributeSubscriptionState()

    data class Error(val exception: Exception) : AnalysisStreamsAttributeSubscriptionState()

    object SubscriptionEstablished : AnalysisStreamsAttributeSubscriptionState()
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

  class AttributeListAttribute(val value: List<UInt>)

  sealed class AttributeListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AttributeListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AttributeListAttributeSubscriptionState()

    object SubscriptionEstablished : AttributeListAttributeSubscriptionState()
  }

  suspend fun enableContextTriggers(
    contextTriggers: List<AvAnalysisClusterContextTriggerStruct>?,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CONTEXT_TRIGGERS_REQ: Int = 0
    contextTriggers?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_CONTEXT_TRIGGERS_REQ))
      for (item in contextTriggers.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
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
  }

  suspend fun disableContextTriggers(
    contextTriggers: List<AvAnalysisClusterContextTriggerStruct>?,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CONTEXT_TRIGGERS_REQ: Int = 0
    contextTriggers?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_CONTEXT_TRIGGERS_REQ))
      for (item in contextTriggers.iterator()) {
        item.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endArray()
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
  }

  suspend fun establishAnalysisStream(
    nodeID: ULong,
    timedInvokeTimeout: Duration? = null,
  ): EstablishAnalysisStreamResponse {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)
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
    val TAG_ANALYSIS_STREAM_ID: Int = 0
    var analysisStreamID_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ANALYSIS_STREAM_ID)) {
        analysisStreamID_decoded = tlvReader.getUShort(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (analysisStreamID_decoded == null) {
      throw IllegalStateException("analysisStreamID not found in TLV")
    }

    tlvReader.exitContainer()

    return EstablishAnalysisStreamResponse(analysisStreamID_decoded)
  }

  suspend fun activateAnalysisStream(
    analysisStreamID: UShort,
    webRTCEndpointID: UShort?,
    pushAVEndpointID: UShort?,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ANALYSIS_STREAM_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ANALYSIS_STREAM_ID_REQ), analysisStreamID)

    val TAG_WEB_RTC_ENDPOINT_ID_REQ: Int = 1
    webRTCEndpointID?.let {
      tlvWriter.put(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID_REQ), webRTCEndpointID)
    }

    val TAG_PUSH_AV_ENDPOINT_ID_REQ: Int = 2
    pushAVEndpointID?.let {
      tlvWriter.put(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID_REQ), pushAVEndpointID)
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
  }

  suspend fun deactivateAnalysisStream(
    analysisStreamID: UShort,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ANALYSIS_STREAM_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ANALYSIS_STREAM_ID_REQ), analysisStreamID)
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

  suspend fun removeAnalysisStream(analysisStreamID: UShort, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ANALYSIS_STREAM_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ANALYSIS_STREAM_ID_REQ), analysisStreamID)
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

  suspend fun readSupportedAmbientContextsAttribute(): SupportedAmbientContextsAttribute {
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

    requireNotNull(attributeData) { "Supportedambientcontexts attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AvAnalysisClusterSemanticTagStruct> =
      buildList<AvAnalysisClusterSemanticTagStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(AvAnalysisClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return SupportedAmbientContextsAttribute(decodedValue)
  }

  suspend fun subscribeSupportedAmbientContextsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SupportedAmbientContextsAttributeSubscriptionState> {
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
            SupportedAmbientContextsAttributeSubscriptionState.Error(
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
            "Supportedambientcontexts attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AvAnalysisClusterSemanticTagStruct> =
            buildList<AvAnalysisClusterSemanticTagStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(AvAnalysisClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }

          emit(SupportedAmbientContextsAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SupportedAmbientContextsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readActiveAmbientContextTriggersAttribute(): ActiveAmbientContextTriggersAttribute {
    val ATTRIBUTE_ID: UInt = 1u

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

    requireNotNull(attributeData) { "Activeambientcontexttriggers attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AvAnalysisClusterContextTriggerStruct> =
      buildList<AvAnalysisClusterContextTriggerStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(AvAnalysisClusterContextTriggerStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return ActiveAmbientContextTriggersAttribute(decodedValue)
  }

  suspend fun subscribeActiveAmbientContextTriggersAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<ActiveAmbientContextTriggersAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 1u
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
            ActiveAmbientContextTriggersAttributeSubscriptionState.Error(
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
            "Activeambientcontexttriggers attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AvAnalysisClusterContextTriggerStruct> =
            buildList<AvAnalysisClusterContextTriggerStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(AvAnalysisClusterContextTriggerStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }

          emit(ActiveAmbientContextTriggersAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ActiveAmbientContextTriggersAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMaxAnalysisStreamCountAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 2u

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

    requireNotNull(attributeData) { "Maxanalysisstreamcount attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMaxAnalysisStreamCountAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 2u
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
            UByteSubscriptionState.Error(
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
            "Maxanalysisstreamcount attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readCurrentAnalysisStreamCountAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 3u

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

    requireNotNull(attributeData) { "Currentanalysisstreamcount attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeCurrentAnalysisStreamCountAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 3u
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
            UByteSubscriptionState.Error(
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
            "Currentanalysisstreamcount attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAnalysisStreamsAttribute(): AnalysisStreamsAttribute {
    val ATTRIBUTE_ID: UInt = 4u

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

    requireNotNull(attributeData) { "Analysisstreams attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AvAnalysisClusterAnalysisStreamStruct>? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        buildList<AvAnalysisClusterAnalysisStreamStruct> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(AvAnalysisClusterAnalysisStreamStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    return AnalysisStreamsAttribute(decodedValue)
  }

  suspend fun subscribeAnalysisStreamsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AnalysisStreamsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 4u
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
            AnalysisStreamsAttributeSubscriptionState.Error(
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
            "Analysisstreams attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AvAnalysisClusterAnalysisStreamStruct>? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              buildList<AvAnalysisClusterAnalysisStreamStruct> {
                tlvReader.enterArray(AnonymousTag)
                while (!tlvReader.isEndOfContainer()) {
                  add(AvAnalysisClusterAnalysisStreamStruct.fromTlv(AnonymousTag, tlvReader))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }

          decodedValue?.let { emit(AnalysisStreamsAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AnalysisStreamsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readTrackingEnabledAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 5u

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

    requireNotNull(attributeData) { "Trackingenabled attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeTrackingEnabledAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded(),
            )
          ),
        timedRequest = timedWriteTimeout,
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun subscribeTrackingEnabledAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 5u
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
            BooleanSubscriptionState.Error(
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
            "Trackingenabled attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

          emit(BooleanSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
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
    private val logger = Logger.getLogger(AvAnalysisCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1367u
  }
}
