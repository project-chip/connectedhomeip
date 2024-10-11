/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package matter.controller.model

/**
 * Represents information about a node, including data on all available endpoints.
 *
 * @param endpoints A mapping of endpoint IDs with the associated cluster data.
 */
class NodeState(val endpoints: MutableMap<Int, EndpointState> = mutableMapOf()) {
  private fun addAttribute(
    endpointId: UShort,
    clusterId: UInt,
    attributeId: UInt,
    attributeState: AttributeState
  ) {
    getEndpointState(endpointId).addAttribute(clusterId, attributeId, attributeState)
  }

  private fun addEvent(endpointId: UShort, clusterId: UInt, eventId: UInt, eventState: EventState) {
    getEndpointState(endpointId).addEvent(clusterId, eventId, eventState)
  }

  private fun setDataVersion(endpointId: UShort, clusterId: UInt, dataVersion: UInt) {
    getEndpointState(endpointId).clusters[clusterId.toLong()]?.dataVersion = dataVersion
  }

  private fun addAttributeStatus(
    endpointId: UShort,
    clusterId: UInt,
    attributeId: UInt,
    statusToAdd: Status
  ) {
    val endpointState = getEndpointState(endpointId)
    val clusterState = endpointState.getClusterState(clusterId)
    if (clusterState.attributes[attributeId.toLong()] != null) {
      clusterState.attributes.remove(attributeId.toLong())
    }
    clusterState.attributeStatuses[attributeId.toLong()] = statusToAdd
  }

  private fun addEventStatus(
    endpointId: UShort,
    clusterId: UInt,
    eventId: UInt,
    statusToAdd: Status
  ) {
    val endpointState = getEndpointState(endpointId)
    val clusterState = endpointState.getClusterState(clusterId)
    if (clusterState.events[eventId.toLong()] != null) {
      clusterState.events.remove(eventId.toLong())
    }
    val eventStatuses = clusterState.eventStatuses.getOrDefault(eventId.toLong(), mutableListOf())
    eventStatuses.add(statusToAdd)

    clusterState.eventStatuses[eventId.toLong()] = eventStatuses
  }

  private fun getEndpointState(endpointId: UShort): EndpointState {
    var endpointState: EndpointState? = endpoints[endpointId.toInt()]
    if (endpointState == null) {
      endpointState = EndpointState(endpointId.toInt())
      endpoints[endpointId.toInt()] = endpointState
    }
    return endpointState
  }

  private fun addAttribute(
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    valueObject: Any,
    tlv: ByteArray,
    jsonString: String
  ) {
    addAttribute(
      endpointId.toUShort(),
      clusterId.toUInt(),
      attributeId.toUInt(),
      AttributeState(
        attributeId,
        tlv,
        jsonString,
        AttributePath(endpointId.toUShort(), clusterId.toUInt(), attributeId.toUInt()),
        valueObject
      )
    )
  }

  private fun addEvent(
    endpointId: Int,
    clusterId: Long,
    eventId: Long,
    eventNumber: Long,
    priorityLevel: Int,
    timestampType: Int,
    timestampValue: Long,
    valueObject: Any,
    tlv: ByteArray,
    jsonString: String
  ) {
    addEvent(
      endpointId.toUShort(),
      clusterId.toUInt(),
      eventId.toUInt(),
      EventState(
        eventId,
        eventNumber,
        priorityLevel,
        timestampType,
        timestampValue,
        tlv,
        EventPath(endpointId.toUShort(), clusterId.toUInt(), eventId.toUInt()),
        jsonString,
        valueObject
      )
    )
  }

  private fun setDataVersion(endpointId: Int, clusterId: Long, dataVersion: Long) {
    setDataVersion(endpointId.toUShort(), clusterId.toUInt(), dataVersion.toUInt())
  }

  private fun addAttributeStatus(
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    status: Int,
    clusterStatus: Int?
  ) {
    addAttributeStatus(
      endpointId.toUShort(),
      clusterId.toUInt(),
      attributeId.toUInt(),
      Status(status, clusterStatus)
    )
  }

