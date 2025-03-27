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

class JointFabricDatastoreClusterDatastoreGroupInformationEntry(
  val groupID: ULong,
  val friendlyName: String,
  val groupKeySetID: UInt,
  val groupCAT: UInt,
  val groupCATVersion: UInt,
  val groupPermission: UInt,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreGroupInformationEntry {\n")
    append("\tgroupID : $groupID\n")
    append("\tfriendlyName : $friendlyName\n")
    append("\tgroupKeySetID : $groupKeySetID\n")
    append("\tgroupCAT : $groupCAT\n")
    append("\tgroupCATVersion : $groupCATVersion\n")
    append("\tgroupPermission : $groupPermission\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupID)
      put(ContextSpecificTag(TAG_FRIENDLY_NAME), friendlyName)
      put(ContextSpecificTag(TAG_GROUP_KEY_SET_ID), groupKeySetID)
      put(ContextSpecificTag(TAG_GROUP_CAT), groupCAT)
      put(ContextSpecificTag(TAG_GROUP_CAT_VERSION), groupCATVersion)
      put(ContextSpecificTag(TAG_GROUP_PERMISSION), groupPermission)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_GROUP_ID = 0
    private const val TAG_FRIENDLY_NAME = 1
    private const val TAG_GROUP_KEY_SET_ID = 2
    private const val TAG_GROUP_CAT = 3
    private const val TAG_GROUP_CAT_VERSION = 4
    private const val TAG_GROUP_PERMISSION = 5
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreGroupInformationEntry {
      tlvReader.enterStructure(tlvTag)
      val groupID = tlvReader.getULong(ContextSpecificTag(TAG_GROUP_ID))
      val friendlyName = tlvReader.getString(ContextSpecificTag(TAG_FRIENDLY_NAME))
      val groupKeySetID = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_KEY_SET_ID))
      val groupCAT = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_CAT))
      val groupCATVersion = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_CAT_VERSION))
      val groupPermission = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_PERMISSION))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreGroupInformationEntry(
        groupID,
        friendlyName,
        groupKeySetID,
        groupCAT,
        groupCATVersion,
        groupPermission,
        fabricIndex,
      )
    }
  }
}
