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

import java.lang.Double.longBitsToDouble
import java.lang.Float.intBitsToFloat

/**
 * Implements Matter TLV reader that supports all values and tags as defined in the Spec.
 *
 * @param bytes the bytes to interpret
 */
class TlvReader(bytes: ByteArray) : Iterable<Element> {
  private val bytes = bytes.copyOf()
  private var index = 0

  /**
   * Reads the next element from the TLV.
   *
   * @throws TlvParsingException if the TLV data was invalid
   */
  fun nextElement(): Element {
    // Ensure that at least one byte for control data is available for reading.
    checkSize("controlByte", 1)
    val controlByte = bytes[index]
    val elementType =
      runCatching { Type.from(controlByte) }
        .onFailure {
          throw TlvParsingException("Type error at $index for ${controlByte.toBinary()}", it)
        }
        .getOrThrow()

    index++

    // Read tag, and advance index past tag bytes
    val tag =
      runCatching { Tag.from(controlByte, index, bytes) }
        .onFailure {
          throw TlvParsingException("Tag error at $index for ${controlByte.toBinary()}", it)
        }
        .getOrThrow()

    index += tag.size

    // Element has either a length section or a fixed number of bytes for the value section. If
    // present, length is encoded as 1 or 2 bytes indicating the number of bytes in the value.
    val lengthSize = elementType.lengthSize
    val valueSize: Int
    if (lengthSize > 0) {
      checkSize("length", lengthSize)
      if (lengthSize > Int.SIZE_BYTES) {
        throw TlvParsingException("Length $lengthSize at $index too long")
      }
      valueSize = bytes.sliceArray(index until index + lengthSize).fromLittleEndianToLong().toInt()
      index += lengthSize
    } else {
      valueSize = elementType.valueSize.toInt()
    }

    // Ensure that the encoded length fits in the range of the array, and advance index to the
    // next control byte.
    checkSize("value", valueSize)
    val valueBytes = bytes.sliceArray(index until index + valueSize)
    index += valueSize

    // Only supporting a small subset of value types currently. Others will just be interpreted
    // as a null value.
    val value: Value =
      when (elementType) {
        is SignedIntType -> IntValue(valueBytes.fromLittleEndianToLong(isSigned = true))
        is UnsignedIntType -> UnsignedIntValue(valueBytes.fromLittleEndianToLong())
        is Utf8StringType -> Utf8StringValue(String(valueBytes, Charsets.UTF_8))
        is ByteStringType -> ByteStringValue(valueBytes)
        is BooleanType -> BooleanValue(elementType.value)
        is FloatType -> FloatValue(intBitsToFloat(valueBytes.fromLittleEndianToLong().toInt()))
        is DoubleType -> DoubleValue(longBitsToDouble(valueBytes.fromLittleEndianToLong()))
        is StructureType -> StructureValue
        is ArrayType -> ArrayValue
        is ListType -> ListValue
        is EndOfContainerType -> EndOfContainerValue
        else -> NullValue
      }

    return Element(tag, value)
  }

  /**
   * Reads the next element from the TLV. Unlike nextElement() this method leaves the TLV reader
   * positioned at the same element and doesn't advance it to the next element.
   *
   * @throws TlvParsingException if the TLV data was invalid
   */
  fun peekElement(): Element {
    val currentIndex = index
    val element = nextElement()
    index = currentIndex
    return element
  }

  /**
   * Reads the encoded Long value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getLong(tag: Tag): Long {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is IntValue) { "Unexpected value $value at index $index (expected IntValue)" }
    return value.value
  }

  /**
   * Reads the encoded ULong value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getULong(tag: Tag): ULong {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is UnsignedIntValue) {
      "Unexpected value $value at index $index (expected UnsignedIntValue)"
    }
    return value.value.toULong()
  }

  /**
   * Reads the encoded Int value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getInt(tag: Tag): Int {
    return checkRange(getLong(tag), Int.MIN_VALUE.toLong()..Int.MAX_VALUE.toLong()).toInt()
  }

  /**
   * Reads the encoded UInt value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getUInt(tag: Tag): UInt {
    return checkRange(getULong(tag), UInt.MIN_VALUE.toULong()..UInt.MAX_VALUE.toULong()).toUInt()
  }

  /**
   * Reads the encoded Short value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getShort(tag: Tag): Short {
    return checkRange(getLong(tag), Short.MIN_VALUE.toLong()..Short.MAX_VALUE.toLong()).toShort()
  }

  /**
   * Reads the encoded UShort value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getUShort(tag: Tag): UShort {
    return checkRange(getULong(tag), UShort.MIN_VALUE.toULong()..UShort.MAX_VALUE.toULong())
      .toUShort()
  }

  /**
   * Reads the encoded Byte value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getByte(tag: Tag): Byte {
    return checkRange(getLong(tag), Byte.MIN_VALUE.toLong()..Byte.MAX_VALUE.toLong()).toByte()
  }

  /**
   * Reads the encoded UByte value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getUByte(tag: Tag): UByte {
    return checkRange(getULong(tag), UByte.MIN_VALUE.toULong()..UByte.MAX_VALUE.toULong()).toUByte()
  }

  /**
   * Reads the encoded Boolean value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getBool(tag: Tag): Boolean {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is BooleanValue) {
      "Unexpected value $value at index $index (expected BooleanValue)"
    }
    return value.value
  }

  /**
   * Reads the encoded Float value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getFloat(tag: Tag): Float {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is FloatValue) { "Unexpected value $value at index $index (expected FloatValue)" }
    return value.value
  }

  /**
   * Reads the encoded Double value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getDouble(tag: Tag): Double {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is DoubleValue) {
      "Unexpected value $value at index $index (expected DoubleValue)"
    }
    return value.value
  }

  /**
   * Reads the encoded UTF8 String value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getUtf8String(tag: Tag): String {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is Utf8StringValue) {
      "Unexpected value $value at index $index (expected Utf8StringValue)"
    }
    return value.value
  }

  /**
   * Reads the encoded Octet String value and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getByteString(tag: Tag): ByteArray {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is ByteStringValue) {
      "Unexpected value $value at index $index (expected ByteStringValue)"
    }
    return value.value
  }

  /**
   * Verifies that the current element is Null with expected tag and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun getNull(tag: Tag) {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is NullValue) { "Unexpected value $value at index $index (expected NullValue)" }
  }

  /**
   * Retrieves a Boolean value associated with the given tag.
   *
   * @param tag The Tag to query for.
   * @return The Boolean value associated with the tag.
   */
  fun getBoolean(tag: Tag): Boolean {
    return getBool(tag)
  }

