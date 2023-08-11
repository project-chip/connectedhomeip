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
