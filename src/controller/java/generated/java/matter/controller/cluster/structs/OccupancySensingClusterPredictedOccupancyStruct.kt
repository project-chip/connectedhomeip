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

class OccupancySensingClusterPredictedOccupancyStruct(
  val startTimestamp: UInt,
  val endTimestamp: UInt,
  val occupancy: UByte,
  val confidence: UByte,
) {
  override fun toString(): String = buildString {
    append("OccupancySensingClusterPredictedOccupancyStruct {\n")
    append("\tstartTimestamp : $startTimestamp\n")
    append("\tendTimestamp : $endTimestamp\n")
    append("\toccupancy : $occupancy\n")
    append("\tconfidence : $confidence\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_TIMESTAMP), startTimestamp)
      put(ContextSpecificTag(TAG_END_TIMESTAMP), endTimestamp)
      put(ContextSpecificTag(TAG_OCCUPANCY), occupancy)
      put(ContextSpecificTag(TAG_CONFIDENCE), confidence)
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_TIMESTAMP = 0
    private const val TAG_END_TIMESTAMP = 1
    private const val TAG_OCCUPANCY = 2
    private const val TAG_CONFIDENCE = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): OccupancySensingClusterPredictedOccupancyStruct {
      tlvReader.enterStructure(tlvTag)
      val startTimestamp = tlvReader.getUInt(ContextSpecificTag(TAG_START_TIMESTAMP))
      val endTimestamp = tlvReader.getUInt(ContextSpecificTag(TAG_END_TIMESTAMP))
      val occupancy = tlvReader.getUByte(ContextSpecificTag(TAG_OCCUPANCY))
      val confidence = tlvReader.getUByte(ContextSpecificTag(TAG_CONFIDENCE))

      tlvReader.exitContainer()

      return OccupancySensingClusterPredictedOccupancyStruct(
        startTimestamp,
        endTimestamp,
        occupancy,
        confidence,
      )
    }
  }
}
