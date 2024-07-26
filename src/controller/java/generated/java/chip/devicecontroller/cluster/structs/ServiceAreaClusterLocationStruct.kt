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

class ServiceAreaClusterLocationStruct(
  val locationID: ULong,
  val mapID: UInt?,
  val locationInfo: ServiceAreaClusterLocationInfoStruct,
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterLocationStruct {\n")
    append("\tlocationID : $locationID\n")
    append("\tmapID : $mapID\n")
    append("\tlocationInfo : $locationInfo\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LOCATION_I_D), locationID)
      if (mapID != null) {
        put(ContextSpecificTag(TAG_MAP_I_D), mapID)
      } else {
        putNull(ContextSpecificTag(TAG_MAP_I_D))
      }
      locationInfo.toTlv(ContextSpecificTag(TAG_LOCATION_INFO), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCATION_I_D = 0
    private const val TAG_MAP_I_D = 1
    private const val TAG_LOCATION_INFO = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterLocationStruct {
      tlvReader.enterStructure(tlvTag)
      val locationID = tlvReader.getULong(ContextSpecificTag(TAG_LOCATION_I_D))
      val mapID =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_MAP_I_D))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MAP_I_D))
          null
        }
      val locationInfo =
        ServiceAreaClusterLocationInfoStruct.fromTlv(
          ContextSpecificTag(TAG_LOCATION_INFO),
          tlvReader,
        )

      tlvReader.exitContainer()

      return ServiceAreaClusterLocationStruct(locationID, mapID, locationInfo)
    }
  }
}
