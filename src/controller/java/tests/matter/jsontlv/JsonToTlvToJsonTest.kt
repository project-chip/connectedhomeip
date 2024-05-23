/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2023 Google LLC.
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

package matter.jsontlv

import com.google.common.truth.Truth.assertThat
import com.google.gson.JsonParser
import kotlin.test.assertFailsWith
import matter.tlv.*
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class JsonToTlvToJsonTest {

  private fun String.octetsToByteArray(): ByteArray =
    replace(" ", "").chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()

  private fun checkValidConversion(
    jsonOriginal: String,
    tlvEncoding: ByteArray,
    jsonExpected: String = jsonOriginal
  ) {
    assertThat(TlvWriter().fromJsonString(jsonOriginal)).isEqualTo(tlvEncoding)
    assertThat(TlvReader(tlvEncoding).toJsonString())
      .isEqualTo(JsonParser.parseString(jsonExpected).asJsonObject.toString())
    if (jsonOriginal != jsonExpected) {
      assertThat(TlvWriter().fromJsonString(jsonExpected)).isEqualTo(tlvEncoding)
    }
  }

  @Test
  fun convertBoolean_false() {
    // Boolean false
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), false)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "0:BOOL" : false
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertBoolean_true() {
    // Boolean true
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), true)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:BOOL" : true
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertSignedInt_1BytePositive() {
    // Signed Integer 42, 1-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(2), 42)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "2:INT" : 42
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertSignedInt_1ByteNegative() {
    // Signed Integer -17, 1-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(3), -17)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "3:INT" : -17
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertUnsignedInt_1Byte() {
    // Unsigned Integer 42, 1-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(4), 42U)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "value:4:UINT" : 42
      }
      """
    val expectedJson = """
      {
        "4:UINT" : 42
      }
      """

    checkValidConversion(json, encoding, expectedJson)
  }

  @Test
  fun convertSignedInt_1Byte2octet() {
    // Signed Integer 42, 1-byte encoded as 2-octet
    val encoding = "15 21 06 2a 00 18".octetsToByteArray()
    val expectedJson = """
      {
        "6:INT" : 42
      }
      """

    // Note: the current implementation follows the minimum encoding policy, which encodes this
    // value as 1-octet. Testing only decoding.
    assertThat(TlvReader(encoding).toJsonString())
      .isEqualTo(JsonParser.parseString(expectedJson).asJsonObject.toString())
  }

  @Test
  fun convertSignedInt_2Bytes() {
    // Signed Integer 4242, 2-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(7), 4242)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "7:INT" : 4242
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertSignedInt_4Bytes() {
    // Signed Integer -170000, 4-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(80), -170000)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "80:INT" : -170000
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertSignedInt_8Bytes() {
    // Signed Integer (Long) 40000000000, 8-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(202), 40000000000)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "202:INT" : "40000000000"
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertUnsignedInt_8Bytes() {
    // Unsigned Integer (Long) 40000000000, 8-octet
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(222), 40000000000U)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "222:UINT" : "40000000000"
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertUtf8String_hello() {
    // UTF-8 String, 1-octet length, "Hello!"
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), "Hello!")
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "0:STRING" : "Hello!"
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertUtf8String_tschuh() {
    // UTF-8 String, 1-octet length, "Tschüs"
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(250), "Tschüs")
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "250:STRING" : "Tschüs"
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertOctetString() {
    // Octet String, 1-octet length, octets 00 01 02 03 04
    val value = "0001020304".octetsToByteArray()
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), value)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:BYTES" : "AAECAwQ="
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertNull() {
    // Null
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putNull(ContextSpecificTag(1))
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:NULL" : null
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertFloat_0() {
    // Single precision floating point 0.0
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), 0.0f)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:FLOAT" : 0.0
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertFloat_1third() {
    // Single precision floating point (1.0 / 3.0)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(100), 1.0f / 3.0f)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "100:FLOAT" : 0.33333334
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertFloat_17_9() {
    // Single precision floating point 17.9
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(101), 17.9f)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "101:FLOAT" : 17.9
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertFloat_positiveInfinity_throwsIllegalArgumentException() {
    // Single precision floating point infinity (∞)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), Float.POSITIVE_INFINITY)
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because the encoded value is unsupported Infinity value
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertFloat_negativeInfinity_throwsIllegalArgumentException() {
    // Single precision floating point negative infinity (-∞)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), Float.NEGATIVE_INFINITY)
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because the encoded value is unsupported Infinity value
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertDouble_0() {
    // Double precision floating point 0.0
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), 0.0)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:DOUBLE" : 0.0
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertDouble_1third() {
    // Double precision floating point (1.0 / 3.0)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), 1.0 / 3.0)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:DOUBLE" : 0.3333333333333333
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertDouble_17_9() {
    // Double precision floating point 17.9
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), 17.9)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:DOUBLE" : 17.9
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertDouble_positiveInfinity_throwsIllegalArgumentException() {
    // Double precision floating point infinity (∞)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), Double.POSITIVE_INFINITY)
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because the encoded value is unsupported Infinity value
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertDouble_negativeInfinity_throwsIllegalArgumentException() {
    // Double precision floating point negative infinity (-∞)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(1), Double.NEGATIVE_INFINITY)
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because the encoded value is unsupported Infinity value
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertEmptyStructure() {
    // Empty Structure, {}
    val encoding =
      TlvWriter().startStructure(AnonymousTag).endStructure().validateTlv().getEncoded()
    val json = """
      {
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertEmptyStructureWithinStructure() {
    // Empty Structure, {}
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(ContextSpecificTag(1))
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:STRUCT" : {}
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertEmptyStructureWithCommonProfileTag_throwsIllegalArgumentException() {
    // Empty Structure, {}
    val encoding =
      TlvWriter()
        .startStructure(CommonProfileTag(2, 1000u))
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because top level element must be anonymous Structure
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertTopLevelArray_throwsIllegalArgumentException() {
    // Empty Array, []
    val tag = AnonymousTag
    val encoding = TlvWriter().startArray(tag).endArray().validateTlv().getEncoded()

    // Throws exception because top level element must be anonymous Structure
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertTopLevelInt_throwsIllegalArgumentException() {
    // Empty Array, []
    val value = 42
    val tag = CommonProfileTag(2, 1000u)
    val encoding = TlvWriter().put(tag, value).validateTlv().getEncoded()

    // Throws exception because top level element must be anonymous Structure
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertArray_empty() {
    // Empty Array, []
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(1))
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1:ARRAY-?" : []
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArrayWithCommonProfileTag2_empty() {
    // Empty Array, []
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(CommonProfileTag(2, 10000u))
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "value:10000:ARRAY-?" : []
      }
      """
    val expectedJson = """
      {
        "10000:ARRAY-?" : []
      }
      """

    checkValidConversion(json, encoding, expectedJson)
  }

  @Test
  fun convertArrayWithCommonProfileTag4_empty() {
    // Empty Array, []
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(CommonProfileTag(4, 1000000u))
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1000000:ARRAY-?" : []
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertList_empty_throwsIllegalArgumentException() {
    // Empty List, []
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startList(ContextSpecificTag(1))
        .endList()
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because TLV Lists are not supported
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertIntegersWithContextTags() {
    // Structure, two context specific tags, Signed Integer, 1 octet values, {0 = 42, 1 = -17}
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), 42)
        .put(ContextSpecificTag(1), -17)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "0:INT" : 42,
        "1:INT" : -17
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertStructure_intsWithContextTags() {
    // Structure, two context specific tags, Signed Integer, 1 octet values, {0 = 42, 1 = -17}
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(ContextSpecificTag(0))
        .put(ContextSpecificTag(0), 42)
        .put(ContextSpecificTag(1), -17)
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:STRUCT" : {
          "0:INT" : 42,
          "1:INT" : -17
        }
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_ints() {
    // Array of Signed Integers (1-octet values): [0, 1, 2, 3, 4]
    val values = longArrayOf(0, 1, 2, 3, 4)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putSignedLongArray(ContextSpecificTag(0), values)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-INT" : [
          0,
          1,
          2,
          3,
          4
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_double() {
    // Array of Doubles: [1.1, 134.2763, -12345.87]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(0))
        .put(AnonymousTag, 1.1)
        .put(AnonymousTag, 134.2763)
        .put(AnonymousTag, -12345.87)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-DOUBLE" : [
          1.1,
          134.2763,
          -12345.87
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_float() {
    // Array of Floats: [1.1, 134.2763, -12345.87]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(CommonProfileTag(2, 1000u))
        .put(AnonymousTag, 1.1f)
        .put(AnonymousTag, 134.2763f)
        .put(AnonymousTag, -12345.87f)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "1000:ARRAY-FLOAT" : [
          1.1,
          134.2763,
          -12345.87
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_string() {
    // Array of Strings: ["ABC", "Options", "more"]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(CommonProfileTag(4, 100000u))
        .put(AnonymousTag, "ABC")
        .put(AnonymousTag, "Options")
        .put(AnonymousTag, "more")
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "100000:ARRAY-STRING" : [
          "ABC",
          "Options",
          "more"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_boolean() {
    // Array of Booleans: [true, false, false]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(255))
        .put(AnonymousTag, true)
        .put(AnonymousTag, false)
        .put(AnonymousTag, false)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "255:ARRAY-BOOL" : [
          true,
          false,
          false
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_null() {
    // Array of Nulls: [null, null]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(1))
        .putNull(AnonymousTag)
        .putNull(AnonymousTag)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "1:ARRAY-NULL" : [
          null,
          null
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_boolean_throwsIllegalArgumentException() {
    // Array of bools: error type doesn't match
    val json =
      """
      {
        "value:1:ARRAY-BOOL" : [
          "yes",
          "no"
        ]
      }
      """

    // Throws exception because subtype encoded in the Json key (Boolean) doesn't match the
    // String
    // type of the elements in the array
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertArray_uint_throwsIllegalArgumentException() {
    // Array of unsigned integers: error type doesn't match
    val json =
      """
      {
        "value:1:ARRAY-UINT" : [
          "yes",
          "no"
        ]
      }
      """

    // Throws exception because subtype encoded in the Json key (Boolean) doesn't match the
    // String
    // type of the elements in the array
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertArray_float_throwsIllegalArgumentException() {
    // Array of floats: error type doesn't match
    val json =
      """
      {
        "1:ARRAY-FLOAT" : [
          {
            "1" : 22,
            "2" : 23
          }
        ]
      }
      """

    // Throws exception because subtype encoded in the Json key (Float) doesn't match the
    // Structure
    // type of the elements in the array
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertArray_uint2_throwsIllegalArgumentException() {
    // Array of unsigned integer: error type doesn't match
    val json = """
      {
        "2:ARRAY-UINT" : [
          null
        ]
      }
      """

    // Throws exception because subtype encoded in the Json key (UInt) doesn't match the Null
    // type of the elements in the array
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertByteStringArray_throwsIllegalArgumentException() {
    // Anonymous Array of ByteString, [{00 01 02 03 04}, {FF}, {4A EF 88}]
    val json =
      """
      {
        "value:ARRAY-BYTES": [
          "AA45ECAwQ="
        ]
      }
      """

    // Throws exception because string is invalid base64 encoded value
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertArray_mixedValues_throwsIllegalArgumentException() {
    // Array of mixed elements: [42, -170000, {}, 17.9, "Hello!"]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(0))
        .put(AnonymousTag, 42)
        .put(AnonymousTag, -170000)
        .startStructure(AnonymousTag)
        .endStructure()
        .put(AnonymousTag, 17.9)
        .put(AnonymousTag, "Hello!")
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Throws exception because TLV Array Must have same type elements
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertAnonymousTag_throwsIllegalArgumentException() {
    // Anonymous tag, Unsigned Integer, 1-octet value, 42U
    val json = """
      {
        "value:UINT" : 42
      }
      """

    // Throws exception because element within structure cannot have anonymous tag
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertKeyWithoutTag_throwsIllegalArgumentException2() {
    // Anonymous tag, Unsigned Integer, 1-octet value, 42U
    val json = """
      {
        "UINT" : 42
      }
      """

    // Throws exception because Json key must have valid tag field
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertContextTag_withinStructure() {
    // Context tag 255 (max), Unsigned Integer, 1-octet value: {255 = 42U}
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(ContextSpecificTag(0))
        .put(ContextSpecificTag(255), 42U)
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "value:0:STRUCT": {
          "name:255:UINT" : 42
        }
      }
      """
    val expectedJson =
      """
      {
        "0:STRUCT": {
          "255:UINT" : 42
        }
      }
      """

    checkValidConversion(json, encoding, expectedJson)
  }

  @Test
  fun convertStructWithMixedTags() {
    // Context and Common Profile tags, Unsigned Integer structure: {255 = 42, 256 = 17000,
    // 65535 =
    // 1, 65536 = 345678, 4294967295 = 500000000000}
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(ContextSpecificTag(0))
        .put(ContextSpecificTag(UByte.MAX_VALUE.toInt()), 42U)
        .put(CommonProfileTag(2, UByte.MAX_VALUE + 1U), 17000U)
        .put(CommonProfileTag(2, UShort.MAX_VALUE.toUInt()), 1U)
        .put(CommonProfileTag(4, UShort.MAX_VALUE + 1U), 345678U)
        .put(CommonProfileTag(4, UInt.MAX_VALUE), 500000000000U)
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:STRUCT": {
          "255:UINT" : 42,
          "256:UINT" : 17000,
          "65535:UINT" : 1,
          "65536:UINT" : 345678,
          "4294967295:UINT" : "500000000000"
        }
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertContextTag_invalidContextTag_throwsIllegalArgumentException() {
    // Context and Common Profile tags, Unsigned Integer structure: {255 = 42, 256 = 17000,
    // 65535 =
    // 1, 65536 = 345678, 4294967295 = 500000000000, , 4294967296 = 34}
    val json =
      """
      {
        "0:STRUCT": {
          "255:UINT" : 42,
          "256:UINT" : 17000,
          "65535:UINT" : 1,
          "65536:UINT" : 345678,
          "4294967295:UINT" : "500000000000",
          "invalid:4294967296:UINT" : 34
        }
      }
      """

    // 4294967296 exceeds valid context specific or common profile tag value of 32-bits
    assertFailsWith<IllegalArgumentException> { TlvWriter().fromJsonString(json) }
  }

  @Test
  fun convertCommonProfileTag2() {
    // Common profile tag 1, Unsigned Integer, 1-octet value, Matter::1 = 42U
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(CommonProfileTag(2, 1000u), 42U)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "1000:UINT" : 42
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun encodeCommonProfileTag4() {
    // Common profile tag 100000, Unsigned Integer, 1-octet value, Matter::100000 = 42U
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(CommonProfileTag(4, 100000u), 42U)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json = """
      {
        "100000:UINT" : 42
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertFullyQualifiedTag8_throwsIllegalArgumentException() {
    // Fully qualified tag, Vendor ID 0xFFF1/65521, profile number 0xDEED/57069, 4-octet tag
    // 0xAA55FEED/2857762541, Unsigned Integer, 1-octet value 42, 65521::57069:2857762541 = 42U
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(FullyQualifiedTag(8, 0xFFF1u, 0xDEEDu, 0xAA55FEEDu), 42U)
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Fully qualified tags are not supported
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertFullyQualifiedTags_throwsIllegalArgumentException() {
    // Structure with the fully qualified tag, Vendor ID 0xFFF1/65521, profile number
    // 0xDEED/57069,
    // 2-octet tag 1. The structure contains a single element labeled using a fully qualified
    // tag
    // under the same profile, with 2-octet tag 0xAA55/43605.65521::57069:1 =
    // {65521::57069:43605 =
    // 42U}
    val value = 42U
    val structTag = FullyQualifiedTag(6, 0xFFF1u, 0xDEEDu, 1u)
    val valueTag = FullyQualifiedTag(6, 0xFFF1u, 57069u, 43605u)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(structTag)
        .put(valueTag, value)
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()

    // Fully qualified tags are not supported
    assertFailsWith<IllegalArgumentException> { TlvReader(encoding).toJsonString() }
  }

  @Test
  fun convertSignedLongArray() {
    // Anonymous Array of Signed Integers: [42, -17, -170000, 40000000000]
    val values = longArrayOf(42, -17, -170000, 40000000000)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putSignedLongArray(ContextSpecificTag(0), values)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-INT": [
          42,
          -17,
          -170000,
          "40000000000"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertUnsignedLongArray() {
    // Anonymous Array of Unigned Integers: [42, 170000, 40000000000]
    val values = longArrayOf(42, 170000, 40000000000)
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putUnsignedLongArray(ContextSpecificTag(0), values)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-UINT": [
          42,
          170000,
          "40000000000"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertByteStringArray() {
    // Anonymous Array of ByteArray, [{00 01 02 03 04}, {FF}, {4A EF 88}]
    val values =
      listOf<ByteArray>(
        "0001020304".octetsToByteArray(),
        "FF".octetsToByteArray(),
        "4AEF88".octetsToByteArray()
      )
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putByteStringArray(ContextSpecificTag(0), values)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-BYTES": [
          "AAECAwQ=",
          "/w==",
          "Su+I"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertStructure_Mixed() {
    // Structure with mixed elements
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(ContextSpecificTag(0))
        .put(ContextSpecificTag(0), 20.toLong())
        .put(ContextSpecificTag(1), true)
        .put(ContextSpecificTag(2), 0.toULong())
        .put(ContextSpecificTag(3), "Test ByteString Value".toByteArray())
        .put(ContextSpecificTag(4), "hello")
        .put(ContextSpecificTag(5), -500000)
        .put(ContextSpecificTag(6), 17.9)
        .put(ContextSpecificTag(7), 17.9f)
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:STRUCT": {
          "0:INT": 20,
          "1:BOOL": true,
          "2:UINT": 0,
          "3:BYTES": "VGVzdCBCeXRlU3RyaW5nIFZhbHVl",
          "4:STRING": "hello",
          "5:INT": -500000,
          "6:DOUBLE": 17.9,
          "7:FLOAT": 17.9
        }
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArrayofStructureWithMixedElements() {
    // Array of structures with mixed elements
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(CommonProfileTag(2, 1000U))
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), 20)
        .put(ContextSpecificTag(1), true)
        .put(ContextSpecificTag(2), 0.toULong())
        .put(ContextSpecificTag(3), "Test ByteString Value 1".toByteArray())
        .put(ContextSpecificTag(4), "hello1")
        .put(ContextSpecificTag(5), -500000)
        .put(ContextSpecificTag(6), 17.9)
        .put(ContextSpecificTag(7), 17.9f)
        .endStructure()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), -10)
        .put(ContextSpecificTag(1), false)
        .put(ContextSpecificTag(2), 128.toULong())
        .put(ContextSpecificTag(3), "Test ByteString Value 2".toByteArray())
        .put(ContextSpecificTag(4), "hello2")
        .put(ContextSpecificTag(5), 40000000000)
        .put(ContextSpecificTag(6), -1754.923)
        .put(ContextSpecificTag(7), 97.945f)
        .endStructure()
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "1000:ARRAY-STRUCT": [
          {
            "0:INT": 20,
            "1:BOOL": true,
            "2:UINT": 0,
            "3:BYTES": "VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDE=",
            "4:STRING": "hello1",
            "5:INT": -500000,
            "6:DOUBLE": 17.9,
            "7:FLOAT": 17.9
          },
          {
            "0:INT": -10,
            "1:BOOL": false,
            "2:UINT": 128,
            "3:BYTES": "VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDI=",
            "4:STRING": "hello2",
            "5:INT": "40000000000",
            "6:DOUBLE": -1754.923,
            "7:FLOAT": 97.945
          }
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertingStructureWithMixedElementsAndNames() {
    // Array of structures with mixed elements
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startStructure(CommonProfileTag(4, 444444U))
        .startStructure(ContextSpecificTag(0))
        .put(ContextSpecificTag(0), "Test String Element #0")
        .put(ContextSpecificTag(1), 1)
        .put(ContextSpecificTag(2), true)
        .endStructure()
        .put(ContextSpecificTag(1), 17.4f)
        .startStructure(ContextSpecificTag(2))
        .put(ContextSpecificTag(0), "Test String Element #2")
        .put(ContextSpecificTag(3), "Test ByteString Element #3".toByteArray())
        .endStructure()
        .endStructure()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "value_name:444444:STRUCT": {
          "0:STRUCT": {
            "name0:0:STRING": "Test String Element #0",
            "name1:1:INT": 1,
            "name2:2:BOOL": true
          },
          "name2:1:FLOAT": 17.4,
          "2:STRUCT": {
            "name0:0:STRING": "Test String Element #2",
            "3:BYTES": "VGVzdCBCeXRlU3RyaW5nIEVsZW1lbnQgIzM="
          }
        }
      }
      """
    val expectedJson =
      """
      {
        "444444:STRUCT": {
          "0:STRUCT": {
            "0:STRING": "Test String Element #0",
            "1:INT": 1,
            "2:BOOL": true
          },
          "1:FLOAT": 17.4,
          "2:STRUCT": {
            "0:STRING": "Test String Element #2",
            "3:BYTES": "VGVzdCBCeXRlU3RyaW5nIEVsZW1lbnQgIzM="
          }
        }
      }
      """

    checkValidConversion(json, encoding, expectedJson)
  }

  @Test
  fun convertArrayOfStrings() {
    // Array of String elements
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(0))
        .put(AnonymousTag, "Test array member 0")
        .put(AnonymousTag, "Test array member 1")
        .put(AnonymousTag, "Test array member 2")
        .put(AnonymousTag, "Test array member 3")
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-STRING": [
          "Test array member 0",
          "Test array member 1",
          "Test array member 2",
          "Test array member 3"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArrayOfByteStrings() {
    // Array of ByteArray elements
    val values =
      listOf<ByteArray>(
        "Test array member 0".toByteArray(),
        "Test array member 1".toByteArray(),
        "Test array member 2".toByteArray(),
        "Test array member 3".toByteArray(),
      )
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .putByteStringArray(ContextSpecificTag(0), values)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-BYTES": [
          "VGVzdCBhcnJheSBtZW1iZXIgMA==",
          "VGVzdCBhcnJheSBtZW1iZXIgMQ==",
          "VGVzdCBhcnJheSBtZW1iZXIgMg==",
          "VGVzdCBhcnJheSBtZW1iZXIgMw=="
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertTopLevelMixedValues() {
    // Top level elements with mixed values
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .put(ContextSpecificTag(0), 42)
        .put(ContextSpecificTag(1), "Test array member 0".toByteArray())
        .put(ContextSpecificTag(2), 156.398)
        .put(ContextSpecificTag(3), 73709551615U)
        .put(ContextSpecificTag(4), true)
        .putNull(ContextSpecificTag(5))
        .startStructure(ContextSpecificTag(6))
        .put(ContextSpecificTag(1), "John")
        .put(ContextSpecificTag(2), 34U)
        .put(ContextSpecificTag(3), true)
        .startArray(ContextSpecificTag(4))
        .put(AnonymousTag, 5)
        .put(AnonymousTag, 9)
        .put(AnonymousTag, 10)
        .endArray()
        .startArray(ContextSpecificTag(5))
        .put(AnonymousTag, "Ammy")
        .put(AnonymousTag, "David")
        .put(AnonymousTag, "Larry")
        .endArray()
        .startArray(ContextSpecificTag(6))
        .put(AnonymousTag, true)
        .put(AnonymousTag, false)
        .put(AnonymousTag, true)
        .endArray()
        .endStructure()
        .put(ContextSpecificTag(7), 0.0f)
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "value:0:INT": 42,
        "value:1:BYTES": "VGVzdCBhcnJheSBtZW1iZXIgMA==",
        "value:2:DOUBLE": 156.398,
        "value:3:UINT": "73709551615",
        "value:4:BOOL": true,
        "value:5:NULL": null,
        "value:6:STRUCT": {
          "name:1:STRING": "John",
          "age:2:UINT": 34,
          "approved:3:BOOL": true,
          "kids:4:ARRAY-INT": [
            5,
            9,
            10
          ],
          "names:5:ARRAY-STRING": [
            "Ammy",
            "David",
            "Larry"
          ],
          "6:ARRAY-BOOL": [
            true,
            false,
            true
          ]
        },
        "value:7:FLOAT": 0.0
      }
      """
    val expectedJson =
      """
      {
        "0:INT": 42,
        "1:BYTES": "VGVzdCBhcnJheSBtZW1iZXIgMA==",
        "2:DOUBLE": 156.398,
        "3:UINT": "73709551615",
        "4:BOOL": true,
        "5:NULL": null,
        "6:STRUCT": {
          "1:STRING": "John",
          "2:UINT": 34,
          "3:BOOL": true,
          "4:ARRAY-INT": [
            5,
            9,
            10
          ],
          "5:ARRAY-STRING": [
            "Ammy",
            "David",
            "Larry"
          ],
          "6:ARRAY-BOOL": [
            true,
            false,
            true
          ]
        },
        "7:FLOAT": 0.0
      }
      """

    checkValidConversion(json, encoding, expectedJson)
  }

  @Test
  fun convertArray_UIntMinMax() {
    // Array of Unsigned Integers, where each element represents MAX possible value for unsigned
    // integere
    // types UByte, UShort, UInt, and ULong: [0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFF_FFFFFFFF]
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(0))
        .put(AnonymousTag, UByte.MAX_VALUE)
        .put(AnonymousTag, UShort.MAX_VALUE)
        .put(AnonymousTag, UInt.MAX_VALUE)
        .put(AnonymousTag, ULong.MAX_VALUE)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-UINT": [
          255,
          65535,
          4294967295,
          "18446744073709551615"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }

  @Test
  fun convertArray_IntMinMax() {
    // Array of Integers, where each element represents MIN or MAX possible value for integere
    // types Byte, Short, Int, and Long
    val encoding =
      TlvWriter()
        .startStructure(AnonymousTag)
        .startArray(ContextSpecificTag(0))
        .put(AnonymousTag, Byte.MIN_VALUE)
        .put(AnonymousTag, Byte.MAX_VALUE)
        .put(AnonymousTag, Short.MIN_VALUE)
        .put(AnonymousTag, Short.MAX_VALUE)
        .put(AnonymousTag, Int.MIN_VALUE)
        .put(AnonymousTag, Int.MAX_VALUE)
        .put(AnonymousTag, Long.MIN_VALUE)
        .put(AnonymousTag, Long.MAX_VALUE)
        .endArray()
        .endStructure()
        .validateTlv()
        .getEncoded()
    val json =
      """
      {
        "0:ARRAY-INT": [
          -128,
          127,
          -32768,
          32767,
          -2147483648,
          2147483647,
          "-9223372036854775808",
          "9223372036854775807"
        ]
      }
      """

    checkValidConversion(json, encoding)
  }
}
