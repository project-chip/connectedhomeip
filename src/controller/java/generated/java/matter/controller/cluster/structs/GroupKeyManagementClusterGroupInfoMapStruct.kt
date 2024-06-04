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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GroupKeyManagementClusterGroupInfoMapStruct(
  val groupId: UShort,
  val endpoints: List<UShort>,
  val groupName: Optional<String>,
  val fabricIndex: UByte
) {
  override fun toString(): String = buildString {
    append("GroupKeyManagementClusterGroupInfoMapStruct {\n")
    append("\tgroupId : $groupId\n")
    append("\tendpoints : $endpoints\n")
    append("\tgroupName : $groupName\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupId)
      startArray(ContextSpecificTag(TAG_ENDPOINTS))
      for (item in endpoints.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      if (groupName.isPresent) {
        val optgroupName = groupName.get()
        put(ContextSpecificTag(TAG_GROUP_NAME), optgroupName)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_GROUP_ID = 1
    private const val TAG_ENDPOINTS = 2
    private const val TAG_GROUP_NAME = 3
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GroupKeyManagementClusterGroupInfoMapStruct {
      tlvReader.enterStructure(tlvTag)
      val groupId = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_ID))
      val endpoints =
        buildList<UShort> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ENDPOINTS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUShort(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val groupName =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_GROUP_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_GROUP_NAME)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return GroupKeyManagementClusterGroupInfoMapStruct(groupId, endpoints, groupName, fabricIndex)
    }
  }
}
