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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class BasicInformationClusterProductAppearanceStruct(val finish: UByte, val primaryColor: UByte?) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterProductAppearanceStruct {\n")
    append("\tfinish : $finish\n")
    append("\tprimaryColor : $primaryColor\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): BasicInformationClusterProductAppearanceStruct {
      tlvReader.enterStructure(tlvTag)
      val finish = tlvReader.getUByte(ContextSpecificTag(TAG_FINISH))
      val primaryColor =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_PRIMARY_COLOR))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PRIMARY_COLOR))
          null
        }

      tlvReader.exitContainer()

      return BasicInformationClusterProductAppearanceStruct(finish, primaryColor)
    }
  }
}
