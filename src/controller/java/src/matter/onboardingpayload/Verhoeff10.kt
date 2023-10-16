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

/** Implements Verhoeff's check-digit algorithm for base-10 strings. */
class Verhoeff10 {
  companion object {
    const val Base = 10
    const val PolygonSize = 5

    val sMultiplyTable =
      intArrayOf(
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        1,
        2,
        3,
        4,
        0,
        6,
        7,
        8,
        9,
        5,
        2,
        3,
        4,
        0,
        1,
        7,
        8,
        9,
        5,
        6,
        3,
        4,
        0,
        1,
        2,
        8,
        9,
        5,
        6,
        7,
        4,
        0,
        1,
        2,
        3,
        9,
        5,
        6,
        7,
        8,
        5,
        9,
        8,
        7,
        6,
        0,
        4,
        3,
        2,
        1,
        6,
        5,
        9,
        8,
        7,
        1,
        0,
        4,
        3,
        2,
        7,
        6,
        5,
        9,
        8,
        2,
        1,
        0,
        4,
        3,
        8,
        7,
        6,
        5,
        9,
        3,
        2,
        1,
        0,
        4,
        9,
        8,
        7,
        6,
        5,
        4,
        3,
        2,
        1,
        0
      )

    // Permutation table for the algorithm.
    val sPermTable = byteArrayOf(1, 5, 7, 6, 2, 8, 3, 0, 9, 4)

    // Compute a check character for a given string.
    fun computeCheckChar(str: String): Char {
      return computeCheckChar(str, str.length)
    }

    fun computeCheckChar(str: String, strLen: Int): Char {
      var c = 0
      for (i in 1..strLen) {
        val ch = str[strLen - i]
        val value = charToVal(ch)
        val p = Verhoeff.permute(value, sPermTable, Base, i)
        c = sMultiplyTable[c * Base + p]
      }
      c = Verhoeff.dihedralInvert(c, PolygonSize)
      return valToChar(c)
    }

    // Verify a check character against a given string.
    fun validateCheckChar(checkChar: Char, str: String): Boolean {
      return validateCheckChar(checkChar, str, str.length)
    }

    fun validateCheckChar(checkChar: Char, str: String, strLen: Int): Boolean {
      return computeCheckChar(str, strLen) == checkChar
    }

    // Verify a check character at the end of a given string.
    fun validateCheckChar(str: String): Boolean {
      return validateCheckChar(str, str.length)
    }

    fun validateCheckChar(str: String, strLen: Int): Boolean {
      if (strLen == 0) return false
      return validateCheckChar(str[strLen - 1], str, strLen - 1)
    }

    // Convert between a character and its corresponding value.
    fun charToVal(ch: Char): Int {
      if (ch in '0'..'9') {
        return ch - '0'
      } else {
        throw IllegalArgumentException("Input character must be a digit")
      }
    }

    private fun valToChar(value: Int): Char {
      if (value in 0..Base) {
        return ('0'.code + value).toChar()
      } else {
        throw IllegalArgumentException("Input value must be a digit")
      }
    }
  }

  private constructor()
}
