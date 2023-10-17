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
import kotlin.test.assertFailsWith
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

// TLV Encoded structure taken from the C++ TLV unit test of the Matter SDK
private val testTlvSampleData: ByteArray =
  ("""
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00, 0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00, 0x88, 0x02,
    0x00, 0x36, 0x00, 0x00, 0x2A, 0x00, 0xEF, 0x02, 0xF0, 0x67, 0xFD, 0xFF, 0x07, 0x00, 0x90, 0x2F,
    0x50, 0x09, 0x00, 0x00, 0x00, 0x15, 0x18, 0x17, 0xD4, 0xBB, 0xAA, 0xDD, 0xCC, 0x11, 0x00, 0xB4,
    0xA0, 0xBB, 0x0D, 0x00, 0x14, 0xB5, 0x00, 0x28, 0x6B, 0xEE, 0x6D, 0x70, 0x11, 0x01, 0x00, 0x0E,
    0x01, 0x53, 0x54, 0x41, 0x52, 0x54, 0x2E, 0x2E, 0x2E, 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x40, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x23, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x24, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x25, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x5E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x26, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x2A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x28, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x29, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x2D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x3D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x5B, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x5D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x3B, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x27, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x2E, 0x2E, 0x2E, 0x45, 0x4E, 0x44, 0x18,
    0x18, 0x18, 0xCC, 0xBB, 0xAA, 0xDD, 0xCC, 0x05, 0x00, 0x0E, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69,
    0x73, 0x20, 0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x8A, 0xFF, 0xFF, 0x33, 0x33, 0x8F, 0x41, 0xAB,
    0x00, 0x00, 0x01, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x31, 0x40, 0x18
""")
    .trimIndent()
    .replace("0x", "")
    .replace(", ", "")
    .replace(",", "")
    .replace("\n", "")
    .chunked(2)
    .map { it.toInt(16) and 0xFF }
    .map { it.toByte() }
    .toByteArray()

private const val TEST_VENDOR_ID: UShort = 0xAABBu
private const val TEST_PRODUCT_ID: UShort = 0xCCDDu

private val testLargeString: String =
  """
  START...
  !123456789ABCDEF@123456789ABCDEF#123456789ABCDEF$123456789ABCDEF
  %123456789ABCDEF^123456789ABCDEF&123456789ABCDEF*123456789ABCDEF
  01234567(9ABCDEF01234567)9ABCDEF01234567-9ABCDEF01234567=9ABCDEF
  01234567[9ABCDEF01234567]9ABCDEF01234567;9ABCDEF01234567'9ABCDEF
  ...END
  """
    .trimIndent()
    .replace("\n", "")

@RunWith(JUnit4::class)
class TlvReadWriteTest {
  private fun String.octetsToByteArray(): ByteArray =
    replace(" ", "").chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()

