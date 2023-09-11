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
import java.util.Optional

class ChannelClusterLineupInfoStruct(
  val operatorName: String,
  val lineupName: Optional<String>,
  val postalCode: Optional<String>,
  val lineupInfoType: Int
) {
  override fun toString(): String = buildString {
    append("ChannelClusterLineupInfoStruct {\n")
    append("\toperatorName : $operatorName\n")
    append("\tlineupName : $lineupName\n")
    append("\tpostalCode : $postalCode\n")
    append("\tlineupInfoType : $lineupInfoType\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_OPERATOR_NAME), operatorName)
      if (lineupName.isPresent) {
        val optlineupName = lineupName.get()
        put(ContextSpecificTag(TAG_LINEUP_NAME), optlineupName)
      }
      if (postalCode.isPresent) {
        val optpostalCode = postalCode.get()
        put(ContextSpecificTag(TAG_POSTAL_CODE), optpostalCode)
      }
      put(ContextSpecificTag(TAG_LINEUP_INFO_TYPE), lineupInfoType)
      endStructure()
    }
  }

  companion object {
    private const val TAG_OPERATOR_NAME = 0
    private const val TAG_LINEUP_NAME = 1
    private const val TAG_POSTAL_CODE = 2
    private const val TAG_LINEUP_INFO_TYPE = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ChannelClusterLineupInfoStruct {
      tlvReader.enterStructure(tag)
      val operatorName = tlvReader.getString(ContextSpecificTag(TAG_OPERATOR_NAME))
      val lineupName =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LINEUP_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LINEUP_NAME)))
        } else {
          Optional.empty()
        }
      val postalCode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSTAL_CODE))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_POSTAL_CODE)))
        } else {
          Optional.empty()
        }
      val lineupInfoType = tlvReader.getInt(ContextSpecificTag(TAG_LINEUP_INFO_TYPE))

      tlvReader.exitContainer()

      return ChannelClusterLineupInfoStruct(operatorName, lineupName, postalCode, lineupInfoType)
    }
  }
}
