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
import matter.devicecontroller.cluster.structs.*
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class BinaryInputBasicCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readActiveTextAttribute(): String? {
    val ATTRIBUTE_ID_ACTIVETEXT: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVETEXT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVETEXT
        }

      requireNotNull(attributeData) { "Activetext attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: String? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getString(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeActiveTextAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_ACTIVETEXT: UInt = 4u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ACTIVETEXT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readDescriptionAttribute(): String? {
    val ATTRIBUTE_ID_DESCRIPTION: UInt = 28u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DESCRIPTION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DESCRIPTION
        }

      requireNotNull(attributeData) { "Description attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: String? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getString(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeDescriptionAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_DESCRIPTION: UInt = 28u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_DESCRIPTION
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInactiveTextAttribute(): String? {
    val ATTRIBUTE_ID_INACTIVETEXT: UInt = 46u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INACTIVETEXT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INACTIVETEXT
        }

      requireNotNull(attributeData) { "Inactivetext attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: String? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getString(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInactiveTextAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INACTIVETEXT: UInt = 46u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INACTIVETEXT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readOutOfServiceAttribute(): Boolean {
    val ATTRIBUTE_ID_OUTOFSERVICE: UInt = 81u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_OUTOFSERVICE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_OUTOFSERVICE
        }

      requireNotNull(attributeData) { "Outofservice attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeOutOfServiceAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_OUTOFSERVICE: UInt = 81u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_OUTOFSERVICE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readPolarityAttribute(): UByte? {
    val ATTRIBUTE_ID_POLARITY: UInt = 84u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POLARITY
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POLARITY
        }

      requireNotNull(attributeData) { "Polarity attribute not found in response" }

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

  suspend fun readPresentValueAttribute(): Boolean {
    val ATTRIBUTE_ID_PRESENTVALUE: UInt = 85u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_PRESENTVALUE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_PRESENTVALUE
        }

      requireNotNull(attributeData) { "Presentvalue attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writePresentValueAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_PRESENTVALUE: UInt = 85u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_PRESENTVALUE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readReliabilityAttribute(): UByte? {
    val ATTRIBUTE_ID_RELIABILITY: UInt = 103u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RELIABILITY
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RELIABILITY
        }

      requireNotNull(attributeData) { "Reliability attribute not found in response" }

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

  suspend fun writeReliabilityAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RELIABILITY: UInt = 103u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_RELIABILITY
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readStatusFlagsAttribute(): UByte {
    val ATTRIBUTE_ID_STATUSFLAGS: UInt = 111u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_STATUSFLAGS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_STATUSFLAGS
        }

      requireNotNull(attributeData) { "Statusflags attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readApplicationTypeAttribute(): UInt? {
    val ATTRIBUTE_ID_APPLICATIONTYPE: UInt = 256u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_APPLICATIONTYPE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_APPLICATIONTYPE
        }

      requireNotNull(attributeData) { "Applicationtype attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUInt(AnonymousTag)
        } else {
          null
        }

      return decodedValue
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
    private val logger = Logger.getLogger(BinaryInputBasicCluster::class.java.name)
    const val CLUSTER_ID: UInt = 15u
  }
}
