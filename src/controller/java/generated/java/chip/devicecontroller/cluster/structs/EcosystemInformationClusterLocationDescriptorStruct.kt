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

class EcosystemInformationClusterLocationDescriptorStruct(
  val locationName: String,
  val floorNumber: Int?,
  val areaType: UInt?,
) {
  override fun toString(): String = buildString {
    append("EcosystemInformationClusterLocationDescriptorStruct {\n")
    append("\tlocationName : $locationName\n")
    append("\tfloorNumber : $floorNumber\n")
    append("\tareaType : $areaType\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LOCATION_NAME), locationName)
      if (floorNumber != null) {
        put(ContextSpecificTag(TAG_FLOOR_NUMBER), floorNumber)
      } else {
        putNull(ContextSpecificTag(TAG_FLOOR_NUMBER))
      }
      if (areaType != null) {
        put(ContextSpecificTag(TAG_AREA_TYPE), areaType)
      } else {
        putNull(ContextSpecificTag(TAG_AREA_TYPE))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCATION_NAME = 0
    private const val TAG_FLOOR_NUMBER = 1
    private const val TAG_AREA_TYPE = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): EcosystemInformationClusterLocationDescriptorStruct {
      tlvReader.enterStructure(tlvTag)
      val locationName = tlvReader.getString(ContextSpecificTag(TAG_LOCATION_NAME))
      val floorNumber =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_FLOOR_NUMBER))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_FLOOR_NUMBER))
          null
        }
      val areaType =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_AREA_TYPE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AREA_TYPE))
          null
        }

      tlvReader.exitContainer()

      return EcosystemInformationClusterLocationDescriptorStruct(
        locationName,
        floorNumber,
        areaType,
      )
    }
  }
}
