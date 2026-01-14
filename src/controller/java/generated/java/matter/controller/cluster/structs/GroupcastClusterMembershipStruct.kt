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

class GroupcastClusterMembershipStruct(
  val groupID: UShort,
  val endpoints: List<UShort>,
  val keyID: UInt,
  val hasAuxiliaryACL: Boolean,
  val expiringKeyID: Optional<UInt>,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("GroupcastClusterMembershipStruct {\n")
    append("\tgroupID : $groupID\n")
    append("\tendpoints : $endpoints\n")
    append("\tkeyID : $keyID\n")
    append("\thasAuxiliaryACL : $hasAuxiliaryACL\n")
    append("\texpiringKeyID : $expiringKeyID\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupID)
      startArray(ContextSpecificTag(TAG_ENDPOINTS))
      for (item in endpoints.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_KEY_ID), keyID)
      put(ContextSpecificTag(TAG_HAS_AUXILIARY_ACL), hasAuxiliaryACL)
      if (expiringKeyID.isPresent) {
        val optexpiringKeyID = expiringKeyID.get()
        put(ContextSpecificTag(TAG_EXPIRING_KEY_ID), optexpiringKeyID)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_GROUP_ID = 0
    private const val TAG_ENDPOINTS = 1
    private const val TAG_KEY_ID = 2
    private const val TAG_HAS_AUXILIARY_ACL = 3
    private const val TAG_EXPIRING_KEY_ID = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GroupcastClusterMembershipStruct {
      tlvReader.enterStructure(tlvTag)
      val groupID = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_ID))
      val endpoints =
        buildList<UShort> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ENDPOINTS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUShort(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val keyID = tlvReader.getUInt(ContextSpecificTag(TAG_KEY_ID))
      val hasAuxiliaryACL = tlvReader.getBoolean(ContextSpecificTag(TAG_HAS_AUXILIARY_ACL))
      val expiringKeyID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXPIRING_KEY_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_EXPIRING_KEY_ID)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return GroupcastClusterMembershipStruct(
        groupID,
        endpoints,
        keyID,
        hasAuxiliaryACL,
        expiringKeyID,
        fabricIndex,
      )
    }
  }
}
