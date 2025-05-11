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

class JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct(
  val nodeID: ULong,
  val endpointID: UShort,
  val listID: UShort,
  val binding: JointFabricDatastoreClusterDatastoreBindingTargetStruct,
  val statusEntry: JointFabricDatastoreClusterDatastoreStatusEntryStruct,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpointID : $endpointID\n")
    append("\tlistID : $listID\n")
    append("\tbinding : $binding\n")
    append("\tstatusEntry : $statusEntry\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT_ID), endpointID)
      put(ContextSpecificTag(TAG_LIST_ID), listID)
      binding.toTlv(ContextSpecificTag(TAG_BINDING), this)
      statusEntry.toTlv(ContextSpecificTag(TAG_STATUS_ENTRY), this)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 0
    private const val TAG_ENDPOINT_ID = 1
    private const val TAG_LIST_ID = 2
    private const val TAG_BINDING = 3
    private const val TAG_STATUS_ENTRY = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val endpointID = tlvReader.getUShort(ContextSpecificTag(TAG_ENDPOINT_ID))
      val listID = tlvReader.getUShort(ContextSpecificTag(TAG_LIST_ID))
      val binding =
        JointFabricDatastoreClusterDatastoreBindingTargetStruct.fromTlv(
          ContextSpecificTag(TAG_BINDING),
          tlvReader,
        )
      val statusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntryStruct.fromTlv(
          ContextSpecificTag(TAG_STATUS_ENTRY),
          tlvReader,
        )
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreEndpointBindingEntryStruct(
        nodeID,
        endpointID,
        listID,
        binding,
        statusEntry,
        fabricIndex,
      )
    }
  }
}
