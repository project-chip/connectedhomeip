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

class BooleanStateClusterStateChangeEvent(val stateValue: Boolean) {
  override fun toString(): String = buildString {
    append("BooleanStateClusterStateChangeEvent {\n")
    append("\tstateValue : $stateValue\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_STATE_VALUE), stateValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_STATE_VALUE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): BooleanStateClusterStateChangeEvent {
      tlvReader.enterStructure(tag)
      val stateValue = tlvReader.getBoolean(ContextSpecificTag(TAG_STATE_VALUE))

      tlvReader.exitContainer()

      return BooleanStateClusterStateChangeEvent(stateValue)
    }
  }
}
