/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ChannelClusterLineupInfoStruct(
  val operatorName: String,
  val lineupName: Optional<String>,
  val postalCode: Optional<String>,
  val lineupInfoType: UInt,
) {
  override fun toString(): String = buildString {
    append("ChannelClusterLineupInfoStruct {\n")
    append("\toperatorName : $operatorName\n")
    append("\tlineupName : $lineupName\n")
    append("\tpostalCode : $postalCode\n")
    append("\tlineupInfoType : $lineupInfoType\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterLineupInfoStruct {
      tlvReader.enterStructure(tlvTag)
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
      val lineupInfoType = tlvReader.getUInt(ContextSpecificTag(TAG_LINEUP_INFO_TYPE))

      tlvReader.exitContainer()

      return ChannelClusterLineupInfoStruct(operatorName, lineupName, postalCode, lineupInfoType)
    }
  }
}
