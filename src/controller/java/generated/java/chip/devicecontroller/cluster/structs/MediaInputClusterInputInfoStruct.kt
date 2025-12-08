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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class MediaInputClusterInputInfoStruct(
  val index: UInt,
  val inputType: UInt,
  val name: String,
  val description: String,
) {
  override fun toString(): String = buildString {
    append("MediaInputClusterInputInfoStruct {\n")
    append("\tindex : $index\n")
    append("\tinputType : $inputType\n")
    append("\tname : $name\n")
    append("\tdescription : $description\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_INDEX), index)
      put(ContextSpecificTag(TAG_INPUT_TYPE), inputType)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_DESCRIPTION), description)
      endStructure()
    }
  }

  companion object {
    private const val TAG_INDEX = 0
    private const val TAG_INPUT_TYPE = 1
    private const val TAG_NAME = 2
    private const val TAG_DESCRIPTION = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaInputClusterInputInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val index = tlvReader.getUInt(ContextSpecificTag(TAG_INDEX))
      val inputType = tlvReader.getUInt(ContextSpecificTag(TAG_INPUT_TYPE))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val description = tlvReader.getString(ContextSpecificTag(TAG_DESCRIPTION))

      tlvReader.exitContainer()

      return MediaInputClusterInputInfoStruct(index, inputType, name, description)
    }
  }
}