  @Test
  fun testTlvSampleData_write() {
    TlvWriter().apply {
      startStructure(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 1u))
      put(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 2u), true)
      put(ImplicitProfileTag(2, 2u), false)
      startArray(ContextSpecificTag(0))
      put(AnonymousTag, 42)
      put(AnonymousTag, -17)
      put(AnonymousTag, -170000)
      put(AnonymousTag, 40000000000UL)
      startStructure(AnonymousTag)
      endStructure()
      startList(AnonymousTag)
      putNull(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 17u))
      putNull(ImplicitProfileTag(4, 900000u))
      putNull(AnonymousTag)
      startStructure(ImplicitProfileTag(4, 4000000000u))
      put(CommonProfileTag(4, 70000u), testLargeString)
      endStructure()
      endList()
      endArray()
      put(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 5u), "This is a test")
      put(ImplicitProfileTag(2, 65535u), 17.9f)
      put(ImplicitProfileTag(4, 65536u), 17.9)
      endStructure()
      validateTlv()
      assertThat(getEncoded()).isEqualTo(testTlvSampleData)
    }
  }

  @Test
  fun testTlvSampleData_read() {
    TlvReader(testTlvSampleData).apply {
      enterStructure(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 1u))
      assertThat(getBool(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 2u))).isEqualTo(true)
      assertThat(getBool(ImplicitProfileTag(2, 2u))).isEqualTo(false)
      enterArray(ContextSpecificTag(0))
      assertThat(getInt(AnonymousTag)).isEqualTo(42)
      assertThat(getInt(AnonymousTag)).isEqualTo(-17)
      assertThat(getInt(AnonymousTag)).isEqualTo(-170000)
      assertThat(getULong(AnonymousTag)).isEqualTo(40000000000UL)
      enterStructure(AnonymousTag)
      exitContainer()
      enterList(AnonymousTag)
      getNull(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 17u))
      getNull(ImplicitProfileTag(4, 900000u))
      getNull(AnonymousTag)
      enterStructure(ImplicitProfileTag(4, 4000000000u))
      assertThat(getUtf8String(CommonProfileTag(4, 70000u))).isEqualTo(testLargeString)
      exitContainer()
      exitContainer()
      exitContainer()
      assertThat(getUtf8String(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 5u)))
        .isEqualTo("This is a test")
      assertThat(getFloat(ImplicitProfileTag(2, 65535u))).isEqualTo(17.9f)
      assertThat(getDouble(ImplicitProfileTag(4, 65536u))).isEqualTo(17.9)
      exitContainer()
      assertThat(getLengthRead()).isEqualTo(testTlvSampleData.size)
      assertThat(isEndOfTlv()).isEqualTo(true)
    }
  }

  @Test
  fun testTlvSampleData_read_useSkipElementAndExitContinerInTheMiddle() {
    TlvReader(testTlvSampleData).apply {
      enterStructure(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 1u))
      skipElement()
      assertThat(getBool(ImplicitProfileTag(2, 2u))).isEqualTo(false)
      enterArray(ContextSpecificTag(0))
      assertThat(getInt(AnonymousTag)).isEqualTo(42)
      exitContainer()
      assertThat(getUtf8String(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 5u)))
        .isEqualTo("This is a test")
      assertThat(getFloat(ImplicitProfileTag(2, 65535u))).isEqualTo(17.9f)
      assertThat(getDouble(ImplicitProfileTag(4, 65536u))).isEqualTo(17.9)
      exitContainer()
      assertThat(getLengthRead()).isEqualTo(testTlvSampleData.size)
      assertThat(isEndOfTlv()).isEqualTo(true)
    }
  }

  @Test
  fun testTlvSampleData_copyElement() {
    val reader = TlvReader(testTlvSampleData)
    val encoding = TlvWriter().copyElement(reader).validateTlv().getEncoded()
    assertThat(encoding).isEqualTo(testTlvSampleData)
  }

  @Test
  fun testTlvSampleData_copyElementWithTag() {
    val reader = TlvReader(testTlvSampleData)
    val encoding =
      TlvWriter()
        .copyElement(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 1u), reader)
        .validateTlv()
        .getEncoded()
    assertThat(encoding).isEqualTo(testTlvSampleData)
  }

  @Test
  fun testCopyElement_throwsIllegalArgumentException() {
    val encoding =
      TlvWriter().startStructure(AnonymousTag).endStructure().validateTlv().getEncoded()
    val reader = TlvReader(encoding)
    reader.skipElement()

    // Throws exception because the reader is positioned at the end of container element
    assertFailsWith<IllegalArgumentException> { TlvWriter().copyElement(reader) }
  }

  @Test
  fun testCopyElement_replaceTag() {
    val tag = CommonProfileTag(2, 1000u)
    val encoding =
      TlvWriter().startStructure(AnonymousTag).endStructure().validateTlv().getEncoded()
    val expectedEncoding = TlvWriter().startStructure(tag).endStructure().validateTlv().getEncoded()

    assertThat(TlvWriter().copyElement(tag, TlvReader(encoding)).validateTlv().getEncoded())
      .isEqualTo(expectedEncoding)
  }

  @Test
  fun testCopyElementUInt_replaceTag() {
    val value = 42U
    val tag1 = CommonProfileTag(2, 1u)
    val tag2 = CommonProfileTag(2, 2u)
    val encoding = TlvWriter().put(tag1, value).validateTlv().getEncoded()
    val expectedEncoding = TlvWriter().put(tag2, value).validateTlv().getEncoded()

    assertThat(TlvWriter().copyElement(tag2, TlvReader(encoding)).validateTlv().getEncoded())
      .isEqualTo(expectedEncoding)
  }

  @Test
  fun testTlvSampleData_copyElementsOneByOne() {
    val reader = TlvReader(testTlvSampleData)
    reader.skipElement()
    val encoding =
      TlvWriter()
        .startStructure(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 1u))
        .copyElement(reader)
        .copyElement(reader)
        .copyElement(reader)
        .copyElement(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 5u), reader)
        .copyElement(reader)
        .copyElement(reader)
        .endStructure()
        .validateTlv()
        .getEncoded()
    assertThat(encoding).isEqualTo(testTlvSampleData)
  }

  @Test
  fun testData_IntMinMax() {
    val encodedTlv =
      TlvWriter()
        .apply {
          put(AnonymousTag, Byte.MIN_VALUE.toByte())
          put(AnonymousTag, Byte.MAX_VALUE.toByte())
          put(AnonymousTag, Short.MIN_VALUE.toShort())
          put(AnonymousTag, Short.MAX_VALUE.toShort())
          put(AnonymousTag, Int.MIN_VALUE.toInt())
          put(AnonymousTag, Int.MAX_VALUE.toInt())
          put(AnonymousTag, Long.MIN_VALUE.toLong())
          put(AnonymousTag, Long.MAX_VALUE.toLong())
          put(AnonymousTag, UByte.MAX_VALUE.toUByte())
          put(AnonymousTag, UShort.MAX_VALUE.toUShort())
          put(AnonymousTag, UInt.MAX_VALUE.toUInt())
          put(AnonymousTag, ULong.MAX_VALUE.toULong())
        }
        .validateTlv()
        .getEncoded()

    TlvReader(encodedTlv).apply {
      assertThat(getByte(AnonymousTag)).isEqualTo(Byte.MIN_VALUE)
      assertThat(getByte(AnonymousTag)).isEqualTo(Byte.MAX_VALUE)
      assertThat(getShort(AnonymousTag)).isEqualTo(Short.MIN_VALUE)
      assertThat(getShort(AnonymousTag)).isEqualTo(Short.MAX_VALUE)
      assertThat(getInt(AnonymousTag)).isEqualTo(Int.MIN_VALUE)
      assertThat(getInt(AnonymousTag)).isEqualTo(Int.MAX_VALUE)
      assertThat(getLong(AnonymousTag)).isEqualTo(Long.MIN_VALUE)
      assertThat(getLong(AnonymousTag)).isEqualTo(Long.MAX_VALUE)
      assertThat(getUByte(AnonymousTag)).isEqualTo(UByte.MAX_VALUE)
      assertThat(getUShort(AnonymousTag)).isEqualTo(UShort.MAX_VALUE)
      assertThat(getUInt(AnonymousTag)).isEqualTo(UInt.MAX_VALUE)
      assertThat(getULong(AnonymousTag)).isEqualTo(ULong.MAX_VALUE)
    }
  }

  @Test
  fun encodeBoolean_false() {
    // Boolean false
    val value = false
    val encoding = "08".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      validateTlv()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getBool(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeBoolean_true() {
    // Boolean true
    val value = true
    val encoding = "09".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getBool(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeSignedInt_1BytePositive() {
    // Signed Integer, 1-octet
    val value = 42
    val encoding = "00 2a".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeSignedInt_1ByteNegative() {
    // Signed Integer, 1-octet
    val value = -17
    val encoding = "00 ef".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeUnsignedInt_1Byte() {
    // Unsigned Integer, 1-octet
    val value = 42u
    val encoding = "04 2a".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeUnsignedInt_1Byte_usePutUnsigned() {
    // Unsigned Integer, 1-octet
    val value = 42
    val encoding = "04 2a".octetsToByteArray()

    TlvWriter().apply {
      putUnsigned(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }
  }

  @Test
  fun encodeSignedInt_1Byte2octet() {
    // Signed Integer, 1-byte encoded as 2-octet
    val value = 42
    val encoding = "01 2a 00".octetsToByteArray()

    // Note: the current implementation follows the minimum encoding policy, which encodes this
    // value as 1-octet. Testing only decoding.

    TlvReader(encoding).apply { assertThat(getInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeSignedInt_2Bytes() {
    // Signed Integer, 2-octet
    val value = 4242
    val encoding = "01 92 10".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeSignedInt_4Bytes() {
    // Signed Integer, 4-octet
    val value = -170000
    val encoding = "02 f0 67 fd ff".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getInt(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeSignedInt_8Bytes() {
    // Signed Integer (Long), 8-octet
    val value = 40000000000
    val encoding = "03 00 90 2f 50 09 00 00 00".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getLong(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeUnsignedInt_8Bytes_usePutUnsigned() {
    // Unsigned Integer (Long), 8-octet
    val value = 40000000000
    val encoding = "07 00 90 2f 50 09 00 00 00".octetsToByteArray()

    TlvWriter().apply {
      putUnsigned(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getULong(AnonymousTag)).isEqualTo(value.toULong()) }
  }

  @Test
  fun encodeSignedInt_8Bytes_useGetULong_throwsIllegalArgumentException() {
    // Signed Integer (Long), 8-octet
    val encoding = "03 00 90 2f 50 09 00 00 00".octetsToByteArray()

    // Throws exception because the encoded value is Long and not ULong as requested by
    // getULong()
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).getULong(AnonymousTag) }
  }

  @Test
  fun encodeSignedInt_8Bytes_useGetInt_throwsTlvParsingException() {
    // Signed Integer (Long), 8-octet
    val encoding = "03 00 90 2f 50 09 00 00 00".octetsToByteArray()

    // Throws exception because the encoded value is out of range of Signed Int
    assertFailsWith<TlvParsingException> { TlvReader(encoding).getInt(AnonymousTag) }
  }

  @Test
  fun encodeSignedInt_8Bytes_getFullyQualifiedTag_throwsIllegalArgumentException() {
    // Signed Integer (Long), 8-octet
    val encoding = "03 00 90 2f 50 09 00 00 00".octetsToByteArray()

    // Throws exception because the encoded value has AnonymousTag tag
    assertFailsWith<IllegalArgumentException> {
      TlvReader(encoding).getLong(FullyQualifiedTag(6, TEST_VENDOR_ID, TEST_PRODUCT_ID, 5u))
    }
  }

  @Test
  fun encodeUtf8String_hello() {
    // UTF-8 String, 1-octet length, "Hello!"
    val value = "Hello!"
    val encoding = "0c 06 48 65 6c 6c 6f 21".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUtf8String(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeUtf8String_tschuh() {
    // UTF-8 String, 1-octet length, "Tschüs"
    val value = "Tschüs"
    val encoding = "0c 07 54 73 63 68 c3 bc 73".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUtf8String(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeOctetString() {
    // Octet String, 1-octet length, octets 00 01 02 03 04
    val value = "00 01 02 03 04".octetsToByteArray()
    val encoding = "10 05 00 01 02 03 04".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getByteString(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeNull() {
    // Null
    val encoding = "14".octetsToByteArray()

    TlvWriter().apply {
      putNull(AnonymousTag)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { getNull(AnonymousTag) }
  }

  @Test
  fun encodeFloat_0() {
    // Single precision floating point 0.0
    val value = 0.0f
    val encoding = "0a 00 00 00 00".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getFloat(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeFloat_1third() {
    // Single precision floating point (1.0 / 3.0)
    val value = 1 / 3.toFloat()
    val encoding = "0a ab aa aa 3e".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getFloat(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeFloat_17_9() {
    // Single precision floating point 17.9
    val value = 17.9f
    val encoding = "0a 33 33 8f 41".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getFloat(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeFloat_positiveInfinity() {
    // Single precision floating point infinity (∞)
    val value = Float.POSITIVE_INFINITY
    val encoding = "0a 00 00 80 7f".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getFloat(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeFloat_negativeInfinity() {
    // Single precision floating point negative infinity (-∞)
    val value = Float.NEGATIVE_INFINITY
    val encoding = "0a 00 00 80 ff".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }
  }

  @Test
  fun encodeDouble_0() {
    // Double precision floating point 0.0
    val value = 0.0
    val encoding = "0b 00 00 00 00 00 00 00 00".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getDouble(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeDouble_1third() {
    // Double precision floating point (1.0 / 3.0)
    val value = 1 / 3.toDouble()
    val encoding = "0b 55 55 55 55 55 55 d5 3f".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getDouble(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeDouble_17_9() {
    // Double precision floating point 17.9
    val value = 17.9
    val encoding = "0b 66 66 66 66 66 e6 31 40".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getDouble(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeDouble_positiveInfinity() {
    // Double precision floating point infinity (∞)
    val value = Double.POSITIVE_INFINITY
    val encoding = "0b 00 00 00 00 00 00 f0 7f".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getDouble(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeDouble_negativeInfinity() {
    // Double precision floating point negative infinity (-∞)
    val value = Double.NEGATIVE_INFINITY
    val encoding = "0b 00 00 00 00 00 00 f0 ff".octetsToByteArray()

    TlvWriter().apply {
      put(AnonymousTag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getDouble(AnonymousTag)).isEqualTo(value) }
  }

  @Test
  fun encodeStructure_empty() {
    // Empty Structure, {}
    val encoding = "15 18".octetsToByteArray()

    TlvWriter().apply {
      startStructure(AnonymousTag)
      endStructure()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterStructure(AnonymousTag)
      exitContainer()
    }
  }

  @Test
  fun encodeStructure_empty_testEndOfContainer() {
    // Empty Structure, {}
    val encoding = "15 18".octetsToByteArray()

    TlvReader(encoding).apply {
      assertThat(isEndOfContainer()).isEqualTo(false)
      enterStructure(AnonymousTag)
      assertThat(isEndOfContainer()).isEqualTo(true)
      exitContainer()
      assertFailsWith<TlvParsingException> { isEndOfContainer() }
    }
  }

  @Test
  fun encodeStructure_notClosed_throwsTlvEncodingException() {
    // Open Structure, {
    TlvWriter().apply {
      startStructure(AnonymousTag)
      assertFailsWith<TlvEncodingException> { validateTlv() }
    }
  }

  @Test
  fun encodeArray_empty() {
    // Empty Array, []
    val encoding = "16 18".octetsToByteArray()

    TlvWriter().apply {
      startArray(AnonymousTag)
      endArray()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      exitContainer()
    }
  }

  @Test
  fun encodeArray_empty_closeUnopennedArray_throwsIllegalArgumentException() {
    // Empty Array, []]
    TlvWriter().apply {
      startArray(AnonymousTag)
      endArray()
      // trying to closed container that is not openned
      assertFailsWith<IllegalArgumentException> { endArray() }
    }
  }

  @Test
  fun encodeList_empty() {
    // Empty List, []
    val encoding = "17 18".octetsToByteArray()

    TlvWriter().apply {
      startList(AnonymousTag)
      endList()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterList(AnonymousTag)
      exitContainer()
    }
  }

  @Test
  fun encodeStructure_intsWithContextTags() {
    // Structure, two context specific tags, Signed Integer, 1 octet values, {0 = 42, 1 = -17}
    val value0 = 42
    val value1 = -17
    val encoding = "15 20 00 2a 20 01 ef 18".octetsToByteArray()

    TlvWriter().apply {
      startStructure(AnonymousTag)
      put(ContextSpecificTag(0), value0)
      put(ContextSpecificTag(1), value1)
      endStructure()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterStructure(AnonymousTag)
      assertThat(getByte(ContextSpecificTag(0))).isEqualTo(value0)
      assertThat(getByte(ContextSpecificTag(1))).isEqualTo(value1)
      exitContainer()
    }
  }

  @Test
  fun encodeArray_ints() {
    // Array, Signed Integer, 1-octet values, [0, 1, 2, 3, 4]
    val encoding = "16 00 00 00 01 00 02 00 03 00 04 18".octetsToByteArray()

    TlvWriter().apply {
      startArray(AnonymousTag)
      put(AnonymousTag, 0)
      put(AnonymousTag, 1)
      put(AnonymousTag, 2)
      put(AnonymousTag, 3)
      put(AnonymousTag, 4)
      endArray()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      assertThat(getInt(AnonymousTag)).isEqualTo(0)
      assertThat(getInt(AnonymousTag)).isEqualTo(1)
      assertThat(getInt(AnonymousTag)).isEqualTo(2)
      assertThat(getInt(AnonymousTag)).isEqualTo(3)
      assertThat(getInt(AnonymousTag)).isEqualTo(4)
      exitContainer()
    }
  }

  @Test
  fun encodeList_mixedInts() {
    // List, mix of anonymous and context tags, Signed Integer, 1 octet values,
    // [[1, 0 = 42, 2, 3, 0 = -17]]
    val encoding = "17 00 01 20 00 2a 00 02 00 03 20 00 ef 18".octetsToByteArray()

    TlvWriter().apply {
      startList(AnonymousTag)
      put(AnonymousTag, 1)
      put(ContextSpecificTag(0), 42)
      put(AnonymousTag, 2)
      put(AnonymousTag, 3)
      put(ContextSpecificTag(0), -17)
      endList()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterList(AnonymousTag)
      assertThat(getInt(AnonymousTag)).isEqualTo(1)
      assertThat(getInt(ContextSpecificTag(0))).isEqualTo(42)
      assertThat(getInt(AnonymousTag)).isEqualTo(2)
      assertThat(getInt(AnonymousTag)).isEqualTo(3)
      assertThat(getInt(ContextSpecificTag(0))).isEqualTo(-17)
      exitContainer()
    }
  }

  @Test
  fun encodeArray_mixedValues() {
    // Array, mix of element types, [42, -170000, {}, 17.9, "Hello!"]
    val encoding =
      "16 00 2a 02 f0 67 fd ff 15 18 0a 33 33 8f 41 0c 06 48 65 6c 6c 6f 21 18".octetsToByteArray()

    TlvWriter().apply {
      startArray(AnonymousTag)
      put(AnonymousTag, 42)
      put(AnonymousTag, -170000)
      startStructure(AnonymousTag)
      endStructure()
      put(AnonymousTag, 17.9f)
      put(AnonymousTag, "Hello!")
      endArray()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      assertThat(getInt(AnonymousTag)).isEqualTo(42)
      assertThat(getInt(AnonymousTag)).isEqualTo(-170000)
      enterStructure(AnonymousTag)
      exitContainer()
      assertThat(getFloat(AnonymousTag)).isEqualTo(17.9f)
      assertThat(getUtf8String(AnonymousTag)).isEqualTo("Hello!")
      exitContainer()
    }
  }

  @Test
  fun encodeAanonymousTag() {
    // Anonymous tag, Unsigned Integer, 1-octet value, 42U
    val value = 42U
    var tag = AnonymousTag
    var encoding = "04 2a".octetsToByteArray()

    TlvWriter().apply {
      put(tag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(tag)).isEqualTo(value) }
  }

  @Test
  fun encodeContextTag_withinStructure() {
    // Context tag 1, Unsigned Integer, 1-octet value, {1 = 42U}
    val value = 42U
    var tag = ContextSpecificTag(1)
    var encoding = "15 24 01 2a 18".octetsToByteArray()

    TlvWriter().apply {
      startStructure(AnonymousTag)
      put(tag, value)
      endStructure()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterStructure(AnonymousTag)
      assertThat(getUInt(tag)).isEqualTo(value)
      exitContainer()
    }
  }

  @Test
  fun encodeContextTag_invalidContextTag_throwsIllegalArgumentException() {
    // Context tag 1, Unsigned Integer, 1-octet value, {1 = 42U}
    val value1 = 42U
    val value2 = 17000
    var tag1 = ContextSpecificTag(UByte.MAX_VALUE.toInt())
    var tag2 = ContextSpecificTag(UByte.MAX_VALUE.toInt() + 1)

    TlvWriter().apply {
      startStructure(AnonymousTag)
      put(tag1, value1)
      // tag2 exeeds valid context specific tag value
      assertFailsWith<IllegalArgumentException> { put(tag2, value2) }
    }
  }

  @Test
  fun encodeAnonymousTagInStructure_throwsIllegalArgumentException() {
    // Anonymous tag 1, Unsigned Integer, 1-octet value, {1 = 42U}
    TlvWriter().apply {
      startStructure(AnonymousTag)
      // anonymous tags are not allowed within structure elements
      assertFailsWith<IllegalArgumentException> { put(AnonymousTag, 42U) }
    }
  }

  @Test
  fun encodeContextTag_withinList() {
    // Context tag 1, Unsigned Integer, 1-octet value, [[1 = 42U]]
    val value = 42U
    var tag = ContextSpecificTag(1)
    var encoding = "17 24 01 2a 18".octetsToByteArray()

    TlvWriter().apply {
      startList(AnonymousTag)
      put(tag, value)
      endList()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterList(AnonymousTag)
      assertThat(getUInt(tag)).isEqualTo(value)
      exitContainer()
    }
  }

  @Test
  fun encodeContextTag_withinArray_throwsIllegalArgumentException() {
    // Context tag 1, Unsigned Integer, 1-octet value, [1 = 42U]
    val value = 42U
    var tag = ContextSpecificTag(1)

    // Array elements SHALL be of anonymous type
    TlvWriter().apply {
      startArray(AnonymousTag)
      assertFailsWith<IllegalArgumentException> { put(tag, value) }
    }
  }

  @Test
  fun encodeContextTag_notInContainer_throwsIllegalArgumentException() {
    // Context tag 1, Unsigned Integer, 1-octet value, 1 = 42U
    val value = 42U
    var tag = ContextSpecificTag(1)

    // Context tag can only be used within a Structure or a List
    assertFailsWith<IllegalArgumentException> { TlvWriter().put(tag, value) }
  }

  @Test
  fun encodeCommonProfileTag2() {
    // Common profile tag 1, Unsigned Integer, 1-octet value, Matter::1 = 42U
    val value = 42U
    var tag = CommonProfileTag(2, 1u)
    var encoding = "44 01 00 2a".octetsToByteArray()

    TlvWriter().apply {
      put(tag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(tag)).isEqualTo(value) }
  }

  @Test
  fun encodeCommonProfileTag4() {
    // Common profile tag 100000, Unsigned Integer, 1-octet value, Matter::100000 = 42U
    val value = 42U
    var tag = CommonProfileTag(4, 100000u)
    var encoding = "64 a0 86 01 00 2a".octetsToByteArray()

    TlvWriter().apply {
      put(tag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(tag)).isEqualTo(value) }
  }

  @Test
  fun encodeFullyQualifiedTag6() {
    // Fully qualified tag, Vendor ID 0xFFF1/65521, profile number 0xDEED/57069, 2-octet tag 1,
    // Unsigned Integer, 1-octet value 42, 65521::57069:1 = 42U
    val value = 42U
    var tag = FullyQualifiedTag(6, 0xFFF1u, 0xDEEDu, 1u)
    var encoding = "c4 f1 ff ed de 01 00 2a".octetsToByteArray()

    TlvWriter().apply {
      put(tag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(tag)).isEqualTo(value) }
  }

  @Test
  fun encodeFullyQualifiedTag8() {
    // Fully qualified tag, Vendor ID 0xFFF1/65521, profile number 0xDEED/57069, 4-octet tag
    // 0xAA55FEED/2857762541, Unsigned Integer, 1-octet value 42, 65521::57069:2857762541 = 42U
    val value = 42U
    var tag = FullyQualifiedTag(8, 0xFFF1u, 0xDEEDu, 0xAA55FEEDu)
    var encoding = "e4 f1 ff ed de ed fe 55 aa 2a".octetsToByteArray()

    TlvWriter().apply {
      put(tag, value)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply { assertThat(getUInt(tag)).isEqualTo(value) }
  }

  @Test
  fun encodeFullyQualifiedTags_withStructure() {
    // Structure with the fully qualified tag, Vendor ID 0xFFF1/65521, profile number
    // 0xDEED/57069,
    // 2-octet tag 1. The structure contains a single element labeled using a fully qualified
    // tag
    // under the same profile, with 2-octet tag 0xAA55/43605.65521::57069:1 =
    // {65521::57069:43605 =
    // 42U}
    val value = 42U
    val structTag = FullyQualifiedTag(6, 0xFFF1u, 0xDEEDu, 1u)
    var valueTag = FullyQualifiedTag(6, 0xFFF1u, 57069u, 43605u)
    var encoding = "d5 f1 ff ed de 01 00 c4 f1 ff ed de 55 aa 2a 18".octetsToByteArray()

    TlvWriter().apply {
      startStructure(structTag)
      put(valueTag, value)
      endStructure()
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterStructure(structTag)
      assertThat(getUInt(valueTag)).isEqualTo(value)
      exitContainer()
    }
  }

  @Test
  fun encodeArray_empty_useputSignedLongArray() {
    // Empty Array, []
    val values = longArrayOf()
    val encoding = "16 18".octetsToByteArray()

    TlvWriter().apply {
      putSignedLongArray(AnonymousTag, values)
      assertThat(getEncoded()).isEqualTo(encoding)
    }
  }

  @Test
  fun putSignedLongArray() {
    // Anonymous Array of Signed Integers, [42, -17, -170000, 40000000000]
    val values = longArrayOf(42, -17, -170000, 40000000000)
    val encoding = "16 00 2a 00 ef 02 f0 67 fd ff 03 00 90 2f 50 09 00 00 00 18".octetsToByteArray()

    TlvWriter().apply {
      putSignedLongArray(AnonymousTag, values)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      assertThat(getLong(AnonymousTag)).isEqualTo(values[0])
      assertThat(getLong(AnonymousTag)).isEqualTo(values[1])
      assertThat(getLong(AnonymousTag)).isEqualTo(values[2])
      assertThat(getLong(AnonymousTag)).isEqualTo(values[3])
      exitContainer()
    }
  }

  @Test
  fun putUnsignedLongArray() {
    // Anonymous Array of Signed Integers, [42, 170000, 40000000000]
    val values = longArrayOf(42, 170000, 40000000000)
    val encoding = "16 04 2a 06 10 98 02 00 07 00 90 2f 50 09 00 00 00 18".octetsToByteArray()

    TlvWriter().apply {
      putUnsignedLongArray(AnonymousTag, values)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      assertThat(getULong(AnonymousTag)).isEqualTo(values[0].toULong())
      assertThat(getULong(AnonymousTag)).isEqualTo(values[1].toULong())
      assertThat(isEndOfContainer()).isFalse()
      assertThat(getULong(AnonymousTag)).isEqualTo(values[2].toULong())
      assertThat(isEndOfContainer()).isTrue()
      assertThat(isEndOfTlv()).isFalse()
      exitContainer()
      assertThat(isEndOfTlv()).isTrue()
    }
  }

  @Test
  fun putByteStringArray() {
    // Anonumous Array of Signed Integers, [{00 01 02 03 04}, {FF}, {4A EF 88}]
    val values =
      listOf<ByteArray>(
        "0001020304".octetsToByteArray(),
        "FF".octetsToByteArray(),
        "4AEF88".octetsToByteArray()
      )
    val encoding = "16 10 05 00 01 02 03 04 10 01 FF 10 03 4A EF 88 18".octetsToByteArray()

    TlvWriter().apply {
      putByteStringArray(AnonymousTag, values)
      assertThat(getEncoded()).isEqualTo(encoding)
    }

    TlvReader(encoding).apply {
      enterArray(AnonymousTag)
      assertThat(getByteString(AnonymousTag)).isEqualTo(values[0])
      assertThat(getByteString(AnonymousTag)).isEqualTo(values[1])
      assertThat(getByteString(AnonymousTag)).isEqualTo(values[2])
      exitContainer()
    }
  }
}
