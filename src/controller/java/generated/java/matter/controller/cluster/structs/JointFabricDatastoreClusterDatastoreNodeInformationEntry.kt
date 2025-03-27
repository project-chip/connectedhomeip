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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class JointFabricDatastoreClusterDatastoreNodeInformationEntry(
  val nodeID: ULong,
  val friendlyName: String,
  val commissioningStatusEntry: JointFabricDatastoreClusterDatastoreStatusEntry,
  val nodeKeySetList: List<JointFabricDatastoreClusterDatastoreNodeKeyEntry>,
  val ACLList: List<JointFabricDatastoreClusterDatastoreACLEntry>,
  val endpointList: List<JointFabricDatastoreClusterDatastoreEndpointEntry>,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreNodeInformationEntry {\n")
    append("\tnodeID : $nodeID\n")
    append("\tfriendlyName : $friendlyName\n")
    append("\tcommissioningStatusEntry : $commissioningStatusEntry\n")
    append("\tnodeKeySetList : $nodeKeySetList\n")
    append("\tACLList : $ACLList\n")
    append("\tendpointList : $endpointList\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_FRIENDLY_NAME), friendlyName)
      commissioningStatusEntry.toTlv(ContextSpecificTag(TAG_COMMISSIONING_STATUS_ENTRY), this)
      startArray(ContextSpecificTag(TAG_NODE_KEY_SET_LIST))
      for (item in nodeKeySetList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_ACL_LIST))
      for (item in ACLList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_ENDPOINT_LIST))
      for (item in endpointList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 1
    private const val TAG_FRIENDLY_NAME = 2
    private const val TAG_COMMISSIONING_STATUS_ENTRY = 3
    private const val TAG_NODE_KEY_SET_LIST = 4
    private const val TAG_ACL_LIST = 5
    private const val TAG_ENDPOINT_LIST = 6
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreNodeInformationEntry {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val friendlyName = tlvReader.getString(ContextSpecificTag(TAG_FRIENDLY_NAME))
      val commissioningStatusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntry.fromTlv(
          ContextSpecificTag(TAG_COMMISSIONING_STATUS_ENTRY),
          tlvReader,
        )
      val nodeKeySetList =
        buildList<JointFabricDatastoreClusterDatastoreNodeKeyEntry> {
          tlvReader.enterArray(ContextSpecificTag(TAG_NODE_KEY_SET_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(JointFabricDatastoreClusterDatastoreNodeKeyEntry.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val ACLList =
        buildList<JointFabricDatastoreClusterDatastoreACLEntry> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ACL_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(JointFabricDatastoreClusterDatastoreACLEntry.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val endpointList =
        buildList<JointFabricDatastoreClusterDatastoreEndpointEntry> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ENDPOINT_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(JointFabricDatastoreClusterDatastoreEndpointEntry.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreNodeInformationEntry(
        nodeID,
        friendlyName,
        commissioningStatusEntry,
        nodeKeySetList,
        ACLList,
        endpointList,
        fabricIndex,
      )
    }
  }
}
