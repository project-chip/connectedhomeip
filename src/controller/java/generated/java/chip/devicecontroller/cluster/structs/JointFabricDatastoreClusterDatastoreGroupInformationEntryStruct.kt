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

class JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct(
  val groupID: ULong,
  val friendlyName: String,
  val groupKeySetID: UInt?,
  val groupCAT: UInt?,
  val groupCATVersion: UInt?,
  val groupPermission: UInt,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct {\n")
    append("\tgroupID : $groupID\n")
    append("\tfriendlyName : $friendlyName\n")
    append("\tgroupKeySetID : $groupKeySetID\n")
    append("\tgroupCAT : $groupCAT\n")
    append("\tgroupCATVersion : $groupCATVersion\n")
    append("\tgroupPermission : $groupPermission\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_GROUP_ID), groupID)
      put(ContextSpecificTag(TAG_FRIENDLY_NAME), friendlyName)
      if (groupKeySetID != null) {
        put(ContextSpecificTag(TAG_GROUP_KEY_SET_ID), groupKeySetID)
      } else {
        putNull(ContextSpecificTag(TAG_GROUP_KEY_SET_ID))
      }
      if (groupCAT != null) {
        put(ContextSpecificTag(TAG_GROUP_CAT), groupCAT)
      } else {
        putNull(ContextSpecificTag(TAG_GROUP_CAT))
      }
      if (groupCATVersion != null) {
        put(ContextSpecificTag(TAG_GROUP_CAT_VERSION), groupCATVersion)
      } else {
        putNull(ContextSpecificTag(TAG_GROUP_CAT_VERSION))
      }
      put(ContextSpecificTag(TAG_GROUP_PERMISSION), groupPermission)
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

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val groupID = tlvReader.getULong(ContextSpecificTag(TAG_GROUP_ID))
      val friendlyName = tlvReader.getString(ContextSpecificTag(TAG_FRIENDLY_NAME))
      val groupKeySetID =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_KEY_SET_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_GROUP_KEY_SET_ID))
          null
        }
      val groupCAT =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_CAT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_GROUP_CAT))
          null
        }
      val groupCATVersion =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_CAT_VERSION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_GROUP_CAT_VERSION))
          null
        }
      val groupPermission = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_PERMISSION))

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreGroupInformationEntryStruct(
        groupID,
        friendlyName,
        groupKeySetID,
        groupCAT,
        groupCATVersion,
        groupPermission,
      )
    }
  }
}
