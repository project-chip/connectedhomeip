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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class JointFabricDatastoreClusterDatastoreACLEntryStruct(
  val nodeID: ULong,
  val listID: UShort,
  val ACLEntry: JointFabricDatastoreClusterDatastoreAccessControlEntryStruct,
  val statusEntry: JointFabricDatastoreClusterDatastoreStatusEntryStruct,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreACLEntryStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tlistID : $listID\n")
    append("\tACLEntry : $ACLEntry\n")
    append("\tstatusEntry : $statusEntry\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_LIST_ID), listID)
      ACLEntry.toTlv(ContextSpecificTag(TAG_ACL_ENTRY), this)
      statusEntry.toTlv(ContextSpecificTag(TAG_STATUS_ENTRY), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 0
    private const val TAG_LIST_ID = 1
    private const val TAG_ACL_ENTRY = 2
    private const val TAG_STATUS_ENTRY = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreACLEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val listID = tlvReader.getUShort(ContextSpecificTag(TAG_LIST_ID))
      val ACLEntry =
        JointFabricDatastoreClusterDatastoreAccessControlEntryStruct.fromTlv(
          ContextSpecificTag(TAG_ACL_ENTRY),
          tlvReader,
        )
      val statusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(
          ContextSpecificTag(TAG_STATUS_ENTRY),
          tlvReader,
        )

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreACLEntryStruct(
        nodeID,
        listID,
        ACLEntry,
        statusEntry,
      )
    }
  }
}
