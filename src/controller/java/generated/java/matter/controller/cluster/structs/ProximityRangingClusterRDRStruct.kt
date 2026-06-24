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

class ProximityRangingClusterRDRStruct(
  val azimuth: UShort,
  val elevation: Short,
  val azimuthAccuracy: UByte,
  val elevationAccuracy: UByte,
  val reference: UByte,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRDRStruct {\n")
    append("\tazimuth : $azimuth\n")
    append("\televation : $elevation\n")
    append("\tazimuthAccuracy : $azimuthAccuracy\n")
    append("\televationAccuracy : $elevationAccuracy\n")
    append("\treference : $reference\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_AZIMUTH), azimuth)
      put(ContextSpecificTag(TAG_ELEVATION), elevation)
      put(ContextSpecificTag(TAG_AZIMUTH_ACCURACY), azimuthAccuracy)
      put(ContextSpecificTag(TAG_ELEVATION_ACCURACY), elevationAccuracy)
      put(ContextSpecificTag(TAG_REFERENCE), reference)
      endStructure()
    }
  }

  companion object {
    private const val TAG_AZIMUTH = 0
    private const val TAG_ELEVATION = 1
    private const val TAG_AZIMUTH_ACCURACY = 2
    private const val TAG_ELEVATION_ACCURACY = 3
    private const val TAG_REFERENCE = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ProximityRangingClusterRDRStruct {
      tlvReader.enterStructure(tlvTag)
      val azimuth = tlvReader.getUShort(ContextSpecificTag(TAG_AZIMUTH))
      val elevation = tlvReader.getShort(ContextSpecificTag(TAG_ELEVATION))
      val azimuthAccuracy = tlvReader.getUByte(ContextSpecificTag(TAG_AZIMUTH_ACCURACY))
      val elevationAccuracy = tlvReader.getUByte(ContextSpecificTag(TAG_ELEVATION_ACCURACY))
      val reference = tlvReader.getUByte(ContextSpecificTag(TAG_REFERENCE))

      tlvReader.exitContainer()

      return ProximityRangingClusterRDRStruct(
        azimuth,
        elevation,
        azimuthAccuracy,
        elevationAccuracy,
        reference,
      )
    }
  }
}
