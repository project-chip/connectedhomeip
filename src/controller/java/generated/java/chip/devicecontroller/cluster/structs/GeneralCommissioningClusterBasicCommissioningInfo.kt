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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GeneralCommissioningClusterBasicCommissioningInfo(
  val failSafeExpiryLengthSeconds: UInt,
  val maxCumulativeFailsafeSeconds: UInt,
) {
  override fun toString(): String = buildString {
    append("GeneralCommissioningClusterBasicCommissioningInfo {\n")
    append("\tfailSafeExpiryLengthSeconds : $failSafeExpiryLengthSeconds\n")
    append("\tmaxCumulativeFailsafeSeconds : $maxCumulativeFailsafeSeconds\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS), failSafeExpiryLengthSeconds)
      put(ContextSpecificTag(TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS), maxCumulativeFailsafeSeconds)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS = 0
    private const val TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): GeneralCommissioningClusterBasicCommissioningInfo {
      tlvReader.enterStructure(tlvTag)
      val failSafeExpiryLengthSeconds =
        tlvReader.getUInt(ContextSpecificTag(TAG_FAIL_SAFE_EXPIRY_LENGTH_SECONDS))
      val maxCumulativeFailsafeSeconds =
        tlvReader.getUInt(ContextSpecificTag(TAG_MAX_CUMULATIVE_FAILSAFE_SECONDS))

      tlvReader.exitContainer()

      return GeneralCommissioningClusterBasicCommissioningInfo(
        failSafeExpiryLengthSeconds,
        maxCumulativeFailsafeSeconds,
      )
    }
  }
}
