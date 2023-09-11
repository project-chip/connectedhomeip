/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.onboardingpayload

object Verhoeff {
  fun dihedralMultiply(x: Int, y: Int, n: Int): Int {
    val n2 = n * 2
    var newX = x % n2
    var newY = y % n2
    if (newX < n) {
      if (newY < n) return (newX + newY) % n
      return ((newX + (newY - n)) % n) + n
    }
    if (newY < n) return ((n + (newX - n) - newY) % n) + n
    return (n + (newX - n) - (newY - n)) % n
  }

  fun dihedralInvert(value: Int, n: Int): Int {
    if (value > 0 && value < n) return n - value
    return value
  }

  fun permute(value: Int, permTable: ByteArray, permTableLen: Int, iterCount: Int): Int {
    var newValue = value % permTableLen
    if (iterCount == 0) return newValue
    return permute(permTable[newValue].toInt(), permTable, permTableLen, iterCount - 1)
  }
}
