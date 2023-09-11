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

class SwitchClusterMultiPressOngoingEvent(
  val newPosition: Int,
  val currentNumberOfPressesCounted: Int
) {
  override fun toString(): String = buildString {
    append("SwitchClusterMultiPressOngoingEvent {\n")
    append("\tnewPosition : $newPosition\n")
    append("\tcurrentNumberOfPressesCounted : $currentNumberOfPressesCounted\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_NEW_POSITION), newPosition)
      put(ContextSpecificTag(TAG_CURRENT_NUMBER_OF_PRESSES_COUNTED), currentNumberOfPressesCounted)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NEW_POSITION = 0
    private const val TAG_CURRENT_NUMBER_OF_PRESSES_COUNTED = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SwitchClusterMultiPressOngoingEvent {
      tlvReader.enterStructure(tag)
      val newPosition = tlvReader.getInt(ContextSpecificTag(TAG_NEW_POSITION))
      val currentNumberOfPressesCounted =
        tlvReader.getInt(ContextSpecificTag(TAG_CURRENT_NUMBER_OF_PRESSES_COUNTED))

      tlvReader.exitContainer()

      return SwitchClusterMultiPressOngoingEvent(newPosition, currentNumberOfPressesCounted)
    }
  }
}
