/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019-2023 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.tlv

/** Converts bytes in a Little Endian format into Long integer. */
internal fun ByteArray.fromLittleEndianToLong(isSigned: Boolean = false): Long =
  foldRightIndexed(0) { i, value, acc ->
    (acc shl 8) or (if (i == lastIndex && isSigned) value.toLong() else (value.toLong() and 0xFF))
  }

/** Converts Number into a byte array in a Little Endian format. */
internal fun Number.toByteArrayLittleEndian(numBytes: Short): ByteArray =
  toLong().toLittleEndianBytes(numBytes)

internal fun UByte.toByteArrayLittleEndian(numBytes: Short): ByteArray =
  toLong().toLittleEndianBytes(numBytes)

internal fun UShort.toByteArrayLittleEndian(numBytes: Short): ByteArray =
  toLong().toLittleEndianBytes(numBytes)

internal fun UInt.toByteArrayLittleEndian(numBytes: Short): ByteArray =
  toLong().toLittleEndianBytes(numBytes)

internal fun ULong.toByteArrayLittleEndian(numBytes: Short): ByteArray =
  toLong().toLittleEndianBytes(numBytes)

private fun Long.toLittleEndianBytes(numBytes: Short) =
  ByteArray(numBytes.toInt()) { i -> (this shr (8 * i)).toByte() }

internal fun signedIntSize(value: Long): Short {
  return when (value) {
    in Byte.MIN_VALUE..Byte.MAX_VALUE -> 1
    in Short.MIN_VALUE..Short.MAX_VALUE -> 2
    in Int.MIN_VALUE..Int.MAX_VALUE -> 4
    else -> 8
  }
}

internal fun unsignedIntSize(value: ULong): Short {
  return when {
    value <= UByte.MAX_VALUE -> 1
    value <= UShort.MAX_VALUE -> 2
    value <= UInt.MAX_VALUE -> 4
    else -> 8
  }
}

internal fun Byte.toBinary(): String = Integer.toBinaryString(toInt() and 0xFF)
