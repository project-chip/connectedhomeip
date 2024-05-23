/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2019-2023 Google LLC.
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

package matter.tlv

import com.google.common.truth.Truth.assertThat
import java.math.BigInteger
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

// Extracted from a Newman device during a pairing flow. Represents a fabric
// ID and keys for the fabric 7885a14c693bf1cb.
private val fabricConfig =
  """
  D50000050001002701CBF13B694CA18578360
  21525010110240201300310149BF1430B26F5
  E4BBF380D3DB855BA1300414E0E8BAA1CAC6E
  8E7216D720BD13C61C5E0E7B9012405002406
  00181818
  """
    .trimIndent()
    .replace("\n", "")
    .chunked(2)
    .map { it.toInt(16) and 0xFF }
    .map { it.toByte() }
    .toByteArray()

@RunWith(JUnit4::class)
class TlvReaderTest {
  private fun String.octetsToByteArray(): ByteArray =
    replace(" ", "").chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()

  @Test
  fun parsingFabricConfig_extractsFabricId() {
    val reader = TlvReader(fabricConfig)
    assertThat(reader.nextElement().value).isInstanceOf(StructureValue::class.java)

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(1)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value)
        .isEqualTo(BigInteger("7885a14c693bf1cb", 16).toLong())
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(2)
      assertThat(value).isInstanceOf(ArrayValue::class.java)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(AnonymousTag::class.java)
      assertThat(value).isInstanceOf(StructureValue::class.java)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(1)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value).isEqualTo(0x1001)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(2)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value).isEqualTo(0x01)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(3)
      assertThat(value).isInstanceOf(ByteStringValue::class.java)
      assertThat((value as ByteStringValue).value)
        .isEqualTo("149BF1430B26F5E4BBF380D3DB855BA1".octetsToByteArray())
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(4)
      assertThat(value).isInstanceOf(ByteStringValue::class.java)
      assertThat((value as ByteStringValue).value)
        .isEqualTo("E0E8BAA1CAC6E8E7216D720BD13C61C5E0E7B901".octetsToByteArray())
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(5)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value).isEqualTo(0)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(6)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value).isEqualTo(0)
    }

    reader.nextElement().apply { assertThat(value).isInstanceOf(EndOfContainerValue::class.java) }

    reader.nextElement().apply { assertThat(value).isInstanceOf(EndOfContainerValue::class.java) }

    reader.nextElement().apply { assertThat(value).isInstanceOf(EndOfContainerValue::class.java) }
  }

  @Test
  fun parsingContextSpecificTagWithUnsignedIntMsbSet_extractsUnsignedInt() {
    // ContextSpecificTag with UnsignedIntValue set to 0xFFF2(65522)
    val config = "2500F2FF".chunked(2).map { it.toInt(16).toByte() }.toByteArray()
    val reader = TlvReader(config)

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
      assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(0)
      assertThat(value).isInstanceOf(UnsignedIntValue::class.java)
      assertThat((value as UnsignedIntValue).value).isEqualTo(65522)
    }
  }

  @Test
  fun iterator_overAllElements_returnsExpectedNumberOfElements() {
    // Fabric configuration contains a total of 13 elements,
    // including container and end of container element types.
    assertThat(TlvReader(fabricConfig).count()).isEqualTo(13)
  }

  @Test
  fun onReset_readerStartsAtBeginning() {
    val reader = TlvReader(fabricConfig)
    val initialElements = reader.take(3)

    reader.reset()

    assertThat(reader.take(3)).isEqualTo(initialElements)
  }

  @Test
  fun onInvalidData_throwsTlvParsingException() {
    val reader = TlvReader(byteArrayOf(0x5))
    assertThrows(TlvParsingException::class.java) { reader.nextElement() }
  }

  @Test
  fun parseByteArray_extractsUtf8String() {
    // Common-profile 2-bytes Tag with UTF-8 String 1-byte type
    val control = 0b01001100.toByte()
    val length = 0b101.toByte()
    val v = "value".toByteArray(Charsets.UTF_8)

    val reader = TlvReader(byteArrayOf(control, 0x0, 0x0, length, *v))
    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(CommonProfileTag::class.java)
      assertThat((tag as CommonProfileTag).size).isEqualTo(2)
      assertThat(value).isInstanceOf(Utf8StringValue::class.java)
      assertThat((value as Utf8StringValue).value).isEqualTo("value")
    }
  }

  @Test
  fun parseByteArray_extractsByteString() {
    // Common-profile 2-bytes Tag with ByteString 1-byte type
    val control = 0b01010000.toByte()
    val length = 0b10000.toByte()
    val v = "byte_string_utf8".toByteArray()
    assertThat(v.size).isEqualTo(16)

    val reader = TlvReader(byteArrayOf(control, 0x0, 0x0, length, *v))
    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(CommonProfileTag::class.java)
      assertThat((tag as CommonProfileTag).size).isEqualTo(2)
      assertThat(value).isInstanceOf(ByteStringValue::class.java)
      assertThat((value as ByteStringValue).value).isEqualTo("byte_string_utf8".toByteArray())
    }
  }

  @Test
  fun parseByteArray_extractsArray() {
    // Common-profile 2-bytes Tag with UTF-8 String 1-byte type
    val stringControl = 0b01001100.toByte()
    val stringLength = 0b101.toByte()
    val stringValue = "value".toByteArray(Charsets.UTF_8)
    val stringBytes = byteArrayOf(stringControl, 0x0, 0x0, stringLength, *stringValue)

    // Anonymous Tag with UTF-8 String 2-byte type (length [0x110, 0x0] in little endian).
    val stringControl2 = 0b00001101.toByte()
    val stringLength2 = 0b110.toByte()
    val stringValue2 = "value2".toByteArray(Charsets.UTF_8)
    val stringBytes2 = byteArrayOf(stringControl2, stringLength2, 0x0, *stringValue2)

    // Anonymous Tag with UTF-8 String 1-byte type
    val stringControl3 = 0b00001100.toByte()
    val stringLength3 = 0b110.toByte()
    val stringValue3 = "value3".toByteArray(Charsets.UTF_8)
    val stringBytes3 = byteArrayOf(stringControl3, stringLength3, *stringValue3)

    // Put String2 and String3 into the array.
    val arrayControl = 0b01010110.toByte()
    val arrayValue = byteArrayOf(*stringBytes2, *stringBytes3)
    val arrayBytes = byteArrayOf(arrayControl, 0x0, 0x0, *arrayValue)

    // Common-profile 2-bytes Tag with UTF-8 String 1-byte type
    val stringControl4 = 0b01001100.toByte()
    val stringLength4 = 0b110.toByte()
    val stringValue4 = "value4".toByteArray(Charsets.UTF_8)
    val stringBytes4 = byteArrayOf(stringControl4, 0x0, 0x0, stringLength4, *stringValue4)

    val reader =
      TlvReader(byteArrayOf(*stringBytes, *arrayBytes, 0b00011000.toByte(), *stringBytes4))
    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(CommonProfileTag::class.java)
      assertThat((tag as CommonProfileTag).size).isEqualTo(2)
      assertThat(value).isInstanceOf(Utf8StringValue::class.java)
      assertThat((value as Utf8StringValue).value).isEqualTo("value")
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(CommonProfileTag::class.java)
      assertThat((tag as CommonProfileTag).size).isEqualTo(2)
      assertThat(value).isInstanceOf(ArrayValue::class.java)
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(AnonymousTag::class.java)
      assertThat(value).isInstanceOf(Utf8StringValue::class.java)
      assertThat((value as Utf8StringValue).value).isEqualTo("value2")
    }

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(AnonymousTag::class.java)
      assertThat(value).isInstanceOf(Utf8StringValue::class.java)
      assertThat((value as Utf8StringValue).value).isEqualTo("value3")
    }

    assertThat(reader.nextElement().value).isInstanceOf(EndOfContainerValue::class.java)

    reader.nextElement().apply {
      assertThat(tag).isInstanceOf(CommonProfileTag::class.java)
      assertThat((tag as CommonProfileTag).size).isEqualTo(2)
      assertThat(value).isInstanceOf(Utf8StringValue::class.java)
      assertThat((value as Utf8StringValue).value).isEqualTo("value4")
    }
  }

  @Test
  fun contextSpecificTag_parsesUnsignedByte() {
    // Context-Specific tag with tag number 254 (0xFE) and value 2
    val bytes = byteArrayOf(0x24, 0xFE.toByte(), 0x02)
    val tlvReader = TlvReader(bytes)
    val tag = tlvReader.nextElement().tag
    assertThat(tag).isInstanceOf(ContextSpecificTag::class.java)
    assertThat((tag as ContextSpecificTag).tagNumber).isEqualTo(0xFE)
  }
}
