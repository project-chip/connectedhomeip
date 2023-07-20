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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ChannelClusterLineupInfoStruct (
    val operatorName: String,
    val lineupName: Optional<String>,
    val postalCode: Optional<String>,
    val lineupInfoType: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ChannelClusterLineupInfoStruct {\n")
    builder.append("\toperatorName : $operatorName\n")
    builder.append("\tlineupName : $lineupName\n")
    builder.append("\tpostalCode : $postalCode\n")
    builder.append("\tlineupInfoType : $lineupInfoType\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_OPERATOR_NAME), operatorName)
    if (lineupName.isPresent) {
      val optlineupName = lineupName.get()
      tlvWriter.put(ContextSpecificTag(TAG_LINEUP_NAME), optlineupName)
    }
    if (postalCode.isPresent) {
      val optpostalCode = postalCode.get()
      tlvWriter.put(ContextSpecificTag(TAG_POSTAL_CODE), optpostalCode)
    }
    tlvWriter.put(ContextSpecificTag(TAG_LINEUP_INFO_TYPE), lineupInfoType)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_OPERATOR_NAME = 0
    private const val TAG_LINEUP_NAME = 1
    private const val TAG_POSTAL_CODE = 2
    private const val TAG_LINEUP_INFO_TYPE = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ChannelClusterLineupInfoStruct {
      tlvReader.enterStructure(tag)
      val operatorName: String = tlvReader.getString(ContextSpecificTag(TAG_OPERATOR_NAME))
      val lineupName: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LINEUP_NAME)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val postalCode: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_POSTAL_CODE)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val lineupInfoType: Int = tlvReader.getInt(ContextSpecificTag(TAG_LINEUP_INFO_TYPE))
      
      tlvReader.exitContainer()

      return ChannelClusterLineupInfoStruct(operatorName, lineupName, postalCode, lineupInfoType)
    }
  }
}
