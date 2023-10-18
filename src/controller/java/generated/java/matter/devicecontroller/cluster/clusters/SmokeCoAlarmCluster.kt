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
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class SmokeCoAlarmCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun selfTestRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
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

  suspend fun readExpressedStateAttribute(): UByte {
    val ATTRIBUTE_ID_EXPRESSEDSTATE: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EXPRESSEDSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EXPRESSEDSTATE
        }

      requireNotNull(attributeData) { "Expressedstate attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readSmokeStateAttribute(): UByte? {
    val ATTRIBUTE_ID_SMOKESTATE: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SMOKESTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SMOKESTATE
        }

      requireNotNull(attributeData) { "Smokestate attribute not found in response" }

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

  suspend fun readCOStateAttribute(): UByte? {
    val ATTRIBUTE_ID_COSTATE: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_COSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_COSTATE
        }

      requireNotNull(attributeData) { "Costate attribute not found in response" }

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

  suspend fun readBatteryAlertAttribute(): UByte {
    val ATTRIBUTE_ID_BATTERYALERT: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BATTERYALERT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BATTERYALERT
        }

      requireNotNull(attributeData) { "Batteryalert attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDeviceMutedAttribute(): UByte? {
    val ATTRIBUTE_ID_DEVICEMUTED: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DEVICEMUTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DEVICEMUTED
        }

      requireNotNull(attributeData) { "Devicemuted attribute not found in response" }

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

  suspend fun readTestInProgressAttribute(): Boolean {
    val ATTRIBUTE_ID_TESTINPROGRESS: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TESTINPROGRESS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TESTINPROGRESS
        }

      requireNotNull(attributeData) { "Testinprogress attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readHardwareFaultAlertAttribute(): Boolean {
    val ATTRIBUTE_ID_HARDWAREFAULTALERT: UInt = 6u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_HARDWAREFAULTALERT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_HARDWAREFAULTALERT
        }

      requireNotNull(attributeData) { "Hardwarefaultalert attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readEndOfServiceAlertAttribute(): UByte {
    val ATTRIBUTE_ID_ENDOFSERVICEALERT: UInt = 7u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENDOFSERVICEALERT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENDOFSERVICEALERT
        }

      requireNotNull(attributeData) { "Endofservicealert attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readInterconnectSmokeAlarmAttribute(): UByte? {
    val ATTRIBUTE_ID_INTERCONNECTSMOKEALARM: UInt = 8u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INTERCONNECTSMOKEALARM
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INTERCONNECTSMOKEALARM
        }

      requireNotNull(attributeData) { "Interconnectsmokealarm attribute not found in response" }

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

  suspend fun readInterconnectCOAlarmAttribute(): UByte? {
    val ATTRIBUTE_ID_INTERCONNECTCOALARM: UInt = 9u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INTERCONNECTCOALARM
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INTERCONNECTCOALARM
        }

      requireNotNull(attributeData) { "Interconnectcoalarm attribute not found in response" }

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

  suspend fun readContaminationStateAttribute(): UByte? {
    val ATTRIBUTE_ID_CONTAMINATIONSTATE: UInt = 10u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CONTAMINATIONSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CONTAMINATIONSTATE
        }

      requireNotNull(attributeData) { "Contaminationstate attribute not found in response" }

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

  suspend fun readSmokeSensitivityLevelAttribute(): UByte? {
    val ATTRIBUTE_ID_SMOKESENSITIVITYLEVEL: UInt = 11u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SMOKESENSITIVITYLEVEL
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SMOKESENSITIVITYLEVEL
        }

      requireNotNull(attributeData) { "Smokesensitivitylevel attribute not found in response" }

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

  suspend fun writeSmokeSensitivityLevelAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_SMOKESENSITIVITYLEVEL: UInt = 11u
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
                  attributeId = ATTRIBUTE_ID_SMOKESENSITIVITYLEVEL
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

  suspend fun readExpiryDateAttribute(): UInt? {
    val ATTRIBUTE_ID_EXPIRYDATE: UInt = 12u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EXPIRYDATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EXPIRYDATE
        }

      requireNotNull(attributeData) { "Expirydate attribute not found in response" }

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
    private val logger = Logger.getLogger(SmokeCoAlarmCluster::class.java.name)
    const val CLUSTER_ID: UInt = 92u
  }
}
