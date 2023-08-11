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
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ContentLauncherClusterContentSearchStruct(
  val parameterList: List<ContentLauncherClusterParameterStruct>
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterContentSearchStruct {\n")
    append("\tparameterList : $parameterList\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      startList(ContextSpecificTag(TAG_PARAMETER_LIST))
      for (item in parameterList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_PARAMETER_LIST = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ContentLauncherClusterContentSearchStruct {
      tlvReader.enterStructure(tag)
      val parameterList =
        buildList<ContentLauncherClusterParameterStruct> {
          tlvReader.enterList(ContextSpecificTag(TAG_PARAMETER_LIST))
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
