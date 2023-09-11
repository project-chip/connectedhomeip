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

class UnitTestingClusterTestFabricScopedEventEvent(val fabricIndex: Int) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterTestFabricScopedEventEvent {\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterTestFabricScopedEventEvent {
      tlvReader.enterStructure(tag)
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return UnitTestingClusterTestFabricScopedEventEvent(fabricIndex)
    }
  }
}
