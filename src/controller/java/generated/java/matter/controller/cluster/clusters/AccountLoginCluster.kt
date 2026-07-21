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
import matter.controller.UIntSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccountLoginCluster(
  private val controller: MatterController,
  private val endpointId: UShort,
) {
  class GetSetupPINResponse(val setupPIN: String)

  class GetDeviceAuthURIResponse(
    val userCode: String,
    val verificationURI: String,
    val verificationURIComplete: String?,
    val expiresIn: UShort,
    val interval: UByte,
  )

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

  suspend fun getSetupPIN(
    tempAccountIdentifier: String,
    timedInvokeTimeout: Duration,
  ): GetSetupPINResponse {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_TEMP_ACCOUNT_IDENTIFIER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_TEMP_ACCOUNT_IDENTIFIER_REQ), tempAccountIdentifier)
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
    val TAG_SETUP_PIN: Int = 0
    var setupPIN_decoded: String? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_SETUP_PIN)) {
        setupPIN_decoded = tlvReader.getString(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (setupPIN_decoded == null) {
      throw IllegalStateException("setupPIN not found in TLV")
    }

    tlvReader.exitContainer()

    return GetSetupPINResponse(setupPIN_decoded)
  }

  suspend fun login(
    tempAccountIdentifier: String,
    setupPIN: String,
    node: ULong?,
    timedInvokeTimeout: Duration,
  ) {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_TEMP_ACCOUNT_IDENTIFIER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_TEMP_ACCOUNT_IDENTIFIER_REQ), tempAccountIdentifier)

    val TAG_SETUP_PIN_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SETUP_PIN_REQ), setupPIN)

    val TAG_NODE_REQ: Int = 2
    node?.let { tlvWriter.put(ContextSpecificTag(TAG_NODE_REQ), node) }
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

  suspend fun logout(node: ULong?, timedInvokeTimeout: Duration) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_REQ: Int = 0
    node?.let { tlvWriter.put(ContextSpecificTag(TAG_NODE_REQ), node) }
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

  suspend fun getDeviceAuthURI(timedInvokeTimeout: Duration): GetDeviceAuthURIResponse {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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
    val TAG_USER_CODE: Int = 0
    var userCode_decoded: String? = null

    val TAG_VERIFICATION_URI: Int = 1
    var verificationURI_decoded: String? = null

    val TAG_VERIFICATION_URI_COMPLETE: Int = 2
    var verificationURIComplete_decoded: String? = null

    val TAG_EXPIRES_IN: Int = 3
    var expiresIn_decoded: UShort? = null

    val TAG_INTERVAL: Int = 4
    var interval_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_USER_CODE)) {
        userCode_decoded = tlvReader.getString(tag)
      } else if (tag == ContextSpecificTag(TAG_VERIFICATION_URI)) {
        verificationURI_decoded = tlvReader.getString(tag)
      } else if (tag == ContextSpecificTag(TAG_VERIFICATION_URI_COMPLETE)) {
        verificationURIComplete_decoded =
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
      } else if (tag == ContextSpecificTag(TAG_EXPIRES_IN)) {
        expiresIn_decoded = tlvReader.getUShort(tag)
      } else if (tag == ContextSpecificTag(TAG_INTERVAL)) {
        interval_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (userCode_decoded == null) {
      throw IllegalStateException("userCode not found in TLV")
    }

    if (verificationURI_decoded == null) {
      throw IllegalStateException("verificationURI not found in TLV")
    }

    if (expiresIn_decoded == null) {
      throw IllegalStateException("expiresIn not found in TLV")
    }

    if (interval_decoded == null) {
      throw IllegalStateException("interval not found in TLV")
    }

    tlvReader.exitContainer()

    return GetDeviceAuthURIResponse(
      userCode_decoded,
      verificationURI_decoded,
      verificationURIComplete_decoded,
      expiresIn_decoded,
      interval_decoded,
    )
  }

  suspend fun readOAuthLoggedInAttribute(): Boolean? {
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

    requireNotNull(attributeData) { "Oauthloggedin attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeOAuthLoggedInAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<BooleanSubscriptionState> {
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

          requireNotNull(attributeData) { "Oauthloggedin attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
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
    private val logger = Logger.getLogger(AccountLoginCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1294u
  }
}
