/*
 *   Copyright (c) 2026 Project CHIP Authors
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
package com.google.chip.chiptool.devicemanagement

import com.google.gson.JsonParser
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class UniversalTlvEncoderTest {

  @Test
  fun testEncodeOnOffCommand() {
    val cluster = MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_ON_OFF)!!
    val onCmd = cluster.commands[0x01L]!!
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(onCmd, emptyMap())

    assertNotNull(tlvBytes)
    assertTrue(tlvBytes.isNotEmpty())

    // Validate TLV can be read back as an empty structure
    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeMoveToLevelCommand() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL)!!
    val cmd = cluster.commands[0x00L]!!
    val params =
      mapOf("level" to 128L, "transitionTime" to 10L, "optionsMask" to 0L, "optionsOverride" to 0L)
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    assertNotNull(tlvBytes)
    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(128.toUByte(), reader.getUByte(ContextSpecificTag(0)))
    assertEquals(10.toUShort(), reader.getUShort(ContextSpecificTag(1)))
    assertEquals(0.toUByte(), reader.getUByte(ContextSpecificTag(2)))
    assertEquals(0.toUByte(), reader.getUByte(ContextSpecificTag(3)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeMoveToColorTemperatureCommand() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL)!!
    val cmd = cluster.commands[0x0AL]!!
    val mireds = MatterClusterMetaRegistry.kelvinToMireds(2700) // 370
    val params = mapOf("colorTemperatureMireds" to mireds.toLong(), "transitionTime" to 5L)
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(370.toUShort(), reader.getUShort(ContextSpecificTag(0)))
    assertEquals(5.toUShort(), reader.getUShort(ContextSpecificTag(1)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeUnlockWithTimeout() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK)!!
    val cmd = cluster.commands[0x03L]!!
    val params = mapOf("timeoutSeconds" to 60L)
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(60.toUShort(), reader.getUShort(ContextSpecificTag(0)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeWindowCoveringGoToLiftPercentage() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING)!!
    val cmd = cluster.commands[0x05L]!!
    // 75% -> 7500 in 100ths
    val params = mapOf("liftPercent100thsValue" to 7500L)
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(7500.toUShort(), reader.getUShort(ContextSpecificTag(0)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeThermostatSetpointRaiseLower() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_THERMOSTAT)!!
    val cmd = cluster.commands[0x00L]!!
    val params =
      mapOf(
        "mode" to 0L, // Heat
        "amount" to 20L, // +2.0 deg C (20 * 0.1C)
      )
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(0.toUByte(), reader.getUByte(ContextSpecificTag(0)))
    assertEquals(20.toByte(), reader.getByte(ContextSpecificTag(1)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeRvcChangeToMode() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_RVC_RUN_MODE)!!
    val cmd = cluster.commands[0x00L]!!
    val params = mapOf("newMode" to 1L) // Cleaning
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    assertEquals(1.toUByte(), reader.getUByte(ContextSpecificTag(0)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testEncodeEvseEnableCharging() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_ENERGY_EVSE)!!
    val cmd = cluster.commands[0x02L]!!
    val params =
      mapOf(
        "chargingEnabledUntil" to null,
        "minimumChargeCurrent" to 6000L,
        "maximumChargeCurrent" to 32000L,
      )
    val tlvBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, params)

    val reader = TlvReader(tlvBytes)
    reader.enterStructure(AnonymousTag)
    reader.getNull(ContextSpecificTag(0))
    assertEquals(6000L, reader.getLong(ContextSpecificTag(1)))
    assertEquals(32000L, reader.getLong(ContextSpecificTag(2)))
    reader.exitContainer()
    assertTrue(reader.isEndOfTlv())
  }

  @Test
  fun testDecodeTemperatureAttributeReport() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(
        MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT
      )!!
    val attr = cluster.attributes[0x0000L]!! // MeasuredValue

    // Build raw TLV: Int16 2350 (23.50°C)
    val writer = TlvWriter()
    writer.put(AnonymousTag, 2350.toShort())
    val tlvBytes = writer.validateTlv().getEncoded()

    val decoded = UniversalTlvEncoder.decodeAttributeReport(attr, tlvBytes)
    assertEquals(2350L, decoded.rawValue)
    assertEquals("23.5°C", decoded.formattedValue)
    assertEquals("°C", decoded.unit)
  }

  @Test
  fun testDecodeHumidityAttributeReport() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(
        MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT
      )!!
    val attr = cluster.attributes[0x0000L]!!

    val writer = TlvWriter()
    writer.put(AnonymousTag, 4520.toUShort()) // 45.20%
    val tlvBytes = writer.validateTlv().getEncoded()

    val decoded = UniversalTlvEncoder.decodeAttributeReport(attr, tlvBytes)
    assertEquals(4520L, decoded.rawValue)
    assertEquals("45.2%", decoded.formattedValue)
    assertEquals("%", decoded.unit)
  }

  @Test
  fun testDecodeDoorLockStateAttributeReport() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK)!!
    val attr = cluster.attributes[0x0000L]!! // LockState (1=Locked)

    val writer = TlvWriter()
    writer.put(AnonymousTag, 1.toUByte())
    val tlvBytes = writer.validateTlv().getEncoded()

    val decoded = UniversalTlvEncoder.decodeAttributeReport(attr, tlvBytes)
    assertEquals("Locked", decoded.formattedValue)
  }

  @Test
  fun testEncodeOptionalNullableFieldOmittedVsExplicitNull() {
    val cmd =
      MatterCommandMeta(
        commandId = 0x00L,
        name = "TestOptionalNullableCmd",
        description = "Test optional+nullable field encoding",
        requestFields =
          listOf(
            MatterFieldMeta(
              tagId = 0,
              name = "mandatoryField",
              type = MatterDataType.UINT8,
              isOptional = false,
              isNullable = false,
            ),
            MatterFieldMeta(
              tagId = 1,
              name = "optionalNullableField",
              type = MatterDataType.OCTET_STRING,
              isOptional = true,
              isNullable = true,
            ),
          ),
      )

    // 1. Omitted optional+nullable field must NOT emit a TLV null tag
    val omittedBytes = UniversalTlvEncoder.encodeCommandPayload(cmd, mapOf("mandatoryField" to 42L))
    val omittedReader = TlvReader(omittedBytes)
    omittedReader.enterStructure(AnonymousTag)
    assertEquals(42.toUByte(), omittedReader.getUByte(ContextSpecificTag(0)))
    assertTrue(omittedReader.isEndOfContainer())
    omittedReader.exitContainer()

    // 2. Explicit null for optional+nullable field MUST emit a TLV null tag
    val explicitNullBytes =
      UniversalTlvEncoder.encodeCommandPayload(
        cmd,
        mapOf("mandatoryField" to 42L, "optionalNullableField" to null),
      )
    val nullReader = TlvReader(explicitNullBytes)
    nullReader.enterStructure(AnonymousTag)
    assertEquals(42.toUByte(), nullReader.getUByte(ContextSpecificTag(0)))
    nullReader.getNull(ContextSpecificTag(1))
    assertTrue(nullReader.isEndOfContainer())
    nullReader.exitContainer()
  }

  @Test
  fun testEncodeCommandAndAttributeWriteJson() {
    val cluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL)!!
    val cmd = cluster.commands[0x00L]!!
    val json =
      UniversalTlvEncoder.encodeCommandJson(cmd, mapOf("level" to 128L, "transitionTime" to 10L))
    val obj = JsonParser.parseString(json).asJsonObject
    assertEquals(128, obj.get("0:UINT").asInt)
    assertEquals(10, obj.get("1:UINT").asInt)

    val fanCluster =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_FAN_CONTROL)!!
    val fanModeAttr = fanCluster.attributes[0x0000L]!!
    val writeJson = UniversalTlvEncoder.encodeAttributeWriteJson(fanModeAttr, "High")
    val writeObj = JsonParser.parseString(writeJson).asJsonObject
    assertEquals(3, writeObj.get("0:UINT").asInt)

    val writeTlv = UniversalTlvEncoder.encodeAttributeWritePayload(fanModeAttr, "High")
    assertEquals(3.toUByte(), TlvReader(writeTlv).getUByte(AnonymousTag))
  }

  @Test
  fun testEncodeOddLengthAndUppercaseHexOctetString() {
    val attr =
      MatterAttributeMeta(
        attributeId = 0x0001L,
        name = "RawOctet",
        type = MatterDataType.OCTET_STRING,
        isWritable = true,
      )
    val tlvOdd = UniversalTlvEncoder.encodeAttributeWritePayload(attr, "0X1")
    val decodedOdd = TlvReader(tlvOdd).getByteArray(AnonymousTag)
    assertEquals(1, decodedOdd.size)
    assertEquals(0x01.toByte(), decodedOdd[0])

    val tlvThree = UniversalTlvEncoder.encodeAttributeWritePayload(attr, "ABC")
    val decodedThree = TlvReader(tlvThree).getByteArray(AnonymousTag)
    assertEquals(2, decodedThree.size)
    assertEquals(0x0A.toByte(), decodedThree[0])
    assertEquals(0xBC.toByte(), decodedThree[1])
  }
}
