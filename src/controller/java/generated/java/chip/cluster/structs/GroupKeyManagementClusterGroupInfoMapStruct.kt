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

class GroupKeyManagementClusterGroupInfoMapStruct (
    val groupId: Int,
    val endpoints: List<Int>,
    val groupName: Optional<String>,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("GroupKeyManagementClusterGroupInfoMapStruct {\n")
    builder.append("\tgroupId : $groupId\n")
    builder.append("\tendpoints : $endpoints\n")
    builder.append("\tgroupName : $groupName\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(1), groupId)
    tlvWriter.startList(ContextSpecificTag(2))
      val iter_endpoints = endpoints.iterator()
      while(iter_endpoints.hasNext()) {
        val next = iter_endpoints.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    if (groupName.isPresent) {
      val opt_groupName = groupName.get()
      tlvWriter.put(ContextSpecificTag(3), opt_groupName)
    }
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : GroupKeyManagementClusterGroupInfoMapStruct {
      tlvReader.enterStructure(tag)
      val groupId: Int = tlvReader.getInt(ContextSpecificTag(1))
      val endpoints: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val groupName: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return GroupKeyManagementClusterGroupInfoMapStruct(groupId, endpoints, groupName, fabricIndex)
    }
  }
}
