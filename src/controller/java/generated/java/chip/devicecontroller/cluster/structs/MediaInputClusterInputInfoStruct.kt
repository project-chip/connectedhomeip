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

class MediaInputClusterInputInfoStruct(
  val index: Int,
  val inputType: Int,
  val name: String,
  val description: String
) {
  override fun toString(): String = buildString {
    append("MediaInputClusterInputInfoStruct {\n")
    append("\tindex : $index\n")
    append("\tinputType : $inputType\n")
    append("\tname : $name\n")
    append("\tdescription : $description\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_INDEX), index)
      put(ContextSpecificTag(TAG_INPUT_TYPE), inputType)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_DESCRIPTION), description)
      endStructure()
    }
  }

  companion object {
    private const val TAG_INDEX = 0
    private const val TAG_INPUT_TYPE = 1
    private const val TAG_NAME = 2
    private const val TAG_DESCRIPTION = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): MediaInputClusterInputInfoStruct {
      tlvReader.enterStructure(tag)
      val index = tlvReader.getInt(ContextSpecificTag(TAG_INDEX))
      val inputType = tlvReader.getInt(ContextSpecificTag(TAG_INPUT_TYPE))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val description = tlvReader.getString(ContextSpecificTag(TAG_DESCRIPTION))

      tlvReader.exitContainer()

      return MediaInputClusterInputInfoStruct(index, inputType, name, description)
    }
  }
}
