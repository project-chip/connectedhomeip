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

class EnergyCalendarClusterCalendarPeriod(
  val startDate: UInt,
  val days: List<EnergyCalendarClusterDayStruct>
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterCalendarPeriod {\n")
    append("\tstartDate : $startDate\n")
    append("\tdays : $days\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_DATE), startDate)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterCalendarPeriod {
      tlvReader.enterStructure(tlvTag)
      val startDate = tlvReader.getUInt(ContextSpecificTag(TAG_START_DATE))
      val days =
        buildList<EnergyCalendarClusterDayStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_DAYS))
          while (!tlvReader.isEndOfContainer()) {
            add(EnergyCalendarClusterDayStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterCalendarPeriod(startDate, days)
    }
  }
}
