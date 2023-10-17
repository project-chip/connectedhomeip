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

import kotlin.experimental.and
import kotlin.experimental.or

private const val MODIFIED_TYPE_MASK = 0b00011100.toByte()
private const val ELEMENT_TYPE_MASK = 0b00011111.toByte()
private const val SIGNED_INT_TYPE = 0b00000000.toByte()
private const val UNSIGNED_INT_TYPE = 0b00000100.toByte()
private const val UTF8_STRING_TYPE = 0b00001100.toByte()
private const val BYTE_STRING_TYPE = 0b00010000.toByte()
private const val BOOLEAN_FALSE = 0b01000.toByte()
private const val BOOLEAN_TRUE = 0b01001.toByte()
private const val FLOATING_POINT_4 = 0b01010.toByte()
private const val FLOATING_POINT_8 = 0b01011.toByte()
private const val NULL = 0b10100.toByte()
private const val STRUCTURE = 0b10101.toByte()
private const val ARRAY = 0b10110.toByte()
private const val LIST = 0b10111.toByte()
private const val END_OF_CONTAINER = 0b11000.toByte()

/**
 * Represents the type of element for a TLV element.
 *
 * @property lengthSize the size, in bytes, of the length section of this element
 * @property valueSize the size, in bytes, of this element's value section
 */
internal sealed class Type(val lengthSize: Short, val valueSize: Short) {
  abstract fun encode(): Byte

  internal companion object {
    /** Returns the element type encoded by the given control byte. */
    fun from(controlByte: Byte): Type {
      // Integer and string types encode the length in the lower two bits. For these types,
      // ignore the lower 2 bits for matching, and extract that size later.
      val modifiedControlByte =
        when (val byte = controlByte and MODIFIED_TYPE_MASK) {
          SIGNED_INT_TYPE,
          UNSIGNED_INT_TYPE,
          UTF8_STRING_TYPE,
          BYTE_STRING_TYPE -> byte
          else -> controlByte and ELEMENT_TYPE_MASK
        }

      return when (modifiedControlByte) {
        SIGNED_INT_TYPE -> SignedIntType(extractSize(controlByte))
        UNSIGNED_INT_TYPE -> UnsignedIntType(extractSize(controlByte))
        UTF8_STRING_TYPE -> Utf8StringType(extractSize(controlByte))
        BYTE_STRING_TYPE -> ByteStringType(extractSize(controlByte))
        BOOLEAN_FALSE -> BooleanType(false)
        BOOLEAN_TRUE -> BooleanType(true)
        FLOATING_POINT_4 -> FloatType()
        FLOATING_POINT_8 -> DoubleType()
        NULL -> NullType()
        STRUCTURE -> StructureType()
        ARRAY -> ArrayType()
        LIST -> ListType()
        END_OF_CONTAINER -> EndOfContainerType
        else ->
          throw IllegalStateException(
            "Unexpected control byte ${modifiedControlByte.toBinaryString()}"
          )
      }
    }

    private fun Byte.toBinaryString() = Integer.toBinaryString(0xFF and this.toInt())

    private fun extractSize(byte: Byte): Short {
      // Variably-sized element types encode their length in the lower 2 bits.
      return when (byte and 0b011) {
        0b000.toByte() -> 1
        0b001.toByte() -> 2
        0b010.toByte() -> 4
        else -> 8
      }
    }
  }
}

private fun encodeSize(size: Short): Byte {
  // Variably-sized element types encode their length in the lower 2 bits.
  return when (size.toInt()) {
    1 -> 0b000.toByte()
    2 -> 0b001.toByte()
    4 -> 0b010.toByte()
    8 -> 0b011.toByte()
    else -> throw IllegalStateException("Unexpected size ${size}")
  }
}

/** Represents a signed integer value. */
internal class SignedIntType(valueSize: Short) : Type(0, valueSize) {
  override fun encode() = SIGNED_INT_TYPE or encodeSize(valueSize)
}

/** Represents an unsigned integer value as a Long. */
internal class UnsignedIntType(valueSize: Short) : Type(0, valueSize) {
  override fun encode() = UNSIGNED_INT_TYPE or encodeSize(valueSize)
}

/** Represents a boolean value. */
internal class BooleanType(val value: Boolean) : Type(0, 0) {
  override fun encode() = if (value) BOOLEAN_TRUE else BOOLEAN_FALSE
}

/** Represents a floating-point float value. */
internal class FloatType : Type(0, 4) {
  override fun encode() = FLOATING_POINT_4
}

/** Represents a floating-point double value. */
internal class DoubleType : Type(0, 8) {
  override fun encode() = FLOATING_POINT_8
}

/** Represents a UTF-8 string value. */
internal class Utf8StringType(lengthSize: Short) : Type(lengthSize, 0) {
  override fun encode() = UTF8_STRING_TYPE or encodeSize(lengthSize)
}

/** Represents a byte string value. */
internal class ByteStringType(lengthSize: Short) : Type(lengthSize, 0) {
  override fun encode() = BYTE_STRING_TYPE or encodeSize(lengthSize)
}

/** Represents a null value. */
internal class NullType : Type(0, 0) {
  override fun encode() = NULL
}

/** Represents a structure container type. */
internal class StructureType : Type(0, 0) {
  override fun encode() = STRUCTURE
}

/** Represents an array container type. */
internal class ArrayType : Type(0, 0) {
  override fun encode() = ARRAY
}

/** Represents a list container type. */
internal class ListType : Type(0, 0) {
  override fun encode() = LIST
}

/** Represents the end of a container type. */
internal object EndOfContainerType : Type(0, 0) {
  override fun encode() = END_OF_CONTAINER
}
