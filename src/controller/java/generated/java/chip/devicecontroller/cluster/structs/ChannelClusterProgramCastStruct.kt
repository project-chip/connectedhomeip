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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ChannelClusterProgramCastStruct (
    val name: String,
    val role: String) {
  override fun toString(): String  = buildString {
    append("ChannelClusterProgramCastStruct {\n")
    append("\tname : $name\n")
    append("\trole : $role\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_ROLE), role)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_ROLE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ChannelClusterProgramCastStruct {
      tlvReader.enterStructure(tlvTag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val role = tlvReader.getString(ContextSpecificTag(TAG_ROLE))
      
      tlvReader.exitContainer()

      return ChannelClusterProgramCastStruct(name, role)
    }
  }
}
