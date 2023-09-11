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

class TimeSynchronizationClusterDSTStatusEvent(val DSTOffsetActive: Boolean) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterDSTStatusEvent {\n")
    append("\tDSTOffsetActive : $DSTOffsetActive\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_D_S_T_OFFSET_ACTIVE), DSTOffsetActive)
      endStructure()
    }
  }

  companion object {
    private const val TAG_D_S_T_OFFSET_ACTIVE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterDSTStatusEvent {
      tlvReader.enterStructure(tag)
      val DSTOffsetActive = tlvReader.getBoolean(ContextSpecificTag(TAG_D_S_T_OFFSET_ACTIVE))

      tlvReader.exitContainer()

      return TimeSynchronizationClusterDSTStatusEvent(DSTOffsetActive)
    }
  }
}
