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

class ServiceAreaClusterMapStruct(val mapID: UInt, val name: String) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterMapStruct {\n")
    append("\tmapID : $mapID\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MAP_ID), mapID)
      put(ContextSpecificTag(TAG_NAME), name)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MAP_ID = 0
    private const val TAG_NAME = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterMapStruct {
      tlvReader.enterStructure(tlvTag)
      val mapID = tlvReader.getUInt(ContextSpecificTag(TAG_MAP_ID))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))

      tlvReader.exitContainer()

      return ServiceAreaClusterMapStruct(mapID, name)
    }
  }
}
