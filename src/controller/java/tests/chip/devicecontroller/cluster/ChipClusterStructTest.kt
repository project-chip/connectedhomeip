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
import chip.tlv.AnonymousTag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class ChipClusterStructTest {
  @Test
  fun doubleNestedStructTlvTest() {
    val simpleStruct =
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val simpleStruct2 =
      UnitTestingClusterSimpleStruct(8, false, 9, byteArrayOf(0x02, 0x03), "test2", 4, 5.6f, 7.8)
    val nestedStructList =
      UnitTestingClusterNestedStructList(
        1,
        true,
        simpleStruct,
        listOf(simpleStruct, simpleStruct2),
        listOf(1L, 2L),
        listOf(byteArrayOf(0x02, 0x03), byteArrayOf(0x03, 0x04)),
        listOf(1, 2)
      )
    val nestedStructList2 =
      UnitTestingClusterNestedStructList(
        2,
        false,
        simpleStruct2,
        listOf(simpleStruct2, simpleStruct),
        listOf(3L, 4L),
        listOf(byteArrayOf(0x04, 0x05), byteArrayOf(0x06, 0x07)),
        listOf(3, 4, 5)
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1,
        Optional.of(2),
        Optional.of(3),
        "test4",
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1, 2, 3),
        Optional.of(listOf(4, 5, 6)),
        Optional.of(listOf(7, 8, 9))
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1,
        Optional.empty(),
        Optional.empty(),
        "test4",
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.empty(),
        Optional.empty(),
        listOf(1, 2, 3),
        Optional.of(listOf(4, 5, 6)),
        Optional.of(listOf(7, 8, 9))
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1,
        Optional.of(2),
        Optional.of(3),
        "test4",
        Optional.empty(),
        Optional.empty(),
        simpleStruct,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1, 2, 3),
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        1,
        Optional.empty(),
        null,
        null,
        Optional.of("test5"),
        Optional.of("test6"),
        simpleStruct,
        Optional.empty(),
        null,
        null,
        Optional.of(listOf(4, 5, 6)),
        Optional.of(listOf(7, 8, 9))
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterNullablesAndOptionalsStruct(
        null,
        Optional.of(2),
        Optional.of(3),
        "test4",
        Optional.empty(),
        null,
        null,
        Optional.of(simpleStruct),
        Optional.of(simpleStruct),
        listOf(1, 2, 3),
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1,
        Optional.of(2),
        3,
        Optional.of(4),
        "test1",
        simpleStruct,
        listOf(1, 2, 3),
        4
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1,
        Optional.empty(),
        3,
        null,
        "test1",
        simpleStruct,
        listOf(1, 2, 3),
        4
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
      UnitTestingClusterSimpleStruct(1, true, 2, byteArrayOf(0x00, 0x01), "test", 3, 4.5f, 6.7)
    val struct =
      UnitTestingClusterTestFabricScoped(
        1,
        Optional.of(2),
        null,
        Optional.empty(),
        "test1",
        simpleStruct,
        listOf(1, 2, 3),
        4
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
    val struct = UnitTestingClusterTestListStructOctet(1L, byteArrayOf(0x01, 0x02, 0x03))

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
