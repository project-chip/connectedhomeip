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

class ModeSelectClusterSemanticTagStruct(val mfgCode: Int, val value: Int) {
  override fun toString(): String = buildString {
    append("ModeSelectClusterSemanticTagStruct {\n")
    append("\tmfgCode : $mfgCode\n")
    append("\tvalue : $value\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_MFG_CODE), mfgCode)
      put(ContextSpecificTag(TAG_VALUE), value)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MFG_CODE = 0
    private const val TAG_VALUE = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ModeSelectClusterSemanticTagStruct {
      tlvReader.enterStructure(tag)
      val mfgCode = tlvReader.getInt(ContextSpecificTag(TAG_MFG_CODE))
      val value = tlvReader.getInt(ContextSpecificTag(TAG_VALUE))

      tlvReader.exitContainer()

      return ModeSelectClusterSemanticTagStruct(mfgCode, value)
    }
  }
}
