/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class GroupKeyManagementClusterGroupKeyMapStruct(
  val groupId: Int,
  val groupKeySetID: Int,
  val fabricIndex: Int
) {
  override fun toString(): String = buildString {
    append("GroupKeyManagementClusterGroupKeyMapStruct {\n")
    append("\tgroupId : $groupId\n")
    append("\tgroupKeySetID : $groupKeySetID\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupId)
      put(ContextSpecificTag(TAG_GROUP_KEY_SET_I_D), groupKeySetID)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_GROUP_ID = 1
    private const val TAG_GROUP_KEY_SET_I_D = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader): GroupKeyManagementClusterGroupKeyMapStruct {
      tlvReader.enterStructure(tag)
      val groupId = tlvReader.getInt(ContextSpecificTag(TAG_GROUP_ID))
      val groupKeySetID = tlvReader.getInt(ContextSpecificTag(TAG_GROUP_KEY_SET_I_D))
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return GroupKeyManagementClusterGroupKeyMapStruct(groupId, groupKeySetID, fabricIndex)
    }
  }
}
