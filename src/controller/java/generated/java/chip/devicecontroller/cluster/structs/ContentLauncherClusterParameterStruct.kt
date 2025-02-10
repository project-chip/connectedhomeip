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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterParameterStruct(
  val type: UInt,
  val value: String,
  val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>>,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterParameterStruct {\n")
    append("\ttype : $type\n")
    append("\tvalue : $value\n")
    append("\texternalIDList : $externalIDList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TYPE), type)
      put(ContextSpecificTag(TAG_VALUE), value)
      if (externalIDList.isPresent) {
        val optexternalIDList = externalIDList.get()
        startArray(ContextSpecificTag(TAG_EXTERNAL_ID_LIST))
        for (item in optexternalIDList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TYPE = 0
    private const val TAG_VALUE = 1
    private const val TAG_EXTERNAL_ID_LIST = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterParameterStruct {
      tlvReader.enterStructure(tlvTag)
      val type = tlvReader.getUInt(ContextSpecificTag(TAG_TYPE))
      val value = tlvReader.getString(ContextSpecificTag(TAG_VALUE))
      val externalIDList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTERNAL_ID_LIST))) {
          Optional.of(
            buildList<ContentLauncherClusterAdditionalInfoStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_EXTERNAL_ID_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(ContentLauncherClusterAdditionalInfoStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterParameterStruct(type, value, externalIDList)
    }
  }
}
