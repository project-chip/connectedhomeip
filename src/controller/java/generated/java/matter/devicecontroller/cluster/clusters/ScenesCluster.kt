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

class ScenesCluster(private val controller: MatterController, private val endpointId: UShort) {
  class AddSceneResponse(val status: UByte, val groupID: UShort, val sceneID: UByte)

  class ViewSceneResponse(
    val status: UByte,
    val groupID: UShort,
    val sceneID: UByte,
    val transitionTime: UShort?,
    val sceneName: String?,
    val extensionFieldSets: List<ScenesClusterExtensionFieldSet>?
  )

  class RemoveSceneResponse(val status: UByte, val groupID: UShort, val sceneID: UByte)

  class RemoveAllScenesResponse(val status: UByte, val groupID: UShort)

  class StoreSceneResponse(val status: UByte, val groupID: UShort, val sceneID: UByte)

  class GetSceneMembershipResponse(
    val status: UByte,
    val capacity: UByte?,
    val groupID: UShort,
    val sceneList: List<UByte>?
  )

  class EnhancedAddSceneResponse(val status: UByte, val groupID: UShort, val sceneID: UByte)

  class EnhancedViewSceneResponse(
    val status: UByte,
    val groupID: UShort,
    val sceneID: UByte,
    val transitionTime: UShort?,
    val sceneName: String?,
    val extensionFieldSets: List<ScenesClusterExtensionFieldSet>?
  )

  class CopySceneResponse(
    val status: UByte,
    val groupIdentifierFrom: UShort,
    val sceneIdentifierFrom: UByte
  )

  class LastConfiguredByAttribute(val value: ULong?)

