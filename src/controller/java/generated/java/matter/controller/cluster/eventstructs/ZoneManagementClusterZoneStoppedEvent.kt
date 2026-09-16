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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ZoneManagementClusterZoneStoppedEvent(
  val zone: UShort,
  val reason: UByte,
  val nodeID: Optional<ULong>,
  val endpointID: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("ZoneManagementClusterZoneStoppedEvent {\n")
    append("\tzone : $zone\n")
    append("\treason : $reason\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpointID : $endpointID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ZONE), zone)
      put(ContextSpecificTag(TAG_REASON), reason)
      if (nodeID.isPresent) {
        val optnodeID = nodeID.get()
        put(ContextSpecificTag(TAG_NODE_ID), optnodeID)
      }
      if (endpointID.isPresent) {
        val optendpointID = endpointID.get()
        put(ContextSpecificTag(TAG_ENDPOINT_ID), optendpointID)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ZONE = 0
    private const val TAG_REASON = 1
    private const val TAG_NODE_ID = 2
    private const val TAG_ENDPOINT_ID = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ZoneManagementClusterZoneStoppedEvent {
      tlvReader.enterStructure(tlvTag)
      val zone = tlvReader.getUShort(ContextSpecificTag(TAG_ZONE))
      val reason = tlvReader.getUByte(ContextSpecificTag(TAG_REASON))
      val nodeID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID)))
        } else {
          Optional.empty()
        }
      val endpointID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT_ID))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_ENDPOINT_ID)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ZoneManagementClusterZoneStoppedEvent(zone, reason, nodeID, endpointID)
    }
  }
}
