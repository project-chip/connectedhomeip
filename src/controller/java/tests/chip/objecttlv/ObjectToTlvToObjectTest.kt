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

package chip.objecttlv

import chip.tlv.*
import com.google.common.truth.Truth.assertThat
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class ObjectToTlvToObjectTest {

  private fun String.octetsToByteArray(): ByteArray =
    replace(" ", "").chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()

  @Test
  fun convertNullToTlv() {
    val value: Any? = null
    val tlv = "14".octetsToByteArray()

    val tlvWriter = TlvWriter().putNull(AnonymousTag)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertIntToTlv() {
    val value = 33
    val tlv = "0021".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertUIntToTlv() {
    val value = 475U
    val tlv = "05db01".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertBooleanToTlv() {
    val value = false
    val tlv = "08".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertFloatToTlv() {
    val value = 1.4f
    val tlv = "0a3333b33f".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertDoubleToTlv() {
    val value = 3.543
    val tlv = "0bf2d24d6210580c40".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertByteArrayToTlv() {
    val value = "0123456789aabbccddeeff".octetsToByteArray()
    val tlv = "100b0123456789aabbccddeeff".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertStringToTlv() {
    val value = "@Hello! Matter!!! I like Matter."
    val tlv =
      "0c204048656c6c6f21204d61747465722121212049206c696b65204d61747465722e".octetsToByteArray()

    val tlvWriter = TlvWriter().put(AnonymousTag, value)
    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    assertThat(value).isEqualTo(compareValue)
  }

  @Test
  fun convertJsonStringToTlv() {
    val value =
      """
        {
            "1:STRUCT": {
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
    val tlv =
      "1535012000142901240200300315546573742042797465537472696e672056616c75652c040568656c6c6f2205e05ef8ff2b066666666666e631402a0733338f411818"
        .octetsToByteArray()

    val tlvWriter =
      TlvWriter().apply {
        startStructure(AnonymousTag)
        startStructure(ContextSpecificTag(1))
        put(ContextSpecificTag(0), 20)
        put(ContextSpecificTag(1), true)
        put(ContextSpecificTag(2), 0U)
        put(ContextSpecificTag(3), "Test ByteString Value".toByteArray())
        put(ContextSpecificTag(4), "hello")
        put(ContextSpecificTag(5), -500000)
        put(ContextSpecificTag(6), 17.9)
        put(ContextSpecificTag(7), 17.9f)
        endStructure()
        endStructure()
      }

    val compareTlvWriter = TlvWriter().fromObject(value)

    assertThat(tlvWriter.getEncoded()).isEqualTo(compareTlvWriter.getEncoded())

    assertThat(tlv).isEqualTo(compareTlvWriter.getEncoded())

    val tlvReader = TlvReader(tlv)
    val compareValue = tlvReader.toObject()

    // Json can't compare String (because of '\n', '\t' etc.). In this reason, check to compare
    // re-parse tlv packet.
    val reparseTlvWriter = TlvWriter().fromObject(compareValue)

    assertThat(tlvWriter.getEncoded()).isEqualTo(reparseTlvWriter.getEncoded())
  }

  @Test
  fun convertListToTlv1() {
    // Int
    val test1 = listOf(1, 3, 5, 7, 9, 2, 4, 6, 8, 10)
    val tlvWriter1 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test1.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter1 = TlvWriter().fromObject(test1)

    assertThat(tlvWriter1.getEncoded()).isEqualTo(compareTlvWriter1.getEncoded())

    // Unsigned Int
    val test2 = listOf(2U, 4U, 6U, 8U, 10U, 1U, 3U, 5U, 7U, 9U)
    val tlvWriter2 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test2.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter2 = TlvWriter().fromObject(test2)

    assertThat(tlvWriter2.getEncoded()).isEqualTo(compareTlvWriter2.getEncoded())

    // Float
    val test3 = listOf(1.2f, 3.4f, 7.8f, -98.3f, 123.3f, 0.123f, -987.1f, 1432.0f, 99.0f, 123.12f)
    val tlvWriter3 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test3.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter3 = TlvWriter().fromObject(test3)

    assertThat(tlvWriter3.getEncoded()).isEqualTo(compareTlvWriter3.getEncoded())

    // Double
    val test4 = listOf(2.5, 4.2, 7.8, 10.6, 123.2, 986.3, 7891.2, 7.0, 1239.123, 9734.34)
    val tlvWriter4 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test4.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter4 = TlvWriter().fromObject(test4)

    assertThat(tlvWriter4.getEncoded()).isEqualTo(compareTlvWriter4.getEncoded())
  }

  @Test
  fun convertListToTlv2() {
    // String
    val test5 = listOf("January", "Feb.", "April", "december", "Happy new year!")
    val tlvWriter5 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test5.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter5 = TlvWriter().fromObject(test5)

    assertThat(tlvWriter5.getEncoded()).isEqualTo(compareTlvWriter5.getEncoded())

    // Boolean
    val test6 = listOf(false, true, false, false, false, true, false)
    val tlvWriter6 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test6.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter6 = TlvWriter().fromObject(test6)

    assertThat(tlvWriter6.getEncoded()).isEqualTo(compareTlvWriter6.getEncoded())

    // ByteArray
    val test7 =
      listOf(
        "1234".octetsToByteArray(),
        "fbcdef".octetsToByteArray(),
        "ff0123".octetsToByteArray(),
        "987654".octetsToByteArray()
      )
    val tlvWriter7 =
      TlvWriter().apply {
        startArray(AnonymousTag)
        test7.forEach { put(AnonymousTag, it) }
        endArray()
      }
    val compareTlvWriter7 = TlvWriter().fromObject(test7)

    assertThat(tlvWriter7.getEncoded()).isEqualTo(compareTlvWriter7.getEncoded())
  }
}
