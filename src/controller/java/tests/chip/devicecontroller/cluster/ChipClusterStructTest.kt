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

import chip.devicecontroller.cluster.structs.UnitTestingClusterDoubleNestedStructList
import chip.devicecontroller.cluster.structs.UnitTestingClusterNestedStructList
import chip.devicecontroller.cluster.structs.UnitTestingClusterNullablesAndOptionalsStruct
import chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct
import chip.devicecontroller.cluster.structs.UnitTestingClusterTestFabricScoped
import chip.devicecontroller.cluster.structs.UnitTestingClusterTestListStructOctet
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class ChipClusterStructTest {
  @Test
  fun doubleNestedStructTlvTest() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val simpleStruct2 =
      UnitTestingClusterSimpleStruct(8U, false, 9U, byteArrayOf(0x02, 0x03), "test2", 4U, 5.6f, 7.8)
    val nestedStructList =
      UnitTestingClusterNestedStructList(
        1U,
        true,
        simpleStruct,
        listOf(simpleStruct, simpleStruct2),
        listOf(1UL, 2UL),
        listOf(byteArrayOf(0x02, 0x03), byteArrayOf(0x03, 0x04)),
        listOf(1U, 2U)
      )
    val nestedStructList2 =
      UnitTestingClusterNestedStructList(
        2U,
        false,
        simpleStruct2,
        listOf(simpleStruct2, simpleStruct),
        listOf(3UL, 4UL),
        listOf(byteArrayOf(0x04, 0x05), byteArrayOf(0x06, 0x07)),
        listOf(3U, 4U, 5U)
      )
    val testStruct =
      UnitTestingClusterDoubleNestedStructList(listOf(nestedStructList, nestedStructList2))

    val tlvWriter = TlvWriter()
    testStruct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareTestStruct =
      UnitTestingClusterDoubleNestedStructList.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareTestStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun nullablesAndOptionalsStructTlvTest1() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1U,
        Optional.of(2U),
        Optional.of(3U),
        "test4",
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1U, 2U, 3U),
        Optional.of(listOf(4U, 5U, 6U)),
        Optional.of(listOf(7U, 8U, 9U))
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  // Optional Check - 1
  fun nullablesAndOptionalsStructTlvTest2() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1U,
        Optional.empty(),
        Optional.empty(),
        "test4",
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.empty(),
        Optional.empty(),
        listOf(1U, 2U, 3U),
        Optional.of(listOf(4U, 5U, 6U)),
        Optional.of(listOf(7U, 8U, 9U))
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  // Optional Check - 2
  fun nullablesAndOptionalsStructTlvTest3() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1U,
        Optional.of(2U),
        Optional.of(3U),
        "test4",
        Optional.empty(),
        Optional.empty(),
        simpleStruct,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1U, 2U, 3U),
        Optional.empty(),
        Optional.empty()
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  // Nullable check - 1
  fun nullablesAndOptionalsStructTlvTest4() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1U,
        Optional.empty(),
        null,
        null,
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.empty(),
        null,
        null,
        Optional.of(listOf(4U, 5U, 6U)),
        Optional.of(listOf(7U, 8U, 9U))
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  // Nullable check - 2
  fun nullablesAndOptionalsStructTlvTest5() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        null,
        Optional.of(2U),
        Optional.of(3U),
        "test4",
        Optional.empty(),
        null,
        null,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1U, 2U, 3U),
        Optional.empty(),
        null
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct =
      UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun testFabricScopedTlvTest1() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1U,
        Optional.of(2U),
        3U,
        Optional.of(4U),
        "test1",
        simpleStruct,
        listOf(1U, 2U, 3U),
        4U
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct = UnitTestingClusterTestFabricScoped.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun testFabricScopedTlvTest2() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1U,
        Optional.empty(),
        3U,
        null,
        "test1",
        simpleStruct,
        listOf(1U, 2U, 3U),
        4U
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct = UnitTestingClusterTestFabricScoped.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun testFabricScopedTlvTest3() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1U, true, 2U, byteArrayOf(0x00, 0x01), "test", 3U, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1U,
        Optional.of(2U),
        null,
        Optional.empty(),
        "test1",
        simpleStruct,
        listOf(1U, 2U, 3U),
        4U
      )

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct = UnitTestingClusterTestFabricScoped.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }

  @Test
  fun testListStructOctetTlvTest() {
    val struct = UnitTestingClusterTestListStructOctet(1UL, byteArrayOf(0x01, 0x02, 0x03))

    val tlvWriter = TlvWriter()
    struct.toTlv(AnonymousTag, tlvWriter)

    // to TLV ByteArray
    val tlv = tlvWriter.getEncoded()

    // Reparse to Struct
    val tlvReader = TlvReader(tlv)
    val compareStruct = UnitTestingClusterTestListStructOctet.fromTlv(AnonymousTag, tlvReader)

    // For comparing, struct to re-generate to tlv bytearray
    val compareTlvWriter = TlvWriter()
    compareStruct.toTlv(AnonymousTag, compareTlvWriter)

    assert(compareTlvWriter.getEncoded().contentEquals(tlvWriter.getEncoded()))
  }
}