  class FabricSceneInfoAttribute(val value: List<ScenesClusterSceneInfoStruct>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun addScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort,
    sceneName: String,
    extensionFieldSets: List<ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int? = null
  ): AddSceneResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)

    val TAG_TRANSITION_TIME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_TRANSITION_TIME_REQ), transitionTime)

    val TAG_SCENE_NAME_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_NAME_REQ), sceneName)

    val TAG_EXTENSION_FIELD_SETS_REQ: Int = 4
    tlvWriter.startArray(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS_REQ))
    for (item in extensionFieldSets.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))
    tlvReader.exitContainer()

    return AddSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun viewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): ViewSceneResponse {
    val commandId: UInt = 1u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))

    val TAG_TRANSITION_TIME: Int = 3
    val transitionTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRANSITION_TIME))) {
        tlvReader.getUShort(ContextSpecificTag(TAG_TRANSITION_TIME))
      } else {
        null
      }

    val TAG_SCENE_NAME: Int = 4
    val sceneName_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_SCENE_NAME))) {
        tlvReader.getString(ContextSpecificTag(TAG_SCENE_NAME))
      } else {
        null
      }

    val TAG_EXTENSION_FIELD_SETS: Int = 5
    val extensionFieldSets_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS))) {
        buildList<ScenesClusterExtensionFieldSet> {
          tlvReader.enterArray(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS))
          while (!tlvReader.isEndOfContainer()) {
            add(ScenesClusterExtensionFieldSet.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }
    tlvReader.exitContainer()

    return ViewSceneResponse(
      status_decoded,
      groupID_decoded,
      sceneID_decoded,
      transitionTime_decoded,
      sceneName_decoded,
      extensionFieldSets_decoded
    )
  }

  suspend fun removeScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): RemoveSceneResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))
    tlvReader.exitContainer()

    return RemoveSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun removeAllScenes(
    groupID: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): RemoveAllScenesResponse {
    val commandId: UInt = 3u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))
    tlvReader.exitContainer()

    return RemoveAllScenesResponse(status_decoded, groupID_decoded)
  }

  suspend fun storeScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): StoreSceneResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))
    tlvReader.exitContainer()

    return StoreSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun recallScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort?,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 5u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)

    val TAG_TRANSITION_TIME_REQ: Int = 2
    transitionTime?.let {
      tlvWriter.put(ContextSpecificTag(TAG_TRANSITION_TIME_REQ), transitionTime)
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
  }

  suspend fun getSceneMembership(
    groupID: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetSceneMembershipResponse {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)
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

    val TAG_CAPACITY: Int = 1
    val capacity_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CAPACITY))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_CAPACITY))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CAPACITY))
          null
        }
      } else {
        null
      }

    val TAG_GROUP_I_D: Int = 2
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_LIST: Int = 3
    val sceneList_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_SCENE_LIST))) {
        buildList<UByte> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SCENE_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }
    tlvReader.exitContainer()

    return GetSceneMembershipResponse(
      status_decoded,
      capacity_decoded,
      groupID_decoded,
      sceneList_decoded
    )
  }

  suspend fun enhancedAddScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort,
    sceneName: String,
    extensionFieldSets: List<ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int? = null
  ): EnhancedAddSceneResponse {
    val commandId: UInt = 64u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)

    val TAG_TRANSITION_TIME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_TRANSITION_TIME_REQ), transitionTime)

    val TAG_SCENE_NAME_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_NAME_REQ), sceneName)

    val TAG_EXTENSION_FIELD_SETS_REQ: Int = 4
    tlvWriter.startArray(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS_REQ))
    for (item in extensionFieldSets.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))
    tlvReader.exitContainer()

    return EnhancedAddSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun enhancedViewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): EnhancedViewSceneResponse {
    val commandId: UInt = 65u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)

    val TAG_SCENE_I_D_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_I_D_REQ), sceneID)
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

    val TAG_GROUP_I_D: Int = 1
    val groupID_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_I_D))

    val TAG_SCENE_I_D: Int = 2
    val sceneID_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_I_D))

    val TAG_TRANSITION_TIME: Int = 3
    val transitionTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRANSITION_TIME))) {
        tlvReader.getUShort(ContextSpecificTag(TAG_TRANSITION_TIME))
      } else {
        null
      }

    val TAG_SCENE_NAME: Int = 4
    val sceneName_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_SCENE_NAME))) {
        tlvReader.getString(ContextSpecificTag(TAG_SCENE_NAME))
      } else {
        null
      }

    val TAG_EXTENSION_FIELD_SETS: Int = 5
    val extensionFieldSets_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS))) {
        buildList<ScenesClusterExtensionFieldSet> {
          tlvReader.enterArray(ContextSpecificTag(TAG_EXTENSION_FIELD_SETS))
          while (!tlvReader.isEndOfContainer()) {
            add(ScenesClusterExtensionFieldSet.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }
    tlvReader.exitContainer()

    return EnhancedViewSceneResponse(
      status_decoded,
      groupID_decoded,
      sceneID_decoded,
      transitionTime_decoded,
      sceneName_decoded,
      extensionFieldSets_decoded
    )
  }

  suspend fun copyScene(
    mode: UByte,
    groupIdentifierFrom: UShort,
    sceneIdentifierFrom: UByte,
    groupIdentifierTo: UShort,
    sceneIdentifierTo: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): CopySceneResponse {
    val commandId: UInt = 66u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_MODE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_MODE_REQ), mode)

    val TAG_GROUP_IDENTIFIER_FROM_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_IDENTIFIER_FROM_REQ), groupIdentifierFrom)

    val TAG_SCENE_IDENTIFIER_FROM_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_IDENTIFIER_FROM_REQ), sceneIdentifierFrom)

    val TAG_GROUP_IDENTIFIER_TO_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_IDENTIFIER_TO_REQ), groupIdentifierTo)

    val TAG_SCENE_IDENTIFIER_TO_REQ: Int = 4
    tlvWriter.put(ContextSpecificTag(TAG_SCENE_IDENTIFIER_TO_REQ), sceneIdentifierTo)
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

    val TAG_GROUP_IDENTIFIER_FROM: Int = 1
    val groupIdentifierFrom_decoded =
      tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_IDENTIFIER_FROM))

    val TAG_SCENE_IDENTIFIER_FROM: Int = 2
    val sceneIdentifierFrom_decoded =
      tlvReader.getUByte(ContextSpecificTag(TAG_SCENE_IDENTIFIER_FROM))
    tlvReader.exitContainer()

    return CopySceneResponse(
      status_decoded,
      groupIdentifierFrom_decoded,
      sceneIdentifierFrom_decoded
    )
  }

  suspend fun readSceneCountAttribute(): UByte? {
    val ATTRIBUTE_ID_SCENECOUNT: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SCENECOUNT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SCENECOUNT
        }

      requireNotNull(attributeData) { "Scenecount attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCurrentSceneAttribute(): UByte? {
    val ATTRIBUTE_ID_CURRENTSCENE: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTSCENE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTSCENE
        }

      requireNotNull(attributeData) { "Currentscene attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCurrentGroupAttribute(): UShort? {
    val ATTRIBUTE_ID_CURRENTGROUP: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTGROUP
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTGROUP
        }

      requireNotNull(attributeData) { "Currentgroup attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSceneValidAttribute(): Boolean? {
    val ATTRIBUTE_ID_SCENEVALID: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SCENEVALID
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SCENEVALID
        }

      requireNotNull(attributeData) { "Scenevalid attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getBoolean(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNameSupportAttribute(): UByte {
    val ATTRIBUTE_ID_NAMESUPPORT: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NAMESUPPORT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NAMESUPPORT
        }

      requireNotNull(attributeData) { "Namesupport attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readLastConfiguredByAttribute(): LastConfiguredByAttribute {
    val ATTRIBUTE_ID_LASTCONFIGUREDBY: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LASTCONFIGUREDBY
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LASTCONFIGUREDBY
        }

      requireNotNull(attributeData) { "Lastconfiguredby attribute not found in response" }

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

      return LastConfiguredByAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSceneTableSizeAttribute(): UShort {
    val ATTRIBUTE_ID_SCENETABLESIZE: UInt = 6u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SCENETABLESIZE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SCENETABLESIZE
        }

      requireNotNull(attributeData) { "Scenetablesize attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readFabricSceneInfoAttribute(): FabricSceneInfoAttribute {
    val ATTRIBUTE_ID_FABRICSCENEINFO: UInt = 7u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FABRICSCENEINFO
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FABRICSCENEINFO
        }

      requireNotNull(attributeData) { "Fabricsceneinfo attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: List<ScenesClusterSceneInfoStruct> =
        buildList<ScenesClusterSceneInfoStruct> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(ScenesClusterSceneInfoStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      return FabricSceneInfoAttribute(decodedValue)
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
    private val logger = Logger.getLogger(ScenesCluster::class.java.name)
    const val CLUSTER_ID: UInt = 5u
  }
}
