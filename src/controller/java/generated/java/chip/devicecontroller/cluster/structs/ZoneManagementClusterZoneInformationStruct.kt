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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ZoneManagementClusterZoneInformationStruct(
  val zoneID: UInt,
  val zoneType: UInt,
  val zoneSource: UInt,
  val twoDCartesianZone: Optional<ZoneManagementClusterTwoDCartesianZoneStruct>,
  val nodeID: Optional<ULong>,
  val endpointID: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ZoneManagementClusterZoneInformationStruct {\n")
    append("\tzoneID : $zoneID\n")
    append("\tzoneType : $zoneType\n")
    append("\tzoneSource : $zoneSource\n")
    append("\ttwoDCartesianZone : $twoDCartesianZone\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpointID : $endpointID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ZONE_ID), zoneID)
      put(ContextSpecificTag(TAG_ZONE_TYPE), zoneType)
      put(ContextSpecificTag(TAG_ZONE_SOURCE), zoneSource)
      if (twoDCartesianZone.isPresent) {
        val opttwoDCartesianZone = twoDCartesianZone.get()
        opttwoDCartesianZone.toTlv(ContextSpecificTag(TAG_TWO_D_CARTESIAN_ZONE), this)
      }
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
    private const val TAG_ZONE_ID = 0
    private const val TAG_ZONE_TYPE = 1
    private const val TAG_ZONE_SOURCE = 2
    private const val TAG_TWO_D_CARTESIAN_ZONE = 3
    private const val TAG_NODE_ID = 4
    private const val TAG_ENDPOINT_ID = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ZoneManagementClusterZoneInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val zoneID = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_ID))
      val zoneType = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_TYPE))
      val zoneSource = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_SOURCE))
      val twoDCartesianZone =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TWO_D_CARTESIAN_ZONE))) {
          Optional.of(
            ZoneManagementClusterTwoDCartesianZoneStruct.fromTlv(
              ContextSpecificTag(TAG_TWO_D_CARTESIAN_ZONE),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val nodeID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID)))
        } else {
          Optional.empty()
        }
      val endpointID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_ID)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ZoneManagementClusterZoneInformationStruct(
        zoneID,
        zoneType,
        zoneSource,
        twoDCartesianZone,
        nodeID,
        endpointID,
      )
    }
  }
}
