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

class ActionsClusterEndpointListStruct(
  val endpointListID: Int,
  val name: String,
  val type: Int,
  val endpoints: List<Int>
) {
  override fun toString(): String = buildString {
    append("ActionsClusterEndpointListStruct {\n")
    append("\tendpointListID : $endpointListID\n")
    append("\tname : $name\n")
    append("\ttype : $type\n")
    append("\tendpoints : $endpoints\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D), endpointListID)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_TYPE), type)
      startList(ContextSpecificTag(TAG_ENDPOINTS))
      for (item in endpoints.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENDPOINT_LIST_I_D = 0
    private const val TAG_NAME = 1
    private const val TAG_TYPE = 2
    private const val TAG_ENDPOINTS = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ActionsClusterEndpointListStruct {
      tlvReader.enterStructure(tag)
      val endpointListID = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val type = tlvReader.getInt(ContextSpecificTag(TAG_TYPE))
      val endpoints =
        buildList<Int> {
          tlvReader.enterList(ContextSpecificTag(TAG_ENDPOINTS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ActionsClusterEndpointListStruct(endpointListID, name, type, endpoints)
    }
  }
}
