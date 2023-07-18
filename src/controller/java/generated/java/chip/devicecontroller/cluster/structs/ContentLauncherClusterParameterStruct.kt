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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
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
    tlvWriter.put(ContextSpecificTag(0), type)
    tlvWriter.put(ContextSpecificTag(1), value)
    if (externalIDList.isPresent) {
      val opt_externalIDList = externalIDList.get()
      tlvWriter.startList(ContextSpecificTag(2))
      val iter_opt_externalIDList = opt_externalIDList.iterator()
      while(iter_opt_externalIDList.hasNext()) {
        val next = iter_opt_externalIDList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterParameterStruct {
      tlvReader.enterStructure(tag)
      val type: Int = tlvReader.getInt(ContextSpecificTag(0))
      val value: String = tlvReader.getString(ContextSpecificTag(1))
      val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>> = try {
      Optional.of(mutableListOf<ContentLauncherClusterAdditionalInfoStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
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
