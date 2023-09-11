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

class SwitchClusterShortReleaseEvent(val previousPosition: Int) {
  override fun toString(): String = buildString {
    append("SwitchClusterShortReleaseEvent {\n")
    append("\tpreviousPosition : $previousPosition\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_PREVIOUS_POSITION), previousPosition)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_POSITION = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SwitchClusterShortReleaseEvent {
      tlvReader.enterStructure(tag)
      val previousPosition = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_POSITION))

      tlvReader.exitContainer()

      return SwitchClusterShortReleaseEvent(previousPosition)
    }
  }
}
