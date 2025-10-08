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

class JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct(
  val nodeID: ULong,
  val friendlyName: String,
  val commissioningStatusEntry: JointFabricDatastoreClusterDatastoreStatusEntryStruct,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tfriendlyName : $friendlyName\n")
    append("\tcommissioningStatusEntry : $commissioningStatusEntry\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_FRIENDLY_NAME), friendlyName)
      commissioningStatusEntry.toTlv(ContextSpecificTag(TAG_COMMISSIONING_STATUS_ENTRY), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 1
    private const val TAG_FRIENDLY_NAME = 2
    private const val TAG_COMMISSIONING_STATUS_ENTRY = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val friendlyName = tlvReader.getString(ContextSpecificTag(TAG_FRIENDLY_NAME))
      val commissioningStatusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(
          ContextSpecificTag(TAG_COMMISSIONING_STATUS_ENTRY),
          tlvReader,
        )

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreNodeInformationEntryStruct(
        nodeID,
        friendlyName,
        commissioningStatusEntry,
      )
    }
  }
}
