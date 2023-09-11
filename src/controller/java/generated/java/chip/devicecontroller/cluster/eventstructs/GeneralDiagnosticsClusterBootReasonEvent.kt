/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class GeneralDiagnosticsClusterBootReasonEvent(val bootReason: Int) {
  override fun toString(): String = buildString {
    append("GeneralDiagnosticsClusterBootReasonEvent {\n")
    append("\tbootReason : $bootReason\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_BOOT_REASON), bootReason)
      endStructure()
    }
  }

  companion object {
    private const val TAG_BOOT_REASON = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): GeneralDiagnosticsClusterBootReasonEvent {
      tlvReader.enterStructure(tag)
      val bootReason = tlvReader.getInt(ContextSpecificTag(TAG_BOOT_REASON))

      tlvReader.exitContainer()

      return GeneralDiagnosticsClusterBootReasonEvent(bootReason)
    }
  }
}
