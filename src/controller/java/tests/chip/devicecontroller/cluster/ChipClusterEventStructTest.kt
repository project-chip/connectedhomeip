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
package chip.devicecontroller.cluster

import chip.devicecontroller.cluster.eventstructs.UnitTestingClusterTestEventEvent
import chip.devicecontroller.cluster.eventstructs.UnitTestingClusterTestFabricScopedEventEvent
import chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter
import org.junit.Test

class ChipClusterEventStructTest {
  @Test
  fun testEventEventTlvTest() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(
        1U,
        true,
        2U,
        byteArrayOf(0x00, 0x01),
        "test",
        3U,
        4.5f,
        6.7,
        Optional.empty()
      )
    val simpleStruct2 =
      UnitTestingClusterSimpleStruct(
        8U,
        false,
        9U,
        byteArrayOf(0x02, 0x03),
        "test2",
        4U,
        5.6f,
        7.8,
        Optional.empty()
      )
    val struct =
      UnitTestingClusterTestEventEvent(
        1U,
        2U,
        true,
        simpleStruct,
        listOf(simpleStruct, simpleStruct2),
        listOf(3U, 4U, 5U)
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
    val struct = UnitTestingClusterTestFabricScopedEventEvent(1U)

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
