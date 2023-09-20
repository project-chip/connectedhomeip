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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class GeneralCommissioningClusterBasicCommissioningInfo(
  val failSafeExpiryLengthSeconds: Int,
  val maxCumulativeFailsafeSeconds: Int
) {
  override fun toString(): String = buildString {
    append("GeneralCommissioningClusterBasicCommissioningInfo {\n")
    append("\tfailSafeExpiryLengthSeconds : $failSafeExpiryLengthSeconds\n")
    append("\tmaxCumulativeFailsafeSeconds : $maxCumulativeFailsafeSeconds\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS), failSafeExpiryLengthSeconds)
      put(ContextSpecificTag(TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS), maxCumulativeFailsafeSeconds)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS = 0
    private const val TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): GeneralCommissioningClusterBasicCommissioningInfo {
      tlvReader.enterStructure(tag)
      val failSafeExpiryLengthSeconds =
        tlvReader.getInt(ContextSpecificTag(TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS))
      val maxCumulativeFailsafeSeconds =
        tlvReader.getInt(ContextSpecificTag(TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS))

      tlvReader.exitContainer()

      return GeneralCommissioningClusterBasicCommissioningInfo(
        failSafeExpiryLengthSeconds,
        maxCumulativeFailsafeSeconds
      )
    }
  }
}
