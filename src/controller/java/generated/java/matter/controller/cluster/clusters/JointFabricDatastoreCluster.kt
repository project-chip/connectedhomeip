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
import matter.controller.ByteArraySubscriptionState
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.StringSubscriptionState
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.ULongSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class JointFabricDatastoreCluster(
  private val controller: MatterController,
  private val endpointId: UShort,
) {
  class GroupKeySetListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreGroupKeySetStruct>
  )

  sealed class GroupKeySetListAttributeSubscriptionState {
    data class Success(val value: List<JointFabricDatastoreClusterDatastoreGroupKeySetStruct>) :
      GroupKeySetListAttributeSubscriptionState()

    data class Error(val exception: Exception) : GroupKeySetListAttributeSubscriptionState()

    object SubscriptionEstablished : GroupKeySetListAttributeSubscriptionState()
  }

  class GroupListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct>
  )

  sealed class GroupListAttributeSubscriptionState {
    data class Success(
      val value: List<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct>
    ) : GroupListAttributeSubscriptionState()

    data class Error(val exception: Exception) : GroupListAttributeSubscriptionState()

    object SubscriptionEstablished : GroupListAttributeSubscriptionState()
  }

  class NodeListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct>
  )

  sealed class NodeListAttributeSubscriptionState {
    data class Success(
      val value: List<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct>
    ) : NodeListAttributeSubscriptionState()

    data class Error(val exception: Exception) : NodeListAttributeSubscriptionState()

    object SubscriptionEstablished : NodeListAttributeSubscriptionState()
  }

  class AdminListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct>
  )

  sealed class AdminListAttributeSubscriptionState {
    data class Success(
      val value: List<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct>
    ) : AdminListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AdminListAttributeSubscriptionState()

    object SubscriptionEstablished : AdminListAttributeSubscriptionState()
  }

  class StatusAttribute(val value: JointFabricDatastoreClusterDatastoreStatusEntryStruct)

  sealed class StatusAttributeSubscriptionState {
    data class Success(val value: JointFabricDatastoreClusterDatastoreStatusEntryStruct) :
      StatusAttributeSubscriptionState()

    data class Error(val exception: Exception) : StatusAttributeSubscriptionState()

    object SubscriptionEstablished : StatusAttributeSubscriptionState()
  }

  class EndpointGroupIDListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct>
  )

  sealed class EndpointGroupIDListAttributeSubscriptionState {
    data class Success(
      val value: List<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct>
    ) : EndpointGroupIDListAttributeSubscriptionState()

    data class Error(val exception: Exception) : EndpointGroupIDListAttributeSubscriptionState()

    object SubscriptionEstablished : EndpointGroupIDListAttributeSubscriptionState()
  }

  class EndpointBindingListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct>
  )

  sealed class EndpointBindingListAttributeSubscriptionState {
    data class Success(
      val value: List<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct>
    ) : EndpointBindingListAttributeSubscriptionState()

    data class Error(val exception: Exception) : EndpointBindingListAttributeSubscriptionState()

    object SubscriptionEstablished : EndpointBindingListAttributeSubscriptionState()
  }

  class NodeKeySetListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct>
  )

  sealed class NodeKeySetListAttributeSubscriptionState {
    data class Success(val value: List<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct>) :
      NodeKeySetListAttributeSubscriptionState()

    data class Error(val exception: Exception) : NodeKeySetListAttributeSubscriptionState()

    object SubscriptionEstablished : NodeKeySetListAttributeSubscriptionState()
  }

  class NodeACLListAttribute(val value: List<JointFabricDatastoreClusterDatastoreACLEntryStruct>)

  sealed class NodeACLListAttributeSubscriptionState {
    data class Success(val value: List<JointFabricDatastoreClusterDatastoreACLEntryStruct>) :
      NodeACLListAttributeSubscriptionState()

    data class Error(val exception: Exception) : NodeACLListAttributeSubscriptionState()

    object SubscriptionEstablished : NodeACLListAttributeSubscriptionState()
  }

  class NodeEndpointListAttribute(
    val value: List<JointFabricDatastoreClusterDatastoreEndpointEntryStruct>
  )

  sealed class NodeEndpointListAttributeSubscriptionState {
    data class Success(val value: List<JointFabricDatastoreClusterDatastoreEndpointEntryStruct>) :
      NodeEndpointListAttributeSubscriptionState()

    data class Error(val exception: Exception) : NodeEndpointListAttributeSubscriptionState()

    object SubscriptionEstablished : NodeEndpointListAttributeSubscriptionState()
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

  suspend fun addKeySet(
    groupKeySet: JointFabricDatastoreClusterDatastoreGroupKeySetStruct,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_KEY_SET_REQ: Int = 0
    groupKeySet.toTlv(ContextSpecificTag(TAG_GROUP_KEY_SET_REQ), tlvWriter)
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

  suspend fun updateKeySet(
    groupKeySet: JointFabricDatastoreClusterDatastoreGroupKeySetStruct,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_KEY_SET_REQ: Int = 0
    groupKeySet.toTlv(ContextSpecificTag(TAG_GROUP_KEY_SET_REQ), tlvWriter)
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

  suspend fun removeKeySet(groupKeySetID: UShort, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_KEY_SET_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_KEY_SET_ID_REQ), groupKeySetID)
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

  suspend fun addGroup(
    groupID: UShort,
    friendlyName: String,
    groupKeySetID: UShort?,
    groupCAT: UShort?,
    groupCATVersion: UShort?,
    groupPermission: UByte,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_ID_REQ), groupID)

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName)

    val TAG_GROUP_KEY_SET_ID_REQ: Int = 2
    groupKeySetID?.let {
      tlvWriter.put(ContextSpecificTag(TAG_GROUP_KEY_SET_ID_REQ), groupKeySetID)
    }

    val TAG_GROUP_CAT_REQ: Int = 3
    groupCAT?.let { tlvWriter.put(ContextSpecificTag(TAG_GROUP_CAT_REQ), groupCAT) }

    val TAG_GROUP_CAT_VERSION_REQ: Int = 4
    groupCATVersion?.let {
      tlvWriter.put(ContextSpecificTag(TAG_GROUP_CAT_VERSION_REQ), groupCATVersion)
    }

    val TAG_GROUP_PERMISSION_REQ: Int = 5
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_PERMISSION_REQ), groupPermission)
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

  suspend fun updateGroup(
    groupID: UShort,
    friendlyName: String?,
    groupKeySetID: UShort?,
    groupCAT: UShort?,
    groupCATVersion: UShort?,
    groupPermission: UByte,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_ID_REQ), groupID)

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    friendlyName?.let { tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName) }

    val TAG_GROUP_KEY_SET_ID_REQ: Int = 2
    groupKeySetID?.let {
      tlvWriter.put(ContextSpecificTag(TAG_GROUP_KEY_SET_ID_REQ), groupKeySetID)
    }

    val TAG_GROUP_CAT_REQ: Int = 3
    groupCAT?.let { tlvWriter.put(ContextSpecificTag(TAG_GROUP_CAT_REQ), groupCAT) }

    val TAG_GROUP_CAT_VERSION_REQ: Int = 4
    groupCATVersion?.let {
      tlvWriter.put(ContextSpecificTag(TAG_GROUP_CAT_VERSION_REQ), groupCATVersion)
    }

    val TAG_GROUP_PERMISSION_REQ: Int = 5
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_PERMISSION_REQ), groupPermission)
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

  suspend fun removeGroup(groupID: UShort, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_GROUP_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_ID_REQ), groupID)
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

  suspend fun addAdmin(
    nodeID: ULong,
    friendlyName: String,
    vendorID: UShort,
    icac: ByteArray,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName)

    val TAG_VENDOR_ID_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_VENDOR_ID_REQ), vendorID)

    val TAG_ICAC_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_ICAC_REQ), icac)
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

  suspend fun updateAdmin(
    nodeID: ULong?,
    friendlyName: String?,
    icac: ByteArray?,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    nodeID?.let { tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID) }

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    friendlyName?.let { tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName) }

    val TAG_ICAC_REQ: Int = 2
    icac?.let { tlvWriter.put(ContextSpecificTag(TAG_ICAC_REQ), icac) }
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

  suspend fun removeAdmin(nodeID: ULong, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 8u

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
  }

  suspend fun addPendingNode(
    nodeID: ULong,
    friendlyName: String,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName)
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

  suspend fun refreshNode(nodeID: ULong, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 10u

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
  }

  suspend fun updateNode(
    nodeID: ULong,
    friendlyName: String,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 11u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_FRIENDLY_NAME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName)
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

  suspend fun removeNode(nodeID: ULong, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 12u

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
  }

  suspend fun updateEndpointForNode(
    endpointID: UShort,
    nodeID: ULong,
    friendlyName: String,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 13u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ENDPOINT_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_ID_REQ), endpointID)

    val TAG_NODE_ID_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_FRIENDLY_NAME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_FRIENDLY_NAME_REQ), friendlyName)
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

  suspend fun addGroupIDToEndpointForNode(
    nodeID: ULong,
    endpointID: UShort,
    groupID: UShort,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 14u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_ENDPOINT_ID_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_ID_REQ), endpointID)

    val TAG_GROUP_ID_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_ID_REQ), groupID)
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

  suspend fun removeGroupIDFromEndpointForNode(
    nodeID: ULong,
    endpointID: UShort,
    groupID: UShort,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 15u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_ENDPOINT_ID_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_ID_REQ), endpointID)

    val TAG_GROUP_ID_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_ID_REQ), groupID)
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

  suspend fun addBindingToEndpointForNode(
    nodeID: ULong,
    endpointID: UShort,
    binding: JointFabricDatastoreClusterDatastoreBindingTargetStruct,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 16u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_ENDPOINT_ID_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_ID_REQ), endpointID)

    val TAG_BINDING_REQ: Int = 2
    binding.toTlv(ContextSpecificTag(TAG_BINDING_REQ), tlvWriter)
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

  suspend fun removeBindingFromEndpointForNode(
    listID: UShort,
    endpointID: UShort,
    nodeID: ULong,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 17u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_LIST_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_LIST_ID_REQ), listID)

    val TAG_ENDPOINT_ID_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_ID_REQ), endpointID)

    val TAG_NODE_ID_REQ: Int = 2
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
  }

  suspend fun addACLToNode(
    nodeID: ULong,
    ACLEntry: JointFabricDatastoreClusterDatastoreAccessControlEntryStruct,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 18u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NODE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NODE_ID_REQ), nodeID)

    val TAG_ACL_ENTRY_REQ: Int = 1
    ACLEntry.toTlv(ContextSpecificTag(TAG_ACL_ENTRY_REQ), tlvWriter)
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

  suspend fun removeACLFromNode(
    listID: UShort,
    nodeID: ULong,
    timedInvokeTimeout: Duration? = null,
  ) {
    val commandId: UInt = 19u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_LIST_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_LIST_ID_REQ), listID)

    val TAG_NODE_ID_REQ: Int = 1
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
  }

  suspend fun readAnchorRootCAAttribute(): ByteArray {
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

    requireNotNull(attributeData) { "Anchorrootca attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeAnchorRootCAAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<ByteArraySubscriptionState> {
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
            ByteArraySubscriptionState.Error(
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

          requireNotNull(attributeData) { "Anchorrootca attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

          emit(ByteArraySubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ByteArraySubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAnchorNodeIDAttribute(): ULong {
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

    requireNotNull(attributeData) { "Anchornodeid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeAnchorNodeIDAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<ULongSubscriptionState> {
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
            ULongSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Anchornodeid attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

          emit(ULongSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ULongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAnchorVendorIDAttribute(): UShort {
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

    requireNotNull(attributeData) { "Anchorvendorid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeAnchorVendorIDAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
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
            "Anchorvendorid attribute not found in Node State update"
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

  suspend fun readFriendlyNameAttribute(): String {
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

    requireNotNull(attributeData) { "Friendlyname attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String = tlvReader.getString(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeFriendlyNameAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<StringSubscriptionState> {
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
            StringSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Friendlyname attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: String = tlvReader.getString(AnonymousTag)

          emit(StringSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(StringSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readGroupKeySetListAttribute(): GroupKeySetListAttribute {
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

    requireNotNull(attributeData) { "Groupkeysetlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreGroupKeySetStruct> =
      buildList<JointFabricDatastoreClusterDatastoreGroupKeySetStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreGroupKeySetStruct.fromTlv(AnonymousTag, tlvReader)
          )
        }
        tlvReader.exitContainer()
      }

    return GroupKeySetListAttribute(decodedValue)
  }

  suspend fun subscribeGroupKeySetListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<GroupKeySetListAttributeSubscriptionState> {
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
            GroupKeySetListAttributeSubscriptionState.Error(
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
            "Groupkeysetlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreGroupKeySetStruct> =
            buildList<JointFabricDatastoreClusterDatastoreGroupKeySetStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreGroupKeySetStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(GroupKeySetListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(GroupKeySetListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readGroupListAttribute(): GroupListAttribute {
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

    requireNotNull(attributeData) { "Grouplist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return GroupListAttribute(decodedValue)
  }

  suspend fun subscribeGroupListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<GroupListAttributeSubscriptionState> {
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
            GroupListAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Grouplist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(GroupListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(GroupListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNodeListAttribute(): NodeListAttribute {
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

    requireNotNull(attributeData) { "Nodelist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return NodeListAttribute(decodedValue)
  }

  suspend fun subscribeNodeListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NodeListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 6u
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
            NodeListAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Nodelist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(NodeListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NodeListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAdminListAttribute(): AdminListAttribute {
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

    requireNotNull(attributeData) { "Adminlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue:
      List<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return AdminListAttribute(decodedValue)
  }

  suspend fun subscribeAdminListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AdminListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 7u
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
            AdminListAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Adminlist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue:
            List<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreAdministratorInformationEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(AdminListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AdminListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readStatusAttribute(): StatusAttribute {
    val ATTRIBUTE_ID: UInt = 8u

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

    requireNotNull(attributeData) { "Status attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: JointFabricDatastoreClusterDatastoreStatusEntryStruct =
      JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(AnonymousTag, tlvReader)

    return StatusAttribute(decodedValue)
  }

  suspend fun subscribeStatusAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<StatusAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 8u
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
            StatusAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Status attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: JointFabricDatastoreClusterDatastoreStatusEntryStruct =
            JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(AnonymousTag, tlvReader)

          emit(StatusAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(StatusAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEndpointGroupIDListAttribute(): EndpointGroupIDListAttribute {
    val ATTRIBUTE_ID: UInt = 9u

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

    requireNotNull(attributeData) { "Endpointgroupidlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return EndpointGroupIDListAttribute(decodedValue)
  }

  suspend fun subscribeEndpointGroupIDListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<EndpointGroupIDListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 9u
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
            EndpointGroupIDListAttributeSubscriptionState.Error(
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
            "Endpointgroupidlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(EndpointGroupIDListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EndpointGroupIDListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEndpointBindingListAttribute(): EndpointBindingListAttribute {
    val ATTRIBUTE_ID: UInt = 10u

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

    requireNotNull(attributeData) { "Endpointbindinglist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return EndpointBindingListAttribute(decodedValue)
  }

  suspend fun subscribeEndpointBindingListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<EndpointBindingListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 10u
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
            EndpointBindingListAttributeSubscriptionState.Error(
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
            "Endpointbindinglist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(EndpointBindingListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EndpointBindingListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNodeKeySetListAttribute(): NodeKeySetListAttribute {
    val ATTRIBUTE_ID: UInt = 11u

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

    requireNotNull(attributeData) { "Nodekeysetlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct.fromTlv(
              AnonymousTag,
              tlvReader,
            )
          )
        }
        tlvReader.exitContainer()
      }

    return NodeKeySetListAttribute(decodedValue)
  }

  suspend fun subscribeNodeKeySetListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NodeKeySetListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 11u
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
            NodeKeySetListAttributeSubscriptionState.Error(
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
            "Nodekeysetlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreNodeKeySetEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(NodeKeySetListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NodeKeySetListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNodeACLListAttribute(): NodeACLListAttribute {
    val ATTRIBUTE_ID: UInt = 12u

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

    requireNotNull(attributeData) { "Nodeacllist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreACLEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreACLEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(JointFabricDatastoreClusterDatastoreACLEntryStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return NodeACLListAttribute(decodedValue)
  }

  suspend fun subscribeNodeACLListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NodeACLListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 12u
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
            NodeACLListAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Nodeacllist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreACLEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreACLEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreACLEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(NodeACLListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NodeACLListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNodeEndpointListAttribute(): NodeEndpointListAttribute {
    val ATTRIBUTE_ID: UInt = 13u

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

    requireNotNull(attributeData) { "Nodeendpointlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointEntryStruct> =
      buildList<JointFabricDatastoreClusterDatastoreEndpointEntryStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(
            JointFabricDatastoreClusterDatastoreEndpointEntryStruct.fromTlv(AnonymousTag, tlvReader)
          )
        }
        tlvReader.exitContainer()
      }

    return NodeEndpointListAttribute(decodedValue)
  }

  suspend fun subscribeNodeEndpointListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NodeEndpointListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 13u
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
            NodeEndpointListAttributeSubscriptionState.Error(
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
            "Nodeendpointlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<JointFabricDatastoreClusterDatastoreEndpointEntryStruct> =
            buildList<JointFabricDatastoreClusterDatastoreEndpointEntryStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(
                  JointFabricDatastoreClusterDatastoreEndpointEntryStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }

          emit(NodeEndpointListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NodeEndpointListAttributeSubscriptionState.SubscriptionEstablished)
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
    private val logger = Logger.getLogger(JointFabricDatastoreCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1874u
  }
}
