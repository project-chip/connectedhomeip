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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyEvseClusterChargingTargetStruct(
  val targetTimeMinutesPastMidnight: UInt,
  val targetSoC: Optional<UInt>,
  val addedEnergy: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("EnergyEvseClusterChargingTargetStruct {\n")
    append("\ttargetTimeMinutesPastMidnight : $targetTimeMinutesPastMidnight\n")
    append("\ttargetSoC : $targetSoC\n")
    append("\taddedEnergy : $addedEnergy\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TARGET_TIME_MINUTES_PAST_MIDNIGHT), targetTimeMinutesPastMidnight)
      if (targetSoC.isPresent) {
        val opttargetSoC = targetSoC.get()
        put(ContextSpecificTag(TAG_TARGET_SO_C), opttargetSoC)
      }
      if (addedEnergy.isPresent) {
        val optaddedEnergy = addedEnergy.get()
        put(ContextSpecificTag(TAG_ADDED_ENERGY), optaddedEnergy)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TARGET_TIME_MINUTES_PAST_MIDNIGHT = 0
    private const val TAG_TARGET_SO_C = 1
    private const val TAG_ADDED_ENERGY = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterChargingTargetStruct {
      tlvReader.enterStructure(tlvTag)
      val targetTimeMinutesPastMidnight =
        tlvReader.getUInt(ContextSpecificTag(TAG_TARGET_TIME_MINUTES_PAST_MIDNIGHT))
      val targetSoC =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TARGET_SO_C))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TARGET_SO_C)))
        } else {
          Optional.empty()
        }
      val addedEnergy =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ADDED_ENERGY))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_ADDED_ENERGY)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterChargingTargetStruct(
        targetTimeMinutesPastMidnight,
        targetSoC,
        addedEnergy,
      )
    }
  }
}
