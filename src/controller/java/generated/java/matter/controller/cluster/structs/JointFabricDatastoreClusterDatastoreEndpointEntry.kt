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

class JointFabricDatastoreClusterDatastoreEndpointEntry(
  val endpointID: UShort,
  val nodeID: ULong,
  val friendlyName: String,
  val statusEntry: JointFabricDatastoreClusterDatastoreStatusEntry,
  val groupIDList: List<JointFabricDatastoreClusterDatastoreGroupIDEntry>,
  val bindingList: List<JointFabricDatastoreClusterDatastoreBindingEntry>,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreEndpointEntry {\n")
    append("\tendpointID : $endpointID\n")
    append("\tnodeID : $nodeID\n")
    append("\tfriendlyName : $friendlyName\n")
    append("\tstatusEntry : $statusEntry\n")
    append("\tgroupIDList : $groupIDList\n")
    append("\tbindingList : $bindingList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ENDPOINT_ID), endpointID)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_FRIENDLY_NAME), friendlyName)
      statusEntry.toTlv(ContextSpecificTag(TAG_STATUS_ENTRY), this)
      startArray(ContextSpecificTag(TAG_GROUP_ID_LIST))
      for (item in groupIDList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_BINDING_LIST))
      for (item in bindingList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENDPOINT_ID = 0
    private const val TAG_NODE_ID = 1
    private const val TAG_FRIENDLY_NAME = 2
    private const val TAG_STATUS_ENTRY = 3
    private const val TAG_GROUP_ID_LIST = 4
    private const val TAG_BINDING_LIST = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreEndpointEntry {
      tlvReader.enterStructure(tlvTag)
      val endpointID = tlvReader.getUShort(ContextSpecificTag(TAG_ENDPOINT_ID))
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val friendlyName = tlvReader.getString(ContextSpecificTag(TAG_FRIENDLY_NAME))
      val statusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntry.fromTlv(
          ContextSpecificTag(TAG_STATUS_ENTRY),
          tlvReader,
        )
      val groupIDList =
        buildList<JointFabricDatastoreClusterDatastoreGroupIDEntry> {
          tlvReader.enterArray(ContextSpecificTag(TAG_GROUP_ID_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(JointFabricDatastoreClusterDatastoreGroupIDEntry.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val bindingList =
        buildList<JointFabricDatastoreClusterDatastoreBindingEntry> {
          tlvReader.enterArray(ContextSpecificTag(TAG_BINDING_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(JointFabricDatastoreClusterDatastoreBindingEntry.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreEndpointEntry(
        endpointID,
        nodeID,
        friendlyName,
        statusEntry,
        groupIDList,
        bindingList,
      )
    }
  }
}
