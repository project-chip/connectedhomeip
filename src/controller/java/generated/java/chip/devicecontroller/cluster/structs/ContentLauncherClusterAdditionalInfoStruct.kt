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

class ContentLauncherClusterAdditionalInfoStruct(val name: String, val value: String) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterAdditionalInfoStruct {\n")
    append("\tname : $name\n")
    append("\tvalue : $value\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_VALUE), value)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_VALUE = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ContentLauncherClusterAdditionalInfoStruct {
      tlvReader.enterStructure(tag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val value = tlvReader.getString(ContextSpecificTag(TAG_VALUE))

      tlvReader.exitContainer()

      return ContentLauncherClusterAdditionalInfoStruct(name, value)
    }
  }
}
