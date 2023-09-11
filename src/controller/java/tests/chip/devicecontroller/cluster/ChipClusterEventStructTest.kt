/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster

import chip.devicecontroller.cluster.eventstructs.UnitTestingClusterTestEventEvent
import chip.devicecontroller.cluster.eventstructs.UnitTestingClusterTestFabricScopedEventEvent
import chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct
import chip.tlv.AnonymousTag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import org.junit.Test

class ChipClusterEventStructTest {
  @Test
  fun testEventEventTlvTest() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val simpleStruct2 =
      UnitTestingClusterSimpleStruct(8, false, 9, byteArrayOf(0x02, 0x03), "test2", 4, 5.6f, 7.8)
    val struct =
      UnitTestingClusterTestEventEvent(
        1,
        2,
        true,
        simpleStruct,
        listOf(simpleStruct, simpleStruct2),
        listOf(3, 4, 5)
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct = UnitTestingClusterTestEventEvent.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun testFabricScopedEventEventTest() {
    val struct = UnitTestingClusterTestFabricScopedEventEvent(1)

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterTestFabricScopedEventEvent.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }
}
