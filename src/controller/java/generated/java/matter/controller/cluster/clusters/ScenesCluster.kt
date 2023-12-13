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
    timedInvokeTimeout: Duration? = null
  ): AddSceneResponse {
    val commandId: UInt = 0u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
    }

    tlvReader.exitContainer()

    return AddSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun viewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeout: Duration? = null
  ): ViewSceneResponse {
    val commandId: UInt = 1u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    val TAG_TRANSITION_TIME: Int = 3
    var transitionTime_decoded: UShort? = null

    val TAG_SCENE_NAME: Int = 4
    var sceneName_decoded: String? = null

    val TAG_EXTENSION_FIELD_SETS: Int = 5
    var extensionFieldSets_decoded: List<ScenesClusterExtensionFieldSet>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_TRANSITION_TIME)) {
        transitionTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUShort(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_SCENE_NAME)) {
        sceneName_decoded =
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

      if (tag == ContextSpecificTag(TAG_EXTENSION_FIELD_SETS)) {
        extensionFieldSets_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<ScenesClusterExtensionFieldSet> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(ScenesClusterExtensionFieldSet.fromTlv(AnonymousTag, tlvReader))
                }
                tlvReader.exitContainer()
              }
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

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
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
    timedInvokeTimeout: Duration? = null
  ): RemoveSceneResponse {
    val commandId: UInt = 2u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
    }

    tlvReader.exitContainer()

    return RemoveSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun removeAllScenes(
    groupID: UShort,
    timedInvokeTimeout: Duration? = null
  ): RemoveAllScenesResponse {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)
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

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    tlvReader.exitContainer()

    return RemoveAllScenesResponse(status_decoded, groupID_decoded)
  }

  suspend fun storeScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeout: Duration? = null
  ): StoreSceneResponse {
    val commandId: UInt = 4u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
    }

    tlvReader.exitContainer()

    return StoreSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun recallScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 5u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getSceneMembership(
    groupID: UShort,
    timedInvokeTimeout: Duration? = null
  ): GetSceneMembershipResponse {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_I_D_REQ), groupID)
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

    val TAG_CAPACITY: Int = 1
    var capacity_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 2
    var groupID_decoded: UShort? = null

    val TAG_SCENE_LIST: Int = 3
    var sceneList_decoded: List<UByte>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_CAPACITY)) {
        capacity_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_LIST)) {
        sceneList_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<UByte> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
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

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
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
    timedInvokeTimeout: Duration? = null
  ): EnhancedAddSceneResponse {
    val commandId: UInt = 64u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
    }

    tlvReader.exitContainer()

    return EnhancedAddSceneResponse(status_decoded, groupID_decoded, sceneID_decoded)
  }

  suspend fun enhancedViewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeout: Duration? = null
  ): EnhancedViewSceneResponse {
    val commandId: UInt = 65u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_I_D: Int = 1
    var groupID_decoded: UShort? = null

    val TAG_SCENE_I_D: Int = 2
    var sceneID_decoded: UByte? = null

    val TAG_TRANSITION_TIME: Int = 3
    var transitionTime_decoded: UShort? = null

    val TAG_SCENE_NAME: Int = 4
    var sceneName_decoded: String? = null

    val TAG_EXTENSION_FIELD_SETS: Int = 5
    var extensionFieldSets_decoded: List<ScenesClusterExtensionFieldSet>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_I_D)) {
        groupID_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_I_D)) {
        sceneID_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_TRANSITION_TIME)) {
        transitionTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUShort(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_SCENE_NAME)) {
        sceneName_decoded =
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

      if (tag == ContextSpecificTag(TAG_EXTENSION_FIELD_SETS)) {
        extensionFieldSets_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<ScenesClusterExtensionFieldSet> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(ScenesClusterExtensionFieldSet.fromTlv(AnonymousTag, tlvReader))
                }
                tlvReader.exitContainer()
              }
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

    if (groupID_decoded == null) {
      throw IllegalStateException("groupID not found in TLV")
    }

    if (sceneID_decoded == null) {
      throw IllegalStateException("sceneID not found in TLV")
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
    timedInvokeTimeout: Duration? = null
  ): CopySceneResponse {
    val commandId: UInt = 66u

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
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_GROUP_IDENTIFIER_FROM: Int = 1
    var groupIdentifierFrom_decoded: UShort? = null

    val TAG_SCENE_IDENTIFIER_FROM: Int = 2
    var sceneIdentifierFrom_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_GROUP_IDENTIFIER_FROM)) {
        groupIdentifierFrom_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_SCENE_IDENTIFIER_FROM)) {
        sceneIdentifierFrom_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    if (groupIdentifierFrom_decoded == null) {
      throw IllegalStateException("groupIdentifierFrom not found in TLV")
    }

    if (sceneIdentifierFrom_decoded == null) {
      throw IllegalStateException("sceneIdentifierFrom not found in TLV")
    }

    tlvReader.exitContainer()

    return CopySceneResponse(
      status_decoded,
      groupIdentifierFrom_decoded,
      sceneIdentifierFrom_decoded
    )
  }

  suspend fun readSceneCountAttribute(): UByte? {
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
  }

  suspend fun readCurrentSceneAttribute(): UByte? {
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
  }

  suspend fun readCurrentGroupAttribute(): UShort? {
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
  }

  suspend fun readSceneValidAttribute(): Boolean? {
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
  }

  suspend fun readNameSupportAttribute(): UByte {
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

    requireNotNull(attributeData) { "Namesupport attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun readLastConfiguredByAttribute(): LastConfiguredByAttribute {
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
  }

  suspend fun readSceneTableSizeAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 6u

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

    requireNotNull(attributeData) { "Scenetablesize attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun readFabricSceneInfoAttribute(): FabricSceneInfoAttribute {
    val ATTRIBUTE_ID: UInt = 7u

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
    private val logger = Logger.getLogger(ScenesCluster::class.java.name)
    const val CLUSTER_ID: UInt = 5u
  }
}
