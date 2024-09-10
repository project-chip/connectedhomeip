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

class ServiceAreaClusterLandmarkInfoStruct(
  val landmarkTag: UByte,
  val relativePositionTag: UByte?,
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterLandmarkInfoStruct {\n")
    append("\tlandmarkTag : $landmarkTag\n")
    append("\trelativePositionTag : $relativePositionTag\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LANDMARK_TAG), landmarkTag)
      if (relativePositionTag != null) {
        put(ContextSpecificTag(TAG_RELATIVE_POSITION_TAG), relativePositionTag)
      } else {
        putNull(ContextSpecificTag(TAG_RELATIVE_POSITION_TAG))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LANDMARK_TAG = 0
    private const val TAG_RELATIVE_POSITION_TAG = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterLandmarkInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val landmarkTag = tlvReader.getUByte(ContextSpecificTag(TAG_LANDMARK_TAG))
      val relativePositionTag =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_RELATIVE_POSITION_TAG))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RELATIVE_POSITION_TAG))
          null
        }

      tlvReader.exitContainer()

      return ServiceAreaClusterLandmarkInfoStruct(landmarkTag, relativePositionTag)
    }
  }
}
