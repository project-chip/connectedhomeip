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

import java.time.Duration
import java.util.Optional
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.controller.model.EventPath

private const val DEFAULT_SUBSCRIPTION_MIN_INTERVAL_S: Long = 0L
private const val DEFAULT_SUBSCRIPTION_MAX_INTERVAL_S: Long = 30L

/**
 * Representation of Timestamp type.
 *
 * This sealed class represents timestamps in two different formats:
 * - [MillisSinceBoot]: System Time in milliseconds since boot.
 * - [MillisSinceEpoch]: POSIX Time in milliseconds from the UNIX epoch (1970-01-01 00:00:00 UTC).
 *
 * @param value The time offset in milliseconds.
 */
sealed class Timestamp {
  data class MillisSinceBoot(val value: Long) : Timestamp()

  data class MillisSinceEpoch(val value: Long) : Timestamp()
}

/**
 * Information about a read request element.
 *
 * @param eventPaths A list of event path information in the read request.
 * @param attributePaths A list of attribute path information in the read request.
 */
class ReadRequest(
  val eventPaths: List<EventPath>,
  val attributePaths: List<AttributePath>,
)

/** Represents data received from a read operation. */
sealed class ReadData {
  /**
   * Represents data related to an event.
   *
   * @param path The event path associated with the data.
   * @param eventNumber the event number value that is scoped to the node
   * @param priorityLevel the priority describes the usage semantics of the event
   * @param timeStamp the timestamp at the time the event was created
   * @param data The ByteArray containing the data in TLV format.
   */
  class Event(
    val path: EventPath,
    val eventNumber: ULong,
    val priorityLevel: UByte,
    val timeStamp: Timestamp,
    val data: ByteArray
  ) : ReadData()

  /**
   * Represents data related to an attribute.
   *
   * @param path The attribute path associated with the data.
   * @param data The ByteArray containing the data in TLV format.
   */
  class Attribute(val path: AttributePath, val data: ByteArray) : ReadData()
}

/** Represents a failure that can occur during a read operation. */
sealed class ReadFailure {
  /**
   * Represents a failure related to an event path.
   *
   * @param path The event path information associated with the failure.
   * @param error The exception that describes the failure.
   */
  class Event(val path: EventPath, val error: Exception) : ReadFailure()

  /**
   * Represents a failure related to an attribute path.
   *
   * @param path The attribute path information associated with the failure.
   * @param error The exception that describes the failure.
   */
  class Attribute(val path: AttributePath, val error: Exception) : ReadFailure()
}

/**
 * Represents the response from a read operation, containing both successes and failures.
 *
 * @param successes A list of successfully read data elements.
 * @param failures A list of failures that occurred during the read operation.
 */
class ReadResponse(val successes: List<ReadData>, val failures: List<ReadFailure>)

/**
 * Information about a subscribe request element.
 *
 * @param eventPaths A list of event path information in the read request.
 * @param attributePaths A list of attribute path information in the read request.
 * @param minInterval The minimum interval boundary floor in seconds.
 * @param maxInterval The maximum interval boundary ceiling in seconds.
 * @param keepSubscriptions Indicates whether to keep existing subscriptions.
 * @param fabricFiltered Limits the data read within fabric-scoped lists to the accessing fabric.
 */
class SubscribeRequest(
  val eventPaths: List<EventPath>,
  val attributePaths: List<AttributePath>,
  val minInterval: Duration = Duration.ofSeconds(DEFAULT_SUBSCRIPTION_MIN_INTERVAL_S),
  val maxInterval: Duration = Duration.ofSeconds(DEFAULT_SUBSCRIPTION_MAX_INTERVAL_S),
  val keepSubscriptions: Boolean = true,
  val fabricFiltered: Boolean = true
)

/** An interface representing the possible states of a subscription. */
sealed class SubscriptionState {
  /**
   * Represents an error notification in the subscription.
   *
   * @param terminationCause The cause of the subscription termination.
   */
  class SubscriptionErrorNotification(val terminationCause: UInt) : SubscriptionState()

  /**
   * Represents an update in the state of a subscribed node.
   *
   * @param updateState The state update received from the subscribed node.
   */
  class NodeStateUpdate(val updateState: ReadResponse) : SubscriptionState()

  /** Represents the state where the subscription has been successfully established. */
  object SubscriptionEstablished : SubscriptionState()
}

/**
 * A write request representation.
 *
 * @param attributePath The attribute path information in the write request.
 * @param tlvPayload The ByteArray representation of the TLV payload.
 * @param dataVersion The data version in the write request.
 */
class WriteRequest(
  val attributePath: AttributePath,
  val tlvPayload: ByteArray,
  val dataVersion: UInt? = null
) {
  private fun getEndpointId(wildcardId: Long): Long {
    return attributePath.endpointId?.toLong() ?: wildcardId
  }

  private fun getClusterId(wildcardId: Long): Long {
    return attributePath.clusterId?.toLong() ?: wildcardId
  }

  private fun getAttributeId(wildcardId: Long): Long {
    return attributePath.attributeId?.toLong() ?: wildcardId
  }

  // For JNI interface
  private fun getJsonString(): String? = null

  fun getTlvByteArray(): ByteArray {
    return tlvPayload
  }

  fun hasDataVersion(): Boolean {
    return dataVersion != null
  }

  fun getDataVersion(): Int {
    return dataVersion?.toInt() ?: 0
  }
}

/**
 * Information about a collection of write request elements.
 *
 * @param requests A list of write request elements.
 * @param timedRequest If set, indicates that this is a timed request with the specified duration.
 */
class WriteRequests(val requests: List<WriteRequest>, val timedRequest: Duration?)

/**
 * Information about a write attribute error.
 *
 * @param attributePath The attribute path field in write response.
 * @param ex The IllegalStateException which encapsulated the error message.
 */
class AttributeWriteError(val attributePath: AttributePath, val ex: Exception)

/** An interface representing the possible write responses. */
sealed interface WriteResponse {
  object Success : WriteResponse

  class PartialWriteFailure(val failures: List<AttributeWriteError>) : WriteResponse
}

/**
 * Information about a invoke request element.
 *
 * @param commandPath Invoked command's path information.
 * @param tlvPayload The ByteArray representation of the TLV payload.
 * @param timedRequest If set, indicates that this is a timed request with the specified duration.
 */
class InvokeRequest(
  val commandPath: CommandPath,
  val tlvPayload: ByteArray,
  val timedRequest: Duration?,
  val jsonString: String? = null
) {
  private fun getEndpointId(wildcardId: Long): Long {
    return commandPath.endpointId?.toLong() ?: wildcardId
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getClusterId(wildcardId: Long): Long {
    return commandPath.clusterId.toLong()
  }

  @Suppress("UNUSED_PARAMETER")
  private fun getCommandId(wildcardId: Long): Long {
    return commandPath.commandId.toLong()
  }

  fun getGroupId(): Optional<Int> {
    return Optional.ofNullable(commandPath.groupId?.toInt())
  }

  fun isEndpointIdValid(): Boolean {
    return commandPath.groupId == null
  }

  fun isGroupIdValid(): Boolean {
    return commandPath.groupId != null
  }

  fun getTlvByteArray(): ByteArray {
    return tlvPayload
  }
}

/**
 * InvokeResponse will be received when a invoke response has been successful received and
 * processed.
 *
 * @param payload An invoke response that could contain tlv data or empty.
 */
class InvokeResponse(val payload: ByteArray, val path: CommandPath, val jsonString: String? = null)
