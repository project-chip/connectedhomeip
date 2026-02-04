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
  val endpoints: Optional<List<UShort>>,
  val keySetID: UShort,
  val hasAuxiliaryACL: Optional<Boolean>,
  val mcastAddrPolicy: UByte,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("GroupcastClusterMembershipStruct {\n")
    append("\tgroupID : $groupID\n")
    append("\tendpoints : $endpoints\n")
    append("\tkeySetID : $keySetID\n")
    append("\thasAuxiliaryACL : $hasAuxiliaryACL\n")
    append("\tmcastAddrPolicy : $mcastAddrPolicy\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupID)
      if (endpoints.isPresent) {
        val optendpoints = endpoints.get()
        startArray(ContextSpecificTag(TAG_ENDPOINTS))
        for (item in optendpoints.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      }
      put(ContextSpecificTag(TAG_KEY_SET_ID), keySetID)
      if (hasAuxiliaryACL.isPresent) {
        val opthasAuxiliaryACL = hasAuxiliaryACL.get()
        put(ContextSpecificTag(TAG_HAS_AUXILIARY_ACL), opthasAuxiliaryACL)
      }
      put(ContextSpecificTag(TAG_MCAST_ADDR_POLICY), mcastAddrPolicy)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_GROUP_ID = 0
    private const val TAG_ENDPOINTS = 1
    private const val TAG_KEY_SET_ID = 2
    private const val TAG_HAS_AUXILIARY_ACL = 3
    private const val TAG_MCAST_ADDR_POLICY = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GroupcastClusterMembershipStruct {
      tlvReader.enterStructure(tlvTag)
      val groupID = tlvReader.getUShort(ContextSpecificTag(TAG_GROUP_ID))
      val endpoints =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINTS))) {
          Optional.of(
            buildList<UShort> {
              tlvReader.enterArray(ContextSpecificTag(TAG_ENDPOINTS))
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUShort(AnonymousTag))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val keySetID = tlvReader.getUShort(ContextSpecificTag(TAG_KEY_SET_ID))
      val hasAuxiliaryACL =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_HAS_AUXILIARY_ACL))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_HAS_AUXILIARY_ACL)))
        } else {
          Optional.empty()
        }
      val mcastAddrPolicy = tlvReader.getUByte(ContextSpecificTag(TAG_MCAST_ADDR_POLICY))
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return GroupcastClusterMembershipStruct(
        groupID,
        endpoints,
        keySetID,
        hasAuxiliaryACL,
        mcastAddrPolicy,
        fabricIndex,
      )
    }
  }
}
