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

private const val TAG_MASK = 0b11100000.toByte()
private const val ANONYMOUS = 0.toByte()
private const val CONTEXT_SPECIFIC = 0b00100000.toByte()
private const val COMMON_PROFILE_2 = 0b01000000.toByte()
private const val COMMON_PROFILE_4 = 0b01100000.toByte()
private const val IMPLICIT_PROFILE_2 = 0b10000000.toByte()
private const val IMPLICIT_PROFILE_4 = 0b10100000.toByte()
private const val FULLY_QUALIFIED_6 = 0b11000000.toByte()
private const val FULLY_QUALIFIED_8 = 0b11100000.toByte()

/** Represents a tag within a TLV element. */
sealed class Tag {
  /** The number of bytes included in this tag. */
  abstract val size: Int

  internal companion object {
    /**
     * Parses a [Tag] from the given byte array using the control byte to determine the size of the
     * tag.
     *
     * @param controlByte the control byte for the element whose tag is being parsed
     * @param startIndex the index within [bytes] at which the tag data starts
     * @param bytes the bytes of the TLV element
     * @throws IllegalStateException if the byte array is too short to include the required tag data
     */
    fun from(controlByte: Byte, startIndex: Int, bytes: ByteArray): Tag {
      return when (controlByte and TAG_MASK) {
        ANONYMOUS -> AnonymousTag
        CONTEXT_SPECIFIC -> {
          ContextSpecificTag(checkBytes(startIndex, 1, bytes).first().toUByte().toInt())
        }
        COMMON_PROFILE_2 -> {
          CommonProfileTag(
            size = 2,
            tagNumber = checkBytes(startIndex, 2, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        COMMON_PROFILE_4 -> {
          CommonProfileTag(
            size = 4,
            tagNumber = checkBytes(startIndex, 4, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        IMPLICIT_PROFILE_2 -> {
          ImplicitProfileTag(
            size = 2,
            tagNumber = checkBytes(startIndex, 2, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        IMPLICIT_PROFILE_4 -> {
          ImplicitProfileTag(
            size = 4,
            tagNumber = checkBytes(startIndex, 4, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        FULLY_QUALIFIED_6 -> {
          FullyQualifiedTag(
            size = 6,
            vendorId = checkBytes(startIndex, 2, bytes).fromLittleEndianToLong().toUShort(),
            profileNumber =
              checkBytes(startIndex + 2, 2, bytes).fromLittleEndianToLong().toUShort(),
            tagNumber = checkBytes(startIndex + 4, 2, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        FULLY_QUALIFIED_8 -> {
          FullyQualifiedTag(
            size = 8,
            vendorId = checkBytes(startIndex, 2, bytes).fromLittleEndianToLong().toUShort(),
            profileNumber =
              checkBytes(startIndex + 2, 2, bytes).fromLittleEndianToLong().toUShort(),
            tagNumber = checkBytes(startIndex + 4, 4, bytes).fromLittleEndianToLong().toUInt()
          )
        }
        else -> throw IllegalArgumentException("Invalid control byte $controlByte")
      }
    }

    /**
     * Encode control byte and a tag as a TLV byte array from a [Tag] object.
     *
     * @param encodedType the partially encoded control byte with element type information
     * @param tag the tag of the encoded element
     * @throws IllegalStateException if the byte array is too short to include the required tag data
     */
    fun encode(encodedType: Byte, tag: Tag): ByteArray {
      // Encode control byte
      val controlByte =
        encodedType or
          when (tag) {
            is AnonymousTag -> ANONYMOUS
            is ContextSpecificTag -> CONTEXT_SPECIFIC
            is CommonProfileTag -> if (tag.size == 2) COMMON_PROFILE_2 else COMMON_PROFILE_4
            is ImplicitProfileTag -> if (tag.size == 2) IMPLICIT_PROFILE_2 else IMPLICIT_PROFILE_4
            is FullyQualifiedTag -> if (tag.size == 6) FULLY_QUALIFIED_6 else FULLY_QUALIFIED_8
          }

      // Encode tag
      val encodedTag =
        when (tag) {
          is AnonymousTag -> byteArrayOf()
          is ContextSpecificTag -> {
            require(tag.tagNumber.toUInt() <= UByte.MAX_VALUE) {
              "Invalid tag value ${tag.tagNumber} for context specific tag"
            }
            byteArrayOf(tag.tagNumber.toByte())
          }
          is CommonProfileTag -> tag.tagNumber.toByteArrayLittleEndian(tag.size.toShort())
          is ImplicitProfileTag -> tag.tagNumber.toByteArrayLittleEndian(tag.size.toShort())
          is FullyQualifiedTag -> {
            tag.vendorId.toByteArrayLittleEndian(2) +
              tag.profileNumber.toByteArrayLittleEndian(2) +
              tag.tagNumber.toByteArrayLittleEndian((tag.size - 4).toShort())
          }
        }

      return byteArrayOf(controlByte) + encodedTag
    }

    private fun checkBytes(startIndex: Int, expectedBytes: Int, actualBytes: ByteArray): ByteArray {
      val remaining = actualBytes.size - startIndex
      if (expectedBytes > remaining) {
        throw IllegalStateException(
          "Invalid tag: Expected $expectedBytes but only $remaining bytes available at $startIndex"
        )
      }
      return actualBytes.sliceArray(startIndex until startIndex + expectedBytes)
    }
  }
}

/** An anonymous tag encoding no data. */
object AnonymousTag : Tag() {
  override val size: Int = 0
}

/** A context-specific tag including a tag number within a structure. */
data class ContextSpecificTag(val tagNumber: Int) : Tag() {
  override val size: Int = 1
}

/** A common-profile tag including a tag number within a structure. */
data class CommonProfileTag(override val size: Int, val tagNumber: UInt) : Tag()

/** An implicit-profile tag including a tag number within a structure. */
data class ImplicitProfileTag(override val size: Int, val tagNumber: UInt) : Tag()

/**
 * A fully-qualified tag including a vendor identifier, a profile number and a tag number within a
 * structure.
 */
data class FullyQualifiedTag(
  override val size: Int,
  val vendorId: UShort,
  val profileNumber: UShort,
  val tagNumber: UInt
) : Tag()
