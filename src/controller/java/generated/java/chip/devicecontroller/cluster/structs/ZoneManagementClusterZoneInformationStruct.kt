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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ZoneManagementClusterZoneInformationStruct (
    val zoneID: UInt,
    val zoneType: UInt,
    val zoneSource: UInt) {
  override fun toString(): String  = buildString {
    append("ZoneManagementClusterZoneInformationStruct {\n")
    append("\tzoneID : $zoneID\n")
    append("\tzoneType : $zoneType\n")
    append("\tzoneSource : $zoneSource\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ZONE_ID), zoneID)
      put(ContextSpecificTag(TAG_ZONE_TYPE), zoneType)
      put(ContextSpecificTag(TAG_ZONE_SOURCE), zoneSource)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ZONE_ID = 0
    private const val TAG_ZONE_TYPE = 1
    private const val TAG_ZONE_SOURCE = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ZoneManagementClusterZoneInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val zoneID = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_ID))
      val zoneType = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_TYPE))
      val zoneSource = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_SOURCE))
      
      tlvReader.exitContainer()

      return ZoneManagementClusterZoneInformationStruct(zoneID, zoneType, zoneSource)
    }
  }
}
