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

class ContentLauncherClusterParameterStruct (
    val type: Int,
    val value: String,
    val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ContentLauncherClusterParameterStruct {\n")
    builder.append("\ttype : $type\n")
    builder.append("\tvalue : $value\n")
    builder.append("\texternalIDList : $externalIDList\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_TYPE), type)
    tlvWriter.put(ContextSpecificTag(TAG_VALUE), value)
    if (externalIDList.isPresent) {
      val optexternalIDList = externalIDList.get()
      tlvWriter.startList(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
      val iteroptexternalIDList = optexternalIDList.iterator()
      while(iteroptexternalIDList.hasNext()) {
        val next = iteroptexternalIDList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_TYPE = 0
    private const val TAG_VALUE = 1
    private const val TAG_EXTERNAL_I_D_LIST = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterParameterStruct {
      tlvReader.enterStructure(tag)
      val type: Int = tlvReader.getInt(ContextSpecificTag(TAG_TYPE))
      val value: String = tlvReader.getString(ContextSpecificTag(TAG_VALUE))
      val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>> = try {
      Optional.of(mutableListOf<ContentLauncherClusterAdditionalInfoStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
      while(true) {
        try {
          this.add(ContentLauncherClusterAdditionalInfoStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ContentLauncherClusterParameterStruct(type, value, externalIDList)
    }
  }
}
