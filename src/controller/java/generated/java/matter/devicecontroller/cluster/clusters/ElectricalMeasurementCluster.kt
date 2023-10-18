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

class ElectricalMeasurementCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun getProfileInfoCommand(timedInvokeTimeoutMs: Int? = null) {
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

  suspend fun getMeasurementProfileCommand(
    attributeId: UShort,
    startTime: UInt,
    numberOfIntervals: UByte,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 1u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ATTRIBUTE_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ATTRIBUTE_ID_REQ), attributeId)

    val TAG_START_TIME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_START_TIME_REQ), startTime)

    val TAG_NUMBER_OF_INTERVALS_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_NUMBER_OF_INTERVALS_REQ), numberOfIntervals)
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

  suspend fun readMeasurementTypeAttribute(): UInt? {
    val ATTRIBUTE_ID_MEASUREMENTTYPE: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREMENTTYPE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREMENTTYPE
        }

      requireNotNull(attributeData) { "Measurementtype attribute not found in response" }

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

  suspend fun readDcVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_DCVOLTAGE: UInt = 256u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCVOLTAGE
        }

      requireNotNull(attributeData) { "Dcvoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcVoltageMinAttribute(): Short? {
    val ATTRIBUTE_ID_DCVOLTAGEMIN: UInt = 257u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCVOLTAGEMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCVOLTAGEMIN
        }

      requireNotNull(attributeData) { "Dcvoltagemin attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcVoltageMaxAttribute(): Short? {
    val ATTRIBUTE_ID_DCVOLTAGEMAX: UInt = 258u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCVOLTAGEMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCVOLTAGEMAX
        }

      requireNotNull(attributeData) { "Dcvoltagemax attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_DCCURRENT: UInt = 259u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCCURRENT
        }

      requireNotNull(attributeData) { "Dccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcCurrentMinAttribute(): Short? {
    val ATTRIBUTE_ID_DCCURRENTMIN: UInt = 260u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCCURRENTMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCCURRENTMIN
        }

      requireNotNull(attributeData) { "Dccurrentmin attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcCurrentMaxAttribute(): Short? {
    val ATTRIBUTE_ID_DCCURRENTMAX: UInt = 261u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCCURRENTMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCCURRENTMAX
        }

      requireNotNull(attributeData) { "Dccurrentmax attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcPowerAttribute(): Short? {
    val ATTRIBUTE_ID_DCPOWER: UInt = 262u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCPOWER
        }

      requireNotNull(attributeData) { "Dcpower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcPowerMinAttribute(): Short? {
    val ATTRIBUTE_ID_DCPOWERMIN: UInt = 263u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCPOWERMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCPOWERMIN
        }

      requireNotNull(attributeData) { "Dcpowermin attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcPowerMaxAttribute(): Short? {
    val ATTRIBUTE_ID_DCPOWERMAX: UInt = 264u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCPOWERMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCPOWERMAX
        }

      requireNotNull(attributeData) { "Dcpowermax attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDcVoltageMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_DCVOLTAGEMULTIPLIER: UInt = 512u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCVOLTAGEMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCVOLTAGEMULTIPLIER
        }

      requireNotNull(attributeData) { "Dcvoltagemultiplier attribute not found in response" }

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

  suspend fun readDcVoltageDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_DCVOLTAGEDIVISOR: UInt = 513u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCVOLTAGEDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCVOLTAGEDIVISOR
        }

      requireNotNull(attributeData) { "Dcvoltagedivisor attribute not found in response" }

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

  suspend fun readDcCurrentMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_DCCURRENTMULTIPLIER: UInt = 514u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCCURRENTMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCCURRENTMULTIPLIER
        }

      requireNotNull(attributeData) { "Dccurrentmultiplier attribute not found in response" }

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

  suspend fun readDcCurrentDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_DCCURRENTDIVISOR: UInt = 515u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCCURRENTDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCCURRENTDIVISOR
        }

      requireNotNull(attributeData) { "Dccurrentdivisor attribute not found in response" }

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

  suspend fun readDcPowerMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_DCPOWERMULTIPLIER: UInt = 516u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCPOWERMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCPOWERMULTIPLIER
        }

      requireNotNull(attributeData) { "Dcpowermultiplier attribute not found in response" }

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

  suspend fun readDcPowerDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_DCPOWERDIVISOR: UInt = 517u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DCPOWERDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DCPOWERDIVISOR
        }

      requireNotNull(attributeData) { "Dcpowerdivisor attribute not found in response" }

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

  suspend fun readAcFrequencyAttribute(): UShort? {
    val ATTRIBUTE_ID_ACFREQUENCY: UInt = 768u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACFREQUENCY
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACFREQUENCY
        }

      requireNotNull(attributeData) { "Acfrequency attribute not found in response" }

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

  suspend fun readAcFrequencyMinAttribute(): UShort? {
    val ATTRIBUTE_ID_ACFREQUENCYMIN: UInt = 769u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACFREQUENCYMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACFREQUENCYMIN
        }

      requireNotNull(attributeData) { "Acfrequencymin attribute not found in response" }

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

  suspend fun readAcFrequencyMaxAttribute(): UShort? {
    val ATTRIBUTE_ID_ACFREQUENCYMAX: UInt = 770u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACFREQUENCYMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACFREQUENCYMAX
        }

      requireNotNull(attributeData) { "Acfrequencymax attribute not found in response" }

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

  suspend fun readNeutralCurrentAttribute(): UShort? {
    val ATTRIBUTE_ID_NEUTRALCURRENT: UInt = 771u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NEUTRALCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NEUTRALCURRENT
        }

      requireNotNull(attributeData) { "Neutralcurrent attribute not found in response" }

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

  suspend fun readTotalActivePowerAttribute(): Int? {
    val ATTRIBUTE_ID_TOTALACTIVEPOWER: UInt = 772u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TOTALACTIVEPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TOTALACTIVEPOWER
        }

      requireNotNull(attributeData) { "Totalactivepower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Int? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getInt(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readTotalReactivePowerAttribute(): Int? {
    val ATTRIBUTE_ID_TOTALREACTIVEPOWER: UInt = 773u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TOTALREACTIVEPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TOTALREACTIVEPOWER
        }

      requireNotNull(attributeData) { "Totalreactivepower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Int? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getInt(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readTotalApparentPowerAttribute(): UInt? {
    val ATTRIBUTE_ID_TOTALAPPARENTPOWER: UInt = 774u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TOTALAPPARENTPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TOTALAPPARENTPOWER
        }

      requireNotNull(attributeData) { "Totalapparentpower attribute not found in response" }

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

  suspend fun readMeasured1stHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED1STHARMONICCURRENT: UInt = 775u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED1STHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED1STHARMONICCURRENT
        }

      requireNotNull(attributeData) { "Measured1stharmoniccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasured3rdHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED3RDHARMONICCURRENT: UInt = 776u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED3RDHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED3RDHARMONICCURRENT
        }

      requireNotNull(attributeData) { "Measured3rdharmoniccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasured5thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED5THHARMONICCURRENT: UInt = 777u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED5THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED5THHARMONICCURRENT
        }

      requireNotNull(attributeData) { "Measured5thharmoniccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasured7thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED7THHARMONICCURRENT: UInt = 778u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED7THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED7THHARMONICCURRENT
        }

      requireNotNull(attributeData) { "Measured7thharmoniccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasured9thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED9THHARMONICCURRENT: UInt = 779u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED9THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED9THHARMONICCURRENT
        }

      requireNotNull(attributeData) { "Measured9thharmoniccurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasured11thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASURED11THHARMONICCURRENT: UInt = 780u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASURED11THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASURED11THHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measured11thharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase1stHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE1STHARMONICCURRENT: UInt = 781u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE1STHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE1STHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase1stharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase3rdHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE3RDHARMONICCURRENT: UInt = 782u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE3RDHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE3RDHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase3rdharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase5thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE5THHARMONICCURRENT: UInt = 783u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE5THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE5THHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase5thharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase7thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE7THHARMONICCURRENT: UInt = 784u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE7THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE7THHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase7thharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase9thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE9THHARMONICCURRENT: UInt = 785u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE9THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE9THHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase9thharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMeasuredPhase11thHarmonicCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_MEASUREDPHASE11THHARMONICCURRENT: UInt = 786u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MEASUREDPHASE11THHARMONICCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MEASUREDPHASE11THHARMONICCURRENT
        }

      requireNotNull(attributeData) {
        "Measuredphase11thharmoniccurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcFrequencyMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_ACFREQUENCYMULTIPLIER: UInt = 1024u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACFREQUENCYMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACFREQUENCYMULTIPLIER
        }

      requireNotNull(attributeData) { "Acfrequencymultiplier attribute not found in response" }

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

  suspend fun readAcFrequencyDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_ACFREQUENCYDIVISOR: UInt = 1025u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACFREQUENCYDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACFREQUENCYDIVISOR
        }

      requireNotNull(attributeData) { "Acfrequencydivisor attribute not found in response" }

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

  suspend fun readPowerMultiplierAttribute(): UInt? {
    val ATTRIBUTE_ID_POWERMULTIPLIER: UInt = 1026u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POWERMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POWERMULTIPLIER
        }

      requireNotNull(attributeData) { "Powermultiplier attribute not found in response" }

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

  suspend fun readPowerDivisorAttribute(): UInt? {
    val ATTRIBUTE_ID_POWERDIVISOR: UInt = 1027u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POWERDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POWERDIVISOR
        }

      requireNotNull(attributeData) { "Powerdivisor attribute not found in response" }

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

  suspend fun readHarmonicCurrentMultiplierAttribute(): Byte? {
    val ATTRIBUTE_ID_HARMONICCURRENTMULTIPLIER: UInt = 1028u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_HARMONICCURRENTMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_HARMONICCURRENTMULTIPLIER
        }

      requireNotNull(attributeData) { "Harmoniccurrentmultiplier attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readPhaseHarmonicCurrentMultiplierAttribute(): Byte? {
    val ATTRIBUTE_ID_PHASEHARMONICCURRENTMULTIPLIER: UInt = 1029u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_PHASEHARMONICCURRENTMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_PHASEHARMONICCURRENTMULTIPLIER
        }

      requireNotNull(attributeData) {
        "Phaseharmoniccurrentmultiplier attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readInstantaneousVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_INSTANTANEOUSVOLTAGE: UInt = 1280u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INSTANTANEOUSVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INSTANTANEOUSVOLTAGE
        }

      requireNotNull(attributeData) { "Instantaneousvoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readInstantaneousLineCurrentAttribute(): UShort? {
    val ATTRIBUTE_ID_INSTANTANEOUSLINECURRENT: UInt = 1281u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INSTANTANEOUSLINECURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INSTANTANEOUSLINECURRENT
        }

      requireNotNull(attributeData) { "Instantaneouslinecurrent attribute not found in response" }

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

  suspend fun readInstantaneousActiveCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_INSTANTANEOUSACTIVECURRENT: UInt = 1282u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INSTANTANEOUSACTIVECURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INSTANTANEOUSACTIVECURRENT
        }

      requireNotNull(attributeData) { "Instantaneousactivecurrent attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readInstantaneousReactiveCurrentAttribute(): Short? {
    val ATTRIBUTE_ID_INSTANTANEOUSREACTIVECURRENT: UInt = 1283u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INSTANTANEOUSREACTIVECURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INSTANTANEOUSREACTIVECURRENT
        }

      requireNotNull(attributeData) {
        "Instantaneousreactivecurrent attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readInstantaneousPowerAttribute(): Short? {
    val ATTRIBUTE_ID_INSTANTANEOUSPOWER: UInt = 1284u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INSTANTANEOUSPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INSTANTANEOUSPOWER
        }

      requireNotNull(attributeData) { "Instantaneouspower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsVoltageAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGE: UInt = 1285u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGE
        }

      requireNotNull(attributeData) { "Rmsvoltage attribute not found in response" }

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

  suspend fun readRmsVoltageMinAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMIN: UInt = 1286u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMIN
        }

      requireNotNull(attributeData) { "Rmsvoltagemin attribute not found in response" }

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

  suspend fun readRmsVoltageMaxAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMAX: UInt = 1287u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMAX
        }

      requireNotNull(attributeData) { "Rmsvoltagemax attribute not found in response" }

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

  suspend fun readRmsCurrentAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENT: UInt = 1288u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENT
        }

      requireNotNull(attributeData) { "Rmscurrent attribute not found in response" }

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

  suspend fun readRmsCurrentMinAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMIN: UInt = 1289u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMIN
        }

      requireNotNull(attributeData) { "Rmscurrentmin attribute not found in response" }

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

  suspend fun readRmsCurrentMaxAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMAX: UInt = 1290u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMAX
        }

      requireNotNull(attributeData) { "Rmscurrentmax attribute not found in response" }

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

  suspend fun readActivePowerAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWER: UInt = 1291u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWER
        }

      requireNotNull(attributeData) { "Activepower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMinAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMIN: UInt = 1292u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMIN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMIN
        }

      requireNotNull(attributeData) { "Activepowermin attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMaxAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMAX: UInt = 1293u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMAX
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMAX
        }

      requireNotNull(attributeData) { "Activepowermax attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readReactivePowerAttribute(): Short? {
    val ATTRIBUTE_ID_REACTIVEPOWER: UInt = 1294u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REACTIVEPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REACTIVEPOWER
        }

      requireNotNull(attributeData) { "Reactivepower attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readApparentPowerAttribute(): UShort? {
    val ATTRIBUTE_ID_APPARENTPOWER: UInt = 1295u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_APPARENTPOWER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_APPARENTPOWER
        }

      requireNotNull(attributeData) { "Apparentpower attribute not found in response" }

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

  suspend fun readPowerFactorAttribute(): Byte? {
    val ATTRIBUTE_ID_POWERFACTOR: UInt = 1296u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POWERFACTOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POWERFACTOR
        }

      requireNotNull(attributeData) { "Powerfactor attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIOD: UInt = 1297u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIOD
        }

      requireNotNull(attributeData) {
        "Averagermsvoltagemeasurementperiod attribute not found in response"
      }

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

  suspend fun writeAverageRmsVoltageMeasurementPeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIOD: UInt = 1297u
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
                  attributeId = ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIOD
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

  suspend fun readAverageRmsUnderVoltageCounterAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTER: UInt = 1299u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTER
        }

      requireNotNull(attributeData) {
        "Averagermsundervoltagecounter attribute not found in response"
      }

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

  suspend fun writeAverageRmsUnderVoltageCounterAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTER: UInt = 1299u
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
                  attributeId = ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTER
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

  suspend fun readRmsExtremeOverVoltagePeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIOD: UInt = 1300u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIOD
        }

      requireNotNull(attributeData) {
        "Rmsextremeovervoltageperiod attribute not found in response"
      }

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

  suspend fun writeRmsExtremeOverVoltagePeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIOD: UInt = 1300u
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
                  attributeId = ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIOD
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

  suspend fun readRmsExtremeUnderVoltagePeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIOD: UInt = 1301u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIOD
        }

      requireNotNull(attributeData) {
        "Rmsextremeundervoltageperiod attribute not found in response"
      }

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

  suspend fun writeRmsExtremeUnderVoltagePeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIOD: UInt = 1301u
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
                  attributeId = ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIOD
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

  suspend fun readRmsVoltageSagPeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESAGPERIOD: UInt = 1302u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESAGPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESAGPERIOD
        }

      requireNotNull(attributeData) { "Rmsvoltagesagperiod attribute not found in response" }

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

  suspend fun writeRmsVoltageSagPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RMSVOLTAGESAGPERIOD: UInt = 1302u
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
                  attributeId = ATTRIBUTE_ID_RMSVOLTAGESAGPERIOD
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

  suspend fun readRmsVoltageSwellPeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESWELLPERIOD: UInt = 1303u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESWELLPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESWELLPERIOD
        }

      requireNotNull(attributeData) { "Rmsvoltageswellperiod attribute not found in response" }

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

  suspend fun writeRmsVoltageSwellPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RMSVOLTAGESWELLPERIOD: UInt = 1303u
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
                  attributeId = ATTRIBUTE_ID_RMSVOLTAGESWELLPERIOD
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

  suspend fun readAcVoltageMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_ACVOLTAGEMULTIPLIER: UInt = 1536u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACVOLTAGEMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACVOLTAGEMULTIPLIER
        }

      requireNotNull(attributeData) { "Acvoltagemultiplier attribute not found in response" }

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

  suspend fun readAcVoltageDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_ACVOLTAGEDIVISOR: UInt = 1537u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACVOLTAGEDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACVOLTAGEDIVISOR
        }

      requireNotNull(attributeData) { "Acvoltagedivisor attribute not found in response" }

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

  suspend fun readAcCurrentMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_ACCURRENTMULTIPLIER: UInt = 1538u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCURRENTMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCURRENTMULTIPLIER
        }

      requireNotNull(attributeData) { "Accurrentmultiplier attribute not found in response" }

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

  suspend fun readAcCurrentDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_ACCURRENTDIVISOR: UInt = 1539u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCURRENTDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCURRENTDIVISOR
        }

      requireNotNull(attributeData) { "Accurrentdivisor attribute not found in response" }

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

  suspend fun readAcPowerMultiplierAttribute(): UShort? {
    val ATTRIBUTE_ID_ACPOWERMULTIPLIER: UInt = 1540u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACPOWERMULTIPLIER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACPOWERMULTIPLIER
        }

      requireNotNull(attributeData) { "Acpowermultiplier attribute not found in response" }

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

  suspend fun readAcPowerDivisorAttribute(): UShort? {
    val ATTRIBUTE_ID_ACPOWERDIVISOR: UInt = 1541u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACPOWERDIVISOR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACPOWERDIVISOR
        }

      requireNotNull(attributeData) { "Acpowerdivisor attribute not found in response" }

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

  suspend fun readOverloadAlarmsMaskAttribute(): UByte? {
    val ATTRIBUTE_ID_OVERLOADALARMSMASK: UInt = 1792u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_OVERLOADALARMSMASK
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_OVERLOADALARMSMASK
        }

      requireNotNull(attributeData) { "Overloadalarmsmask attribute not found in response" }

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

  suspend fun writeOverloadAlarmsMaskAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_OVERLOADALARMSMASK: UInt = 1792u
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
                  attributeId = ATTRIBUTE_ID_OVERLOADALARMSMASK
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

  suspend fun readVoltageOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_VOLTAGEOVERLOAD: UInt = 1793u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_VOLTAGEOVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_VOLTAGEOVERLOAD
        }

      requireNotNull(attributeData) { "Voltageoverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCurrentOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_CURRENTOVERLOAD: UInt = 1794u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CURRENTOVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CURRENTOVERLOAD
        }

      requireNotNull(attributeData) { "Currentoverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcOverloadAlarmsMaskAttribute(): UShort? {
    val ATTRIBUTE_ID_ACOVERLOADALARMSMASK: UInt = 2048u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACOVERLOADALARMSMASK
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACOVERLOADALARMSMASK
        }

      requireNotNull(attributeData) { "Acoverloadalarmsmask attribute not found in response" }

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

  suspend fun writeAcOverloadAlarmsMaskAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_ACOVERLOADALARMSMASK: UInt = 2048u
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
                  attributeId = ATTRIBUTE_ID_ACOVERLOADALARMSMASK
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

  suspend fun readAcVoltageOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_ACVOLTAGEOVERLOAD: UInt = 2049u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACVOLTAGEOVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACVOLTAGEOVERLOAD
        }

      requireNotNull(attributeData) { "Acvoltageoverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcCurrentOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_ACCURRENTOVERLOAD: UInt = 2050u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCURRENTOVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCURRENTOVERLOAD
        }

      requireNotNull(attributeData) { "Accurrentoverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcActivePowerOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_ACACTIVEPOWEROVERLOAD: UInt = 2051u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACACTIVEPOWEROVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACACTIVEPOWEROVERLOAD
        }

      requireNotNull(attributeData) { "Acactivepoweroverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcReactivePowerOverloadAttribute(): Short? {
    val ATTRIBUTE_ID_ACREACTIVEPOWEROVERLOAD: UInt = 2052u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACREACTIVEPOWEROVERLOAD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACREACTIVEPOWEROVERLOAD
        }

      requireNotNull(attributeData) { "Acreactivepoweroverload attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAverageRmsOverVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGE: UInt = 2053u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGE
        }

      requireNotNull(attributeData) { "Averagermsovervoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAverageRmsUnderVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGE: UInt = 2054u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGE
        }

      requireNotNull(attributeData) { "Averagermsundervoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsExtremeOverVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGE: UInt = 2055u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGE
        }

      requireNotNull(attributeData) { "Rmsextremeovervoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsExtremeUnderVoltageAttribute(): Short? {
    val ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGE: UInt = 2056u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGE
        }

      requireNotNull(attributeData) { "Rmsextremeundervoltage attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsVoltageSagAttribute(): Short? {
    val ATTRIBUTE_ID_RMSVOLTAGESAG: UInt = 2057u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESAG
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESAG
        }

      requireNotNull(attributeData) { "Rmsvoltagesag attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsVoltageSwellAttribute(): Short? {
    val ATTRIBUTE_ID_RMSVOLTAGESWELL: UInt = 2058u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESWELL
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESWELL
        }

      requireNotNull(attributeData) { "Rmsvoltageswell attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readLineCurrentPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_LINECURRENTPHASEB: UInt = 2305u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LINECURRENTPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LINECURRENTPHASEB
        }

      requireNotNull(attributeData) { "Linecurrentphaseb attribute not found in response" }

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

  suspend fun readActiveCurrentPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVECURRENTPHASEB: UInt = 2306u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVECURRENTPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVECURRENTPHASEB
        }

      requireNotNull(attributeData) { "Activecurrentphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readReactiveCurrentPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_REACTIVECURRENTPHASEB: UInt = 2307u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REACTIVECURRENTPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REACTIVECURRENTPHASEB
        }

      requireNotNull(attributeData) { "Reactivecurrentphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsVoltagePhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEPHASEB: UInt = 2309u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEPHASEB
        }

      requireNotNull(attributeData) { "Rmsvoltagephaseb attribute not found in response" }

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

  suspend fun readRmsVoltageMinPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMINPHASEB: UInt = 2310u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMINPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMINPHASEB
        }

      requireNotNull(attributeData) { "Rmsvoltageminphaseb attribute not found in response" }

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

  suspend fun readRmsVoltageMaxPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEB: UInt = 2311u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEB
        }

      requireNotNull(attributeData) { "Rmsvoltagemaxphaseb attribute not found in response" }

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

  suspend fun readRmsCurrentPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTPHASEB: UInt = 2312u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTPHASEB
        }

      requireNotNull(attributeData) { "Rmscurrentphaseb attribute not found in response" }

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

  suspend fun readRmsCurrentMinPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMINPHASEB: UInt = 2313u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMINPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMINPHASEB
        }

      requireNotNull(attributeData) { "Rmscurrentminphaseb attribute not found in response" }

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

  suspend fun readRmsCurrentMaxPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMAXPHASEB: UInt = 2314u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMAXPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMAXPHASEB
        }

      requireNotNull(attributeData) { "Rmscurrentmaxphaseb attribute not found in response" }

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

  suspend fun readActivePowerPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERPHASEB: UInt = 2315u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERPHASEB
        }

      requireNotNull(attributeData) { "Activepowerphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMinPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMINPHASEB: UInt = 2316u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMINPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMINPHASEB
        }

      requireNotNull(attributeData) { "Activepowerminphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMaxPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEB: UInt = 2317u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEB
        }

      requireNotNull(attributeData) { "Activepowermaxphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readReactivePowerPhaseBAttribute(): Short? {
    val ATTRIBUTE_ID_REACTIVEPOWERPHASEB: UInt = 2318u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REACTIVEPOWERPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REACTIVEPOWERPHASEB
        }

      requireNotNull(attributeData) { "Reactivepowerphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readApparentPowerPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_APPARENTPOWERPHASEB: UInt = 2319u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_APPARENTPOWERPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_APPARENTPOWERPHASEB
        }

      requireNotNull(attributeData) { "Apparentpowerphaseb attribute not found in response" }

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

  suspend fun readPowerFactorPhaseBAttribute(): Byte? {
    val ATTRIBUTE_ID_POWERFACTORPHASEB: UInt = 2320u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POWERFACTORPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POWERFACTORPHASEB
        }

      requireNotNull(attributeData) { "Powerfactorphaseb attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEB: UInt = 2321u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEB
        }

      requireNotNull(attributeData) {
        "Averagermsvoltagemeasurementperiodphaseb attribute not found in response"
      }

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

  suspend fun readAverageRmsOverVoltageCounterPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEB: UInt = 2322u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEB
        }

      requireNotNull(attributeData) {
        "Averagermsovervoltagecounterphaseb attribute not found in response"
      }

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

  suspend fun readAverageRmsUnderVoltageCounterPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEB: UInt = 2323u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEB
        }

      requireNotNull(attributeData) {
        "Averagermsundervoltagecounterphaseb attribute not found in response"
      }

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

  suspend fun readRmsExtremeOverVoltagePeriodPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEB: UInt = 2324u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEB
        }

      requireNotNull(attributeData) {
        "Rmsextremeovervoltageperiodphaseb attribute not found in response"
      }

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

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEB: UInt = 2325u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEB
        }

      requireNotNull(attributeData) {
        "Rmsextremeundervoltageperiodphaseb attribute not found in response"
      }

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

  suspend fun readRmsVoltageSagPeriodPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEB: UInt = 2326u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEB
        }

      requireNotNull(attributeData) { "Rmsvoltagesagperiodphaseb attribute not found in response" }

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

  suspend fun readRmsVoltageSwellPeriodPhaseBAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEB: UInt = 2327u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEB
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEB
        }

      requireNotNull(attributeData) {
        "Rmsvoltageswellperiodphaseb attribute not found in response"
      }

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

  suspend fun readLineCurrentPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_LINECURRENTPHASEC: UInt = 2561u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LINECURRENTPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LINECURRENTPHASEC
        }

      requireNotNull(attributeData) { "Linecurrentphasec attribute not found in response" }

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

  suspend fun readActiveCurrentPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVECURRENTPHASEC: UInt = 2562u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVECURRENTPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVECURRENTPHASEC
        }

      requireNotNull(attributeData) { "Activecurrentphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readReactiveCurrentPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_REACTIVECURRENTPHASEC: UInt = 2563u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REACTIVECURRENTPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REACTIVECURRENTPHASEC
        }

      requireNotNull(attributeData) { "Reactivecurrentphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readRmsVoltagePhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEPHASEC: UInt = 2565u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEPHASEC
        }

      requireNotNull(attributeData) { "Rmsvoltagephasec attribute not found in response" }

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

  suspend fun readRmsVoltageMinPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMINPHASEC: UInt = 2566u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMINPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMINPHASEC
        }

      requireNotNull(attributeData) { "Rmsvoltageminphasec attribute not found in response" }

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

  suspend fun readRmsVoltageMaxPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEC: UInt = 2567u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGEMAXPHASEC
        }

      requireNotNull(attributeData) { "Rmsvoltagemaxphasec attribute not found in response" }

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

  suspend fun readRmsCurrentPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTPHASEC: UInt = 2568u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTPHASEC
        }

      requireNotNull(attributeData) { "Rmscurrentphasec attribute not found in response" }

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

  suspend fun readRmsCurrentMinPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMINPHASEC: UInt = 2569u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMINPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMINPHASEC
        }

      requireNotNull(attributeData) { "Rmscurrentminphasec attribute not found in response" }

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

  suspend fun readRmsCurrentMaxPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSCURRENTMAXPHASEC: UInt = 2570u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSCURRENTMAXPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSCURRENTMAXPHASEC
        }

      requireNotNull(attributeData) { "Rmscurrentmaxphasec attribute not found in response" }

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

  suspend fun readActivePowerPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERPHASEC: UInt = 2571u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERPHASEC
        }

      requireNotNull(attributeData) { "Activepowerphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMinPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMINPHASEC: UInt = 2572u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMINPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMINPHASEC
        }

      requireNotNull(attributeData) { "Activepowerminphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActivePowerMaxPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEC: UInt = 2573u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTIVEPOWERMAXPHASEC
        }

      requireNotNull(attributeData) { "Activepowermaxphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readReactivePowerPhaseCAttribute(): Short? {
    val ATTRIBUTE_ID_REACTIVEPOWERPHASEC: UInt = 2574u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REACTIVEPOWERPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REACTIVEPOWERPHASEC
        }

      requireNotNull(attributeData) { "Reactivepowerphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getShort(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readApparentPowerPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_APPARENTPOWERPHASEC: UInt = 2575u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_APPARENTPOWERPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_APPARENTPOWERPHASEC
        }

      requireNotNull(attributeData) { "Apparentpowerphasec attribute not found in response" }

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

  suspend fun readPowerFactorPhaseCAttribute(): Byte? {
    val ATTRIBUTE_ID_POWERFACTORPHASEC: UInt = 2576u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_POWERFACTORPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_POWERFACTORPHASEC
        }

      requireNotNull(attributeData) { "Powerfactorphasec attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByte(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEC: UInt = 2577u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSVOLTAGEMEASUREMENTPERIODPHASEC
        }

      requireNotNull(attributeData) {
        "Averagermsvoltagemeasurementperiodphasec attribute not found in response"
      }

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

  suspend fun readAverageRmsOverVoltageCounterPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEC: UInt = 2578u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSOVERVOLTAGECOUNTERPHASEC
        }

      requireNotNull(attributeData) {
        "Averagermsovervoltagecounterphasec attribute not found in response"
      }

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

  suspend fun readAverageRmsUnderVoltageCounterPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEC: UInt = 2579u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AVERAGERMSUNDERVOLTAGECOUNTERPHASEC
        }

      requireNotNull(attributeData) {
        "Averagermsundervoltagecounterphasec attribute not found in response"
      }

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

  suspend fun readRmsExtremeOverVoltagePeriodPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEC: UInt = 2580u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEOVERVOLTAGEPERIODPHASEC
        }

      requireNotNull(attributeData) {
        "Rmsextremeovervoltageperiodphasec attribute not found in response"
      }

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

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEC: UInt = 2581u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSEXTREMEUNDERVOLTAGEPERIODPHASEC
        }

      requireNotNull(attributeData) {
        "Rmsextremeundervoltageperiodphasec attribute not found in response"
      }

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

  suspend fun readRmsVoltageSagPeriodPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEC: UInt = 2582u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESAGPERIODPHASEC
        }

      requireNotNull(attributeData) { "Rmsvoltagesagperiodphasec attribute not found in response" }

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

  suspend fun readRmsVoltageSwellPeriodPhaseCAttribute(): UShort? {
    val ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEC: UInt = 2583u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEC
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RMSVOLTAGESWELLPERIODPHASEC
        }

      requireNotNull(attributeData) {
        "Rmsvoltageswellperiodphasec attribute not found in response"
      }

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
    private val logger = Logger.getLogger(ElectricalMeasurementCluster::class.java.name)
    const val CLUSTER_ID: UInt = 2820u
  }
}
