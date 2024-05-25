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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ServiceAreaClusterLocationInfoStruct(
  val homeLocationInfo: ServiceAreaClusterHomeLocationStruct?,
  val landmarkTag: UByte?,
  val positionTag: UByte?,
  val surfaceTag: UByte?
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterLocationInfoStruct {\n")
    append("\thomeLocationInfo : $homeLocationInfo\n")
    append("\tlandmarkTag : $landmarkTag\n")
    append("\tpositionTag : $positionTag\n")
    append("\tsurfaceTag : $surfaceTag\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (homeLocationInfo != null) {
        homeLocationInfo.toTlv(ContextSpecificTag(TAG_HOME_LOCATION_INFO), this)
      } else {
        putNull(ContextSpecificTag(TAG_HOME_LOCATION_INFO))
      }
      if (landmarkTag != null) {
        put(ContextSpecificTag(TAG_LANDMARK_TAG), landmarkTag)
      } else {
        putNull(ContextSpecificTag(TAG_LANDMARK_TAG))
      }
      if (positionTag != null) {
        put(ContextSpecificTag(TAG_POSITION_TAG), positionTag)
      } else {
        putNull(ContextSpecificTag(TAG_POSITION_TAG))
      }
      if (surfaceTag != null) {
        put(ContextSpecificTag(TAG_SURFACE_TAG), surfaceTag)
      } else {
        putNull(ContextSpecificTag(TAG_SURFACE_TAG))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_HOME_LOCATION_INFO = 0
    private const val TAG_LANDMARK_TAG = 1
    private const val TAG_POSITION_TAG = 2
    private const val TAG_SURFACE_TAG = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterLocationInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val homeLocationInfo = if (!tlvReader.isNull()) {
      ServiceAreaClusterHomeLocationStruct.fromTlv(ContextSpecificTag(TAG_HOME_LOCATION_INFO), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_HOME_LOCATION_INFO))
      null
    }
      val landmarkTag = if (!tlvReader.isNull()) {
      tlvReader.getUByte(ContextSpecificTag(TAG_LANDMARK_TAG))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_LANDMARK_TAG))
      null
    }
      val positionTag = if (!tlvReader.isNull()) {
      tlvReader.getUByte(ContextSpecificTag(TAG_POSITION_TAG))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_POSITION_TAG))
      null
    }
      val surfaceTag = if (!tlvReader.isNull()) {
      tlvReader.getUByte(ContextSpecificTag(TAG_SURFACE_TAG))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_SURFACE_TAG))
      null
    }
      
      tlvReader.exitContainer()

      return ServiceAreaClusterLocationInfoStruct(homeLocationInfo, landmarkTag, positionTag, surfaceTag)
    }
  }
}
