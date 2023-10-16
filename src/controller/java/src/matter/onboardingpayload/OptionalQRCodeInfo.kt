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
}

class OptionalQRCodeInfoExtension : OptionalQRCodeInfo() {
  var int64: Long = 0
  var uint32: Long = 0
  var uint64: Long = 0
}
