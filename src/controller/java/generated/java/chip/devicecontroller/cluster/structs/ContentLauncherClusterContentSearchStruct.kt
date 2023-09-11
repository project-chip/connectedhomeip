/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
