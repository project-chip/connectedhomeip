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

class BridgedDeviceBasicInformationClusterProductAppearanceStruct(
  val finish: Int,
  val primaryColor: Int?
) {
  override fun toString(): String = buildString {
    append("BridgedDeviceBasicInformationClusterProductAppearanceStruct {\n")
    append("\tfinish : $finish\n")
    append("\tprimaryColor : $primaryColor\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_FINISH), finish)
      if (primaryColor != null) {
        put(ContextSpecificTag(TAG_PRIMARY_COLOR), primaryColor)
      } else {
        putNull(ContextSpecificTag(TAG_PRIMARY_COLOR))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_FINISH = 0
    private const val TAG_PRIMARY_COLOR = 1

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): BridgedDeviceBasicInformationClusterProductAppearanceStruct {
      tlvReader.enterStructure(tag)
      val finish = tlvReader.getInt(ContextSpecificTag(TAG_FINISH))
      val primaryColor =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_PRIMARY_COLOR))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PRIMARY_COLOR))
          null
        }

      tlvReader.exitContainer()

      return BridgedDeviceBasicInformationClusterProductAppearanceStruct(finish, primaryColor)
    }
  }
}
