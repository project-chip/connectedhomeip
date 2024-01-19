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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyEvseClusterChargingTargetScheduleStruct(
  val dayOfWeekForSequence: UByte,
  val chargingTargets: List<EnergyEvseClusterChargingTargetStruct>
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
      put(ContextSpecificTag(TAG_DAY_OF_WEEK_FOR_SEQUENCE), dayOfWeekForSequence)
      startArray(ContextSpecificTag(TAG_CHARGING_TARGETS))
      for (item in chargingTargets.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_OF_WEEK_FOR_SEQUENCE = 0
    private const val TAG_CHARGING_TARGETS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyEvseClusterChargingTargetScheduleStruct {
      tlvReader.enterStructure(tlvTag)
      val dayOfWeekForSequence =
        tlvReader.getUByte(ContextSpecificTag(TAG_DAY_OF_WEEK_FOR_SEQUENCE))
      val chargingTargets =
        buildList<EnergyEvseClusterChargingTargetStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_CHARGING_TARGETS))
          while (!tlvReader.isEndOfContainer()) {
            add(EnergyEvseClusterChargingTargetStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return EnergyEvseClusterChargingTargetScheduleStruct(dayOfWeekForSequence, chargingTargets)
    }
  }
}
