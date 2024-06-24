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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyCalendarClusterCalendarPeriodStruct(
  val startDate: ULong?,
  val days: List<EnergyCalendarClusterDayStruct>
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterCalendarPeriodStruct {\n")
    append("\tstartDate : $startDate\n")
    append("\tdays : $days\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (startDate != null) {
        put(ContextSpecificTag(TAG_START_DATE), startDate)
      } else {
        putNull(ContextSpecificTag(TAG_START_DATE))
      }
      startArray(ContextSpecificTag(TAG_DAYS))
      for (item in days.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_DATE = 0
    private const val TAG_DAYS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterCalendarPeriodStruct {
      tlvReader.enterStructure(tlvTag)
      val startDate =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_START_DATE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_START_DATE))
          null
        }
      val days =
        buildList<EnergyCalendarClusterDayStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_DAYS))
          while (!tlvReader.isEndOfContainer()) {
            add(EnergyCalendarClusterDayStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterCalendarPeriodStruct(startDate, days)
    }
  }
}
