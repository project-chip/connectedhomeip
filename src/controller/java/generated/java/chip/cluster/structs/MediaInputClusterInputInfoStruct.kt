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

class MediaInputClusterInputInfoStruct (
    val index: Int,
    val inputType: Int,
    val name: String,
    val description: String) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("MediaInputClusterInputInfoStruct {\n")
    builder.append("\tindex : $index\n")
    builder.append("\tinputType : $inputType\n")
    builder.append("\tname : $name\n")
    builder.append("\tdescription : $description\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), index)
    tlvWriter.put(ContextSpecificTag(1), inputType)
    tlvWriter.put(ContextSpecificTag(2), name)
    tlvWriter.put(ContextSpecificTag(3), description)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : MediaInputClusterInputInfoStruct {
      tlvReader.enterStructure(tag)
      val index: Int = tlvReader.getInt(ContextSpecificTag(0))
      val inputType: Int = tlvReader.getInt(ContextSpecificTag(1))
      val name: String = tlvReader.getString(ContextSpecificTag(2))
      val description: String = tlvReader.getString(ContextSpecificTag(3))
      
      tlvReader.exitContainer()

      return MediaInputClusterInputInfoStruct(index, inputType, name, description)
    }
  }
}
