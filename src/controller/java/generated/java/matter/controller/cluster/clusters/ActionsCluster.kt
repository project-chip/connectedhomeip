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

class ActionsCluster(private val controller: MatterController, private val endpointId: UShort) {
  class ActionListAttribute(val value: List<ActionsClusterActionStruct>)

  class EndpointListsAttribute(val value: List<ActionsClusterEndpointListStruct>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun instantAction(
    actionID: UShort,
    invokeID: UInt?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun instantActionWithTransition(
    actionID: UShort,
    invokeID: UInt?,
    transitionTime: UShort,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }

    val TAG_TRANSITION_TIME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_TRANSITION_TIME_REQ), transitionTime)
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

  suspend fun startAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun startActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }

    val TAG_DURATION_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_DURATION_REQ), duration)
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

  suspend fun stopAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun pauseAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun pauseActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }

    val TAG_DURATION_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_DURATION_REQ), duration)
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

  suspend fun resumeAction(
    actionID: UShort,
    invokeID: UInt?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun enableAction(
    actionID: UShort,
    invokeID: UInt?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 8u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun enableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }

    val TAG_DURATION_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_DURATION_REQ), duration)
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

  suspend fun disableAction(
    actionID: UShort,
    invokeID: UInt?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 10u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }
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

  suspend fun disableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 11u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ACTION_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ACTION_I_D_REQ), actionID)

    val TAG_INVOKE_I_D_REQ: Int = 1
    invokeID?.let { tlvWriter.put(ContextSpecificTag(TAG_INVOKE_I_D_REQ), invokeID) }

    val TAG_DURATION_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_DURATION_REQ), duration)
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

  suspend fun readActionListAttribute(): ActionListAttribute {
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

    requireNotNull(attributeData) { "Actionlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ActionsClusterActionStruct> =
      buildList<ActionsClusterActionStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(ActionsClusterActionStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return ActionListAttribute(decodedValue)
  }

  suspend fun readEndpointListsAttribute(): EndpointListsAttribute {
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

    requireNotNull(attributeData) { "Endpointlists attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ActionsClusterEndpointListStruct> =
      buildList<ActionsClusterEndpointListStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(ActionsClusterEndpointListStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return EndpointListsAttribute(decodedValue)
  }

  suspend fun readSetupURLAttribute(): String? {
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

    requireNotNull(attributeData) { "Setupurl attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getString(AnonymousTag)
      } else {
        null
      }

    return decodedValue
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
    private val logger = Logger.getLogger(ActionsCluster::class.java.name)
    const val CLUSTER_ID: UInt = 37u
  }
}
