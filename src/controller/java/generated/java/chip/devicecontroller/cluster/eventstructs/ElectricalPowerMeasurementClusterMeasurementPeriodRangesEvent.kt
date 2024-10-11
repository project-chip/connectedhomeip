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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent(
  val ranges:
    List<
      chip.devicecontroller.cluster.structs.ElectricalPowerMeasurementClusterMeasurementRangeStruct
    >
) {
  override fun toString(): String = buildString {
    append("ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent {\n")
    append("\tranges : $ranges\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_RANGES))
      for (item in ranges.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_RANGES = 0

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent {
      tlvReader.enterStructure(tlvTag)
      val ranges =
        buildList<
          chip.devicecontroller.cluster.structs.ElectricalPowerMeasurementClusterMeasurementRangeStruct
        > {
          tlvReader.enterArray(ContextSpecificTag(TAG_RANGES))
          while (!tlvReader.isEndOfContainer()) {
            this.add(
              chip.devicecontroller.cluster.structs
                .ElectricalPowerMeasurementClusterMeasurementRangeStruct
                .fromTlv(AnonymousTag, tlvReader)
            )
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent(ranges)
    }
  }
}
