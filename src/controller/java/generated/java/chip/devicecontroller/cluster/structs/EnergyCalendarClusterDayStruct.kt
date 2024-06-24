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

class EnergyCalendarClusterDayStruct(
  val date: Optional<ULong>,
  val daysOfWeek: Optional<UInt>,
  val transitions: List<EnergyCalendarClusterTransitionStruct>,
  val calendarID: Optional<ULong>
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterDayStruct {\n")
    append("\tdate : $date\n")
    append("\tdaysOfWeek : $daysOfWeek\n")
    append("\ttransitions : $transitions\n")
    append("\tcalendarID : $calendarID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (date.isPresent) {
        val optdate = date.get()
        put(ContextSpecificTag(TAG_DATE), optdate)
      }
      if (daysOfWeek.isPresent) {
        val optdaysOfWeek = daysOfWeek.get()
        put(ContextSpecificTag(TAG_DAYS_OF_WEEK), optdaysOfWeek)
      }
      startArray(ContextSpecificTag(TAG_TRANSITIONS))
      for (item in transitions.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      if (calendarID.isPresent) {
        val optcalendarID = calendarID.get()
        put(ContextSpecificTag(TAG_CALENDAR_I_D), optcalendarID)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DATE = 0
    private const val TAG_DAYS_OF_WEEK = 1
    private const val TAG_TRANSITIONS = 2
    private const val TAG_CALENDAR_I_D = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterDayStruct {
      tlvReader.enterStructure(tlvTag)
      val date =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_DATE)))
        } else {
          Optional.empty()
        }
      val daysOfWeek =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DAYS_OF_WEEK))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DAYS_OF_WEEK)))
        } else {
          Optional.empty()
        }
      val transitions =
        buildList<EnergyCalendarClusterTransitionStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_TRANSITIONS))
          while (!tlvReader.isEndOfContainer()) {
            add(EnergyCalendarClusterTransitionStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val calendarID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CALENDAR_I_D))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_CALENDAR_I_D)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterDayStruct(date, daysOfWeek, transitions, calendarID)
    }
  }
}
