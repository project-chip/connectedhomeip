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
