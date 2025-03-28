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

class JointFabricDatastoreClusterDatastoreBindingEntry(
  val listID: UInt,
  val statusEntry: JointFabricDatastoreClusterDatastoreStatusEntry,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreBindingEntry {\n")
    append("\tlistID : $listID\n")
    append("\tstatusEntry : $statusEntry\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LIST_ID), listID)
      statusEntry.toTlv(ContextSpecificTag(TAG_STATUS_ENTRY), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_LIST_ID = 0
    private const val TAG_STATUS_ENTRY = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreBindingEntry {
      tlvReader.enterStructure(tlvTag)
      val listID = tlvReader.getUInt(ContextSpecificTag(TAG_LIST_ID))
      val statusEntry =
        JointFabricDatastoreClusterDatastoreStatusEntry.fromTlv(
          ContextSpecificTag(TAG_STATUS_ENTRY),
          tlvReader,
        )

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreBindingEntry(listID, statusEntry)
    }
  }
}
