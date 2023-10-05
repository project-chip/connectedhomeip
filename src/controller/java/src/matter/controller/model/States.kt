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
data class NodeState(val endpoints: Map<Int, EndpointState>)

/**
 * Represents information about an endpoint and its cluster data.
 *
 * @param id The endpoint ID.
 * @param clusters A mapping of cluster IDs to the cluster data.
 */
data class EndpointState(val id: Int, val clusters: Map<Long, ClusterState>)

/**
 * Represents information about a cluster.
 *
 * @param id The cluster ID.
 * @param attributes A mapping of attribute IDs in this cluster with their respective values.
 * @param events A mapping of event IDs to lists of events that occurred on the node under this
 *   cluster.
 */
data class ClusterState(
  val id: Long,
  val attributes: Map<Long, AttributeState>,
  val events: Map<Long, List<EventState>>
)

/**
 * Represents information about an attribute.
 *
 * @param id The attribute ID.
 * @param tlvValue The raw TLV-encoded attribute value.
 * @param jsonValue A JSON string representing the raw attribute value.
 */
data class AttributeState(val id: Long, val tlvValue: ByteArray, val jsonValue: String)

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
  val tlvValue: ByteArray
)
