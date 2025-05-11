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

class JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct(
  val nodeID: ULong,
  val endpointID: UInt,
  val groupID: UInt,
  val statusEntry: JointFabricDatastoreClusterDatastoreStatusEntryStruct,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpointID : $endpointID\n")
    append("\tgroupID : $groupID\n")
    append("\tstatusEntry : $statusEntry\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT_ID), endpointID)
      put(ContextSpecificTag(TAG_GROUP_ID), groupID)
      statusEntry.toTlv(ContextSpecificTag(TAG_STATUS_ENTRY), this)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 0
    private const val TAG_ENDPOINT_ID = 1
    private const val TAG_GROUP_ID = 2
    private const val TAG_STATUS_ENTRY = 3
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val endpointID = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_ID))
      val groupID = tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_ID))
      val statusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(
          ContextSpecificTag(TAG_STATUS_ENTRY),
          tlvReader,
        )
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreEndpointGroupIDEntryStruct(
        nodeID,
        endpointID,
        groupID,
        statusEntry,
        fabricIndex,
      )
    }
  }
}
