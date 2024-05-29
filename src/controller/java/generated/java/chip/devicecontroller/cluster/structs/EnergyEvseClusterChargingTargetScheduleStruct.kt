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
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyEvseClusterChargingTargetScheduleStruct(
  val dayOfWeekForSequence: Optional<UInt>,
  val chargingTargets: Optional<List<EnergyEvseClusterChargingTargetStruct>>
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterChargingTargetScheduleStruct {\n")
    append("\tdayOfWeekForSequence : $dayOfWeekForSequence\n")
    append("\tchargingTargets : $chargingTargets\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (dayOfWeekForSequence.isPresent) {
        val optdayOfWeekForSequence = dayOfWeekForSequence.get()
        put(ContextSpecificTag(TAG_DAY_OF_WEEK_FOR_SEQUENCE), optdayOfWeekForSequence)
      }
      if (chargingTargets.isPresent) {
        val optchargingTargets = chargingTargets.get()
        startArray(ContextSpecificTag(TAG_CHARGING_TARGETS))
        for (item in optchargingTargets.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_OF_WEEK_FOR_SEQUENCE = 0
    private const val TAG_CHARGING_TARGETS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterChargingTargetScheduleStruct {
      tlvReader.enterStructure(tlvTag)
      val dayOfWeekForSequence =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DAY_OF_WEEK_FOR_SEQUENCE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK_FOR_SEQUENCE)))
        } else {
          Optional.empty()
        }
      val chargingTargets =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CHARGING_TARGETS))) {
          Optional.of(
            buildList<EnergyEvseClusterChargingTargetStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_CHARGING_TARGETS))
              while (!tlvReader.isEndOfContainer()) {
                add(EnergyEvseClusterChargingTargetStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterChargingTargetScheduleStruct(dayOfWeekForSequence, chargingTargets)
    }
  }
}
