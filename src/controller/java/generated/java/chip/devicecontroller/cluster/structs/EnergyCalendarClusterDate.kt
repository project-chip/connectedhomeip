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

class EnergyCalendarClusterDate(
  val year: Optional<UInt>,
  val month: Optional<UInt>,
  val day: Optional<UInt>,
  val dayOfWeek: Optional<UInt>
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterDate {\n")
    append("\tyear : $year\n")
    append("\tmonth : $month\n")
    append("\tday : $day\n")
    append("\tdayOfWeek : $dayOfWeek\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (year.isPresent) {
        val optyear = year.get()
        put(ContextSpecificTag(TAG_YEAR), optyear)
      }
      if (month.isPresent) {
        val optmonth = month.get()
        put(ContextSpecificTag(TAG_MONTH), optmonth)
      }
      if (day.isPresent) {
        val optday = day.get()
        put(ContextSpecificTag(TAG_DAY), optday)
      }
      if (dayOfWeek.isPresent) {
        val optdayOfWeek = dayOfWeek.get()
        put(ContextSpecificTag(TAG_DAY_OF_WEEK), optdayOfWeek)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_YEAR = 0
    private const val TAG_MONTH = 1
    private const val TAG_DAY = 2
    private const val TAG_DAY_OF_WEEK = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterDate {
      tlvReader.enterStructure(tlvTag)
      val year =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_YEAR))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_YEAR)))
        } else {
          Optional.empty()
        }
      val month =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MONTH))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MONTH)))
        } else {
          Optional.empty()
        }
      val day =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DAY))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DAY)))
        } else {
          Optional.empty()
        }
      val dayOfWeek =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DAY_OF_WEEK))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterDate(year, month, day, dayOfWeek)
    }
  }
}
