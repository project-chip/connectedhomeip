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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterContentSearchStruct(
  val parameterList: List<ContentLauncherClusterParameterStruct>
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterContentSearchStruct {\n")
    append("\tparameterList : $parameterList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_PARAMETER_LIST))
      for (item in parameterList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_PARAMETER_LIST = 0

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterContentSearchStruct {
      tlvReader.enterStructure(tlvTag)
      val parameterList =
        buildList<ContentLauncherClusterParameterStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_PARAMETER_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(ContentLauncherClusterParameterStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterContentSearchStruct(parameterList)
    }
  }
}
