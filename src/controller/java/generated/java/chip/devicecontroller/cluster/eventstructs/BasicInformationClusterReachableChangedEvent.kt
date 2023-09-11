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

class BasicInformationClusterReachableChangedEvent(val reachableNewValue: Boolean) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterReachableChangedEvent {\n")
    append("\treachableNewValue : $reachableNewValue\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_REACHABLE_NEW_VALUE), reachableNewValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_REACHABLE_NEW_VALUE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): BasicInformationClusterReachableChangedEvent {
      tlvReader.enterStructure(tag)
      val reachableNewValue = tlvReader.getBoolean(ContextSpecificTag(TAG_REACHABLE_NEW_VALUE))

      tlvReader.exitContainer()

      return BasicInformationClusterReachableChangedEvent(reachableNewValue)
    }
  }
}
