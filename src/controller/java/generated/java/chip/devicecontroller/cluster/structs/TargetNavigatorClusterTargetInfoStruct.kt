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

class TargetNavigatorClusterTargetInfoStruct(val identifier: Int, val name: String) {
  override fun toString(): String = buildString {
    append("TargetNavigatorClusterTargetInfoStruct {\n")
    append("\tidentifier : $identifier\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_IDENTIFIER), identifier)
      put(ContextSpecificTag(TAG_NAME), name)
      endStructure()
    }
  }

  companion object {
    private const val TAG_IDENTIFIER = 0
    private const val TAG_NAME = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TargetNavigatorClusterTargetInfoStruct {
      tlvReader.enterStructure(tag)
      val identifier = tlvReader.getInt(ContextSpecificTag(TAG_IDENTIFIER))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))

      tlvReader.exitContainer()

      return TargetNavigatorClusterTargetInfoStruct(identifier, name)
    }
  }
}
