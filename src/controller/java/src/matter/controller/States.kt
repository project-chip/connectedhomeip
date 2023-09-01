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
package matter.controller

/**
 * Information about a node, including data on all available endpoints.
 *
 * @param endpoints a mapping of endpoint IDs with the associated cluster data
 * @param events a list of events occurred on the node
 */
data class NodeState(val endpoints: Map<Int, EndpointState>, val events: ArrayList<EventState>)

/**
 * Information about an endpoint and its cluster data.
 *
 * @param id the endpoint ID
 * @param clusters a mapping of cluster IDs to the cluster data
 */
data class EndpointState(val id: Int, val clusters: Map<Long, ClusterState>)

/**
 * Information about a cluster.
 *
 * @param id the cluster ID
 * @param attributes a mapping of attribute IDs in this cluster with their respective values
 */
data class ClusterState(val id: Long, val attributes: Map<Long, AttributeState>)

/**
 * Information about an attribute.
 *
 * @param id the attribute ID
 * @param tlvValue the raw TLV-encoded attribute value
 * @param jsonValue a JSON string representing the raw attribute value
 */
data class AttributeState(val id: Long, val tlvValue: ByteArray, val jsonValue: String)

/**
 * Information about an event.
 *
 * @param eventId the event ID
 * @param endpointId the endpoint ID
 * @param clusterId the clusterId ID
 * @param eventNumber the event number value that is scoped to the node
 * @param priorityLevel the priority describes the usage semantics of the event
 * @param timestampType indicates POSIX Time or SYSTEM Time, in milliseconds
 * @param timestampValue represents an offset, in milliseconds since the UNIX epoch or boot
 * @param tlvValue the raw TLV-encoded event value
 */
data class EventState(
  val eventId: Long,
  val endpointId: Int,
  val clusterId: Long,
  val eventNumber: Long,
  val priorityLevel: Int,
  val timestampType: Int,
  val timestampValue: Long,
  val tlvValue: ByteArray,
)
