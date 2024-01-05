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

enum class OptionalQRCodeInfoType {
  TYPE_UNKNOWN,
  TYPE_STRING,
  TYPE_INT32,
  TYPE_INT64,
  TYPE_UINT32,
  TYPE_UINT64
}

open class OptionalQRCodeInfo {
  var tag: Int = 0
  var type: OptionalQRCodeInfoType = OptionalQRCodeInfoType.TYPE_UNKNOWN
  var data: String? = null
  var int32: Int = 0

  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (other !is OptionalQRCodeInfo) return false

    return tag == other.tag && type == other.type && data == other.data && int32 == other.int32
  }

  override fun hashCode(): Int {
    var result = tag
    result = 31 * result + type.hashCode()
    result = 31 * result + (data?.hashCode() ?: 0)
    result = 31 * result + int32
    return result
  }
}

class OptionalQRCodeInfoExtension : OptionalQRCodeInfo() {
  var int64: Long = 0
  var uint32: UInt = 0u
  var uint64: ULong = 0u

  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    if (!super.equals(other)) return false
    if (other !is OptionalQRCodeInfoExtension) return false

    return int64 == other.int64 && uint32 == other.uint32 && uint64 == other.uint64
  }

  override fun toString(): String {
    return "OptionalQRCodeInfoExtension(" +
      "tag=$tag, " +
      "type=$type, " +
      "data=$data, " +
      "int32=$int32, " +
      "int64=$int64, " +
      "uint32=$uint32, " +
      "uint64=$uint64" +
      ")"
  }

  override fun hashCode(): Int {
    var result = super.hashCode()
    result = 31 * result + int64.hashCode()
    result = 31 * result + uint32.hashCode()
    result = 31 * result + uint64.hashCode()
    return result
  }
}
