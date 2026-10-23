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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterOccupancyChangeEvent(
  val previousOccupancy: Optional<UInt>,
  val currentOccupancy: UInt,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterOccupancyChangeEvent {\n")
    append("\tpreviousOccupancy : $previousOccupancy\n")
    append("\tcurrentOccupancy : $currentOccupancy\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (previousOccupancy.isPresent) {
        val optpreviousOccupancy = previousOccupancy.get()
        put(ContextSpecificTag(TAG_PREVIOUS_OCCUPANCY), optpreviousOccupancy)
      }
      put(ContextSpecificTag(TAG_CURRENT_OCCUPANCY), currentOccupancy)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_OCCUPANCY = 0
    private const val TAG_CURRENT_OCCUPANCY = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterOccupancyChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val previousOccupancy =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREVIOUS_OCCUPANCY))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PREVIOUS_OCCUPANCY)))
        } else {
          Optional.empty()
        }
      val currentOccupancy = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_OCCUPANCY))

      tlvReader.exitContainer()

      return ThermostatClusterOccupancyChangeEvent(previousOccupancy, currentOccupancy)
    }
  }
}
