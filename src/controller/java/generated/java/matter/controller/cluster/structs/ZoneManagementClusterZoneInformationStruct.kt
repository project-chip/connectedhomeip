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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ZoneManagementClusterZoneInformationStruct(
  val zoneID: UShort,
  val zoneType: UByte,
  val zoneSource: UByte,
  val twoDCartesianZone: Optional<ZoneManagementClusterTwoDCartesianZoneStruct>,
) {
  override fun toString(): String = buildString {
    append("ZoneManagementClusterZoneInformationStruct {\n")
    append("\tzoneID : $zoneID\n")
    append("\tzoneType : $zoneType\n")
    append("\tzoneSource : $zoneSource\n")
    append("\ttwoDCartesianZone : $twoDCartesianZone\n")
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
      endStructure()
    }
  }

  companion object {
    private const val TAG_ZONE_ID = 0
    private const val TAG_ZONE_TYPE = 1
    private const val TAG_ZONE_SOURCE = 2
    private const val TAG_TWO_D_CARTESIAN_ZONE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ZoneManagementClusterZoneInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val zoneID = tlvReader.getUShort(ContextSpecificTag(TAG_ZONE_ID))
      val zoneType = tlvReader.getUByte(ContextSpecificTag(TAG_ZONE_TYPE))
      val zoneSource = tlvReader.getUByte(ContextSpecificTag(TAG_ZONE_SOURCE))
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

      tlvReader.exitContainer()

      return ZoneManagementClusterZoneInformationStruct(
        zoneID,
        zoneType,
        zoneSource,
        twoDCartesianZone,
      )
    }
  }
}
