/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
