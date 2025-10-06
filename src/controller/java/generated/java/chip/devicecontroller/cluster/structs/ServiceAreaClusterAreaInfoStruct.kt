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

class ServiceAreaClusterAreaInfoStruct(
  val locationInfo: ServiceAreaClusterLocationDescriptorStruct?,
  val landmarkInfo: ServiceAreaClusterLandmarkInfoStruct?,
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterAreaInfoStruct {\n")
    append("\tlocationInfo : $locationInfo\n")
    append("\tlandmarkInfo : $landmarkInfo\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (locationInfo != null) {
        locationInfo.toTlv(ContextSpecificTag(TAG_LOCATION_INFO), this)
      } else {
        putNull(ContextSpecificTag(TAG_LOCATION_INFO))
      }
      if (landmarkInfo != null) {
        landmarkInfo.toTlv(ContextSpecificTag(TAG_LANDMARK_INFO), this)
      } else {
        putNull(ContextSpecificTag(TAG_LANDMARK_INFO))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCATION_INFO = 0
    private const val TAG_LANDMARK_INFO = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterAreaInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val locationInfo =
        if (!tlvReader.isNull()) {
          ServiceAreaClusterLocationDescriptorStruct.fromTlv(
            ContextSpecificTag(TAG_LOCATION_INFO),
            tlvReader,
          )
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LOCATION_INFO))
          null
        }
      val landmarkInfo =
        if (!tlvReader.isNull()) {
          ServiceAreaClusterLandmarkInfoStruct.fromTlv(
            ContextSpecificTag(TAG_LANDMARK_INFO),
            tlvReader,
          )
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LANDMARK_INFO))
          null
        }

      tlvReader.exitContainer()

      return ServiceAreaClusterAreaInfoStruct(locationInfo, landmarkInfo)
    }
  }
}
