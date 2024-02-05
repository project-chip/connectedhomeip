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
 * Represents a full path for reading an attribute from a node.
 *
 * @param endpointId The UShort representing the endpoint to read from.
 * @param clusterId The UInt representing the cluster on the endpoint to read from.
 * @param attributeId The UInt representing the attribute(s) on the cluster to read.
 */
data class AttributePath(val endpointId: UShort, val clusterId: UInt, val attributeId: UInt) {
  override fun toString(): String = "$endpointId/$clusterId/$attributeId"

  @Suppress("UNUSED_PARAMETER")
  private fun getEndpointId(wildcardId: Long): Long {
    return endpointId.toLong()
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getClusterId(wildcardId: Long): Long {
    return clusterId.toLong()
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getAttributeId(wildcardId: Long): Long {
    return attributeId.toLong()
  }
}

/**
 * Represents a full path to an event emitted from a node.
 *
 * @param endpointId The UShort representing the endpoint to read from.
 * @param clusterId The UInt representing the cluster on the endpoint to read from.
 * @param eventId The UInt representing the event(s) from the cluster.
 */
data class EventPath(
  val endpointId: UShort,
  val clusterId: UInt,
  val eventId: UInt,
  val isUrgent: Boolean = false
) {
  override fun toString(): String = "$endpointId/$clusterId/$eventId"

  @Suppress("UNUSED_PARAMETER")
  private fun getEndpointId(wildcardId: Long): Long {
    return endpointId.toLong()
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getClusterId(wildcardId: Long): Long {
    return clusterId.toLong()
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getEventId(wildcardId: Long): Long {
    return eventId.toLong()
  }
}

/**
 * Represents a full path to a command sent to a node.
 *
 * @param endpointId The UShort representing the endpoint to read from.
 * @param clusterId The UInt representing the cluster on the endpoint to read from.
 * @param commandId The UInt representing the command(s) from the cluster.
 */
data class CommandPath(
  val endpointId: UShort,
  val clusterId: UInt,
  val commandId: UInt,
  val groupId: UInt? = null
) {
  override fun toString(): String = "$endpointId/$clusterId/$commandId"
}