  private fun addEventStatus(
    endpointId: Int,
    clusterId: Long,
    eventId: Long,
    status: Int,
    clusterStatus: Int?
  ) {
    addEventStatus(
      endpointId.toUShort(),
      clusterId.toUInt(),
      eventId.toUInt(),
      Status(status, clusterStatus)
    )
  }
}

/**
 * Represents information about an endpoint and its cluster data.
 *
 * @param id The endpoint ID.
 * @param clusters A mapping of cluster IDs to the cluster data.
 */
class EndpointState(val id: Int, val clusters: MutableMap<Long, ClusterState> = mutableMapOf()) {
  fun addAttribute(clusterId: UInt, attributeId: UInt, attributeState: AttributeState) {
    getClusterState(clusterId).addAttribute(attributeId, attributeState)
  }

  fun addEvent(clusterId: UInt, eventId: UInt, eventState: EventState) {
    getClusterState(clusterId).addEvent(eventId, eventState)
  }

  internal fun getClusterState(clusterId: UInt): ClusterState {
    var clusterState: ClusterState? = clusters[clusterId.toLong()]
    if (clusterState == null) {
      clusterState = ClusterState(clusterId.toLong())
      clusters[clusterId.toLong()] = clusterState
    }
    return clusterState
  }
}

/**
 * Represents information about a cluster.
 *
 * @param id The cluster ID.
 * @param attributes A mapping of attribute IDs in this cluster with their respective values.
 * @param events A mapping of event IDs to lists of events that occurred on the node under this
 *   cluster.
 */
class ClusterState(
  val id: Long,
  val attributes: MutableMap<Long, AttributeState> = mutableMapOf(),
  val events: MutableMap<Long, MutableList<EventState>> = mutableMapOf(),
  var dataVersion: UInt? = null,
  val attributeStatuses: MutableMap<Long, Status> = mutableMapOf(),
  val eventStatuses: MutableMap<Long, MutableList<Status>> = mutableMapOf()
) {
  fun addAttribute(attributeId: UInt, attributeState: AttributeState) {
    attributes[attributeId.toLong()] = attributeState
  }

  fun addEvent(eventId: UInt, eventState: EventState) {
    var eventStateList = events[eventId.toLong()]
    if (eventStateList == null) {
      eventStateList = mutableListOf()
      events[eventId.toLong()] = eventStateList
    }

    eventStateList.add(eventState)
  }
}

/**
 * Represents information about an attribute.
 *
 * @param id The attribute ID.
 * @param tlvValue The raw TLV-encoded attribute value.
 * @param jsonValue A JSON string representing the raw attribute value.
 */
data class AttributeState(
  val id: Long,
  val tlvValue: ByteArray,
  val jsonValue: String,
  val path: AttributePath,
  val valueObject: Any? = null
)

/**
 * Represents information about an event.
 *
 * @param eventId The event ID.
 * @param eventNumber The event number value that is scoped to the node.
 * @param priorityLevel The priority level describing the usage semantics of the event.
 * @param timestampType Indicates POSIX Time or SYSTEM Time, in milliseconds.
 * @param timestampValue Represents an offset, in milliseconds since the UNIX epoch or boot.
 * @param tlvValue The raw TLV-encoded event value.
 */
data class EventState(
  val eventId: Long,
  val eventNumber: Long,
  val priorityLevel: Int,
  val timestampType: Int,
  val timestampValue: Long,
  val tlvValue: ByteArray,
  val path: EventPath,
  val jsonValue: String? = null,
  val valueObject: Any? = null,
) {
  enum class TypeStampTypeEnum(val type: Int) {
    MILLIS_SINCE_BOOT(0),
    MILLIS_SINCE_EPOCH(1)
  }

  fun getTimestampType(): TypeStampTypeEnum? {
    for (type in TypeStampTypeEnum.values()) {
      if (type.type == timestampType) {
        return type
      }
    }
    return null
  }
}