  /**
   * Retrieves a String value associated with the given tag. The returned string is in UTF-8 format.
   *
   * @param tag The Tag to query for.
   * @return The String value associated with the tag.
   */
  fun getString(tag: Tag): String {
    return getUtf8String(tag)
  }

  /**
   * Retrieves a ByteArray value associated with the given tag.
   *
   * @param tag The Tag to query for.
   * @return The ByteArray value associated with the tag.
   */
  fun getByteArray(tag: Tag): ByteArray {
    return getByteString(tag)
  }

  /**
   * Checks if the current element's value is of type NullValue.
   *
   * @return True if the current element's value is NullValue, otherwise false.
   */
  fun isNull(): Boolean {
    val value = peekElement().value
    return (value is NullValue)
  }

  /**
   * Checks if the next tag in sequence matches the provided tag.
   *
   * @param tag The Tag to compare against the next tag.
   * @return True if the next tag matches the provided tag, otherwise false.
   */
  fun isNextTag(tag: Tag): Boolean {
    val nextTag = peekElement().tag
    return (nextTag == tag)
  }

  /**
   * Verifies that the current element is a start of a Structure and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun enterStructure(tag: Tag) {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is StructureValue) {
      "Unexpected value $value at index $index (expected StructureValue)"
    }
  }

  /**
   * Verifies that the current element is a start of an Array and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun enterArray(tag: Tag) {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is ArrayValue) { "Unexpected value $value at index $index (expected ArrayValue)" }
  }

  /**
   * Verifies that the current element is a start of a List and advances to the next element.
   *
   * @throws TlvParsingException if the element is not of the expected type or tag
   */
  fun enterList(tag: Tag) {
    val value = nextElement().verifyTagAndGetValue(tag)
    require(value is ListValue) { "Unexpected value $value at index $index (expected ListValue)" }
  }

  /**
   * Completes the reading of a Tlv container and prepares to read elements after the container.
   *
   * Note that if a TlvReader is not currently positioned at the EndOfContainerValue then function
   * skips all unread element in container until it finds the end.
   *
   * @throws TlvParsingException if the end of the container element is not found
   */
  fun exitContainer() {
    var relevantDepth = 1
    while (relevantDepth > 0) {
      val value = nextElement().value
      if (value is EndOfContainerValue) {
        relevantDepth--
      } else if (value is StructureValue || value is ArrayValue || value is ListValue) {
        relevantDepth++
      }
    }
  }

  private fun Element.verifyTagAndGetValue(expectedTag: Tag): Value {
    require(tag == expectedTag) {
      "Unexpected value tag $tag at index $index (expected $expectedTag)"
    }
    return value
  }

  /**
   * Skips the current element and advances to the next element.
   *
   * @throws TlvParsingException if the TLV data was invalid
   */
  fun skipElement() {
    nextElement()
  }

  /** Returns the total number of bytes read since the TlvReader was initialized. */
  fun getLengthRead(): Int {
    return index
  }

  /** Returns the total number of bytes that can be read until the end of TLV data is reached. */
  fun getRemainingLength(): Int {
    return bytes.size - index
  }

  /** Returns true if TlvReader is positioned at the end of container. */
  fun isEndOfContainer(): Boolean {
    // Ensure that at least one byte for control data is available for reading.
    checkSize("controlByte", 1)
    return bytes[index] == EndOfContainerType.encode()
  }

  /** Returns true if TlvReader reached the end of Tlv data. Returns false otherwise. */
  fun isEndOfTlv(): Boolean {
    return bytes.size == index
  }

  /** Resets the reader to the start of the provided byte array. */
  fun reset() {
    index = 0
  }

  override fun iterator(): Iterator<Element> {
    return object : AbstractIterator<Element>() {
      override fun computeNext() {
        if (index < bytes.size) {
          setNext(nextElement())
        } else {
          done()
        }
      }
    }
  }

  private fun checkSize(propertyName: String, size: Number) {
    if (index + size.toInt() > bytes.size) {
      throw TlvParsingException(
        "Invalid $propertyName length $size at index $index with ${bytes.size - index} available."
      )
    }
  }

  private fun <T : Comparable<T>> checkRange(
    value: T,
    range: ClosedRange<T>,
    message: String = "Value $value at index $index is out of range $range"
  ): T {
    if (value !in range) {
      throw TlvParsingException(message)
    }
    return value
  }
}

/** Exception thrown if there was an issue decoding the Matter TLV data. */
class TlvParsingException internal constructor(msg: String, cause: Throwable? = null) :
  RuntimeException(msg, cause)
