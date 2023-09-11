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

class SwitchClusterInitialPressEvent(val newPosition: Int) {
  override fun toString(): String = buildString {
    append("SwitchClusterInitialPressEvent {\n")
    append("\tnewPosition : $newPosition\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_NEW_POSITION), newPosition)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NEW_POSITION = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SwitchClusterInitialPressEvent {
      tlvReader.enterStructure(tag)
      val newPosition = tlvReader.getInt(ContextSpecificTag(TAG_NEW_POSITION))

      tlvReader.exitContainer()

      return SwitchClusterInitialPressEvent(newPosition)
    }
  }
}
