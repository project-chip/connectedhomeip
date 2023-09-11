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

class SwitchClusterMultiPressCompleteEvent(
  val previousPosition: Int,
  val totalNumberOfPressesCounted: Int
) {
  override fun toString(): String = buildString {
    append("SwitchClusterMultiPressCompleteEvent {\n")
    append("\tpreviousPosition : $previousPosition\n")
    append("\ttotalNumberOfPressesCounted : $totalNumberOfPressesCounted\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_PREVIOUS_POSITION), previousPosition)
      put(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED), totalNumberOfPressesCounted)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_POSITION = 0
    private const val TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SwitchClusterMultiPressCompleteEvent {
      tlvReader.enterStructure(tag)
      val previousPosition = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_POSITION))
      val totalNumberOfPressesCounted =
        tlvReader.getInt(ContextSpecificTag(TAG_TOTAL_NUMBER_OF_PRESSES_COUNTED))

      tlvReader.exitContainer()

      return SwitchClusterMultiPressCompleteEvent(previousPosition, totalNumberOfPressesCounted)
    }
  }
}
