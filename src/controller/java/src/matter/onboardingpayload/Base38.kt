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

package matter.onboardingpayload

private val kCodes =
  charArrayOf(
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    '-',
    '.'
  )
private val kBase38CharactersNeededInNBytesChunk = byteArrayOf(2, 4, 5)
private val kRadix = kCodes.size.toByte()
private val kMaxBytesSingleChunkLen = 3

/*
 * Implements converting an array of bytes into a Base38 String.
 *
 * The encoding chosen is: treat every 3 bytes of input data as a little-endian
 * uint32_t, then div and mod that into 5 base38 characters, with the least-significant
 * encoding bits in the first character of the resulting string. If a number of bytes
 * is used that is not multiple of 3, then last 2 bytes are encoded to 4 base38 characters
 * or last 1 byte is encoded to 2 base38 characters. Algoritm considers worst case size
 * of bytes chunks and does not introduce code length optimization.
 *
 * The resulting size of the out_buf span will be the size of data written.
 */
fun base38Encode(inBuf: ByteArray, outBuf: CharArray): Unit {
  var inBufLen = inBuf.size
  var inIdx = 0
  var outIdx = 0

  while (inBufLen > 0) {
    var value = 0
    val bytesInChunk =
      if (inBufLen >= kMaxBytesSingleChunkLen) kMaxBytesSingleChunkLen else inBufLen

    for (byteIdx in 0 until bytesInChunk) {
      value += (inBuf[inIdx + byteIdx].toInt() and 0xFF) shl (8 * byteIdx)
    }
    inBufLen -= bytesInChunk
    inIdx += bytesInChunk

    val base38CharactersNeeded = kBase38CharactersNeededInNBytesChunk[bytesInChunk - 1].toByte()

    if ((outIdx + base38CharactersNeeded) > outBuf.size) {
      throw OnboardingPayloadException("Buffer is too small")
    }

    for (character in 0 until base38CharactersNeeded) {
      outBuf[outIdx++] = kCodes[value % kRadix]
      value /= kRadix.toInt()
    }
  }
}

/*
 * Returns size needed to store encoded string given number of input bytes.
 */
fun base38EncodedLength(numBytes: Int): Int {
  return (numBytes / 3) * 5 + (numBytes % 3) * 2
}

/** Implements converting a Base38 String into an array of bytes. */
fun base38Decode(base38: String): ArrayList<Byte> {
  val result = ArrayList<Byte>()
  var base38CharactersNumber = base38.length
  var decodedBase38Characters = 0

  while (base38CharactersNumber > 0) {
    val base38CharactersInChunk: Byte
    val bytesInDecodedChunk: Byte

    if (base38CharactersNumber >= kBase38CharactersNeededInNBytesChunk[2]) {
      base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[2]
      bytesInDecodedChunk = 3
    } else if (base38CharactersNumber == kBase38CharactersNeededInNBytesChunk[1].toInt()) {
      base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[1]
      bytesInDecodedChunk = 2
    } else if (base38CharactersNumber == kBase38CharactersNeededInNBytesChunk[0].toInt()) {
      base38CharactersInChunk = kBase38CharactersNeededInNBytesChunk[0]
      bytesInDecodedChunk = 1
    } else {
      throw OnboardingPayloadException("Invalid string length")
    }

    var value = 0
    for (i in base38CharactersInChunk downTo 1) {
      val v = decodeChar(base38[decodedBase38Characters + i - 1])
      if (v < 0) {
        throw OnboardingPayloadException("Invalid integer value")
      }
      value = value * kRadix + v
    }

    decodedBase38Characters += base38CharactersInChunk
    base38CharactersNumber -= base38CharactersInChunk.toInt()

    for (i in 0 until bytesInDecodedChunk) {
      result.add(value.toByte())
      value = value shr 8
    }

    if (value > 0) {
      throw OnboardingPayloadException("Invalid argument")
    }
  }

  return result
}

private fun decodeChar(c: Char): Byte {
  val kBogus: Byte = -1
  val decodes =
    byteArrayOf(
      36, // '-', =45
      37, // '.', =46
      kBogus, // '/', =47
      0, // '0', =48
      1, // '1', =49
      2, // '2', =50
      3, // '3', =51
      4, // '4', =52
      5, // '5', =53
      6, // '6', =54
      7, // '7', =55
      8, // '8', =56
      9, // '9', =57
      kBogus, // ':', =58
      kBogus, // ';', =59
      kBogus, // '<', =50
      kBogus, // '=', =61
      kBogus, // '>', =62
      kBogus, // '?', =63
      kBogus, // '@', =64
      10, // 'A', =65
      11, // 'B', =66
      12, // 'C', =67
      13, // 'D', =68
      14, // 'E', =69
      15, // 'F', =70
      16, // 'G', =71
      17, // 'H', =72
      18, // 'I', =73
      19, // 'J', =74
      20, // 'K', =75
      21, // 'L', =76
      22, // 'M', =77
      23, // 'N', =78
      24, // 'O', =79
      25, // 'P', =80
      26, // 'Q', =81
      27, // 'R', =82
      28, // 'S', =83
      29, // 'T', =84
      30, // 'U', =85
      31, // 'V', =86
      32, // 'W', =87
      33, // 'X', =88
      34, // 'Y', =89
      35 // 'Z', =90
    )

  if (c < '-' || c > 'Z') {
    throw OnboardingPayloadException("Invalid character: $c")
  }

  val v: Byte = decodes[c - '-']
  if (v == kBogus) {
    throw OnboardingPayloadException("Invalid integer value")
  }

  return v
}
