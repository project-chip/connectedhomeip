/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.onboardingpayload

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
}

class OptionalQRCodeInfoExtension : OptionalQRCodeInfo() {
  var int64: Long = 0
  var uint32: Long = 0
  var uint64: Long = 0
}
