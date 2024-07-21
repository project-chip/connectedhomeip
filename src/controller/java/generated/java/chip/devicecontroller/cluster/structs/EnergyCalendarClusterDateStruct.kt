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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyCalendarClusterDateStruct(
  val year: UInt?,
  val month: UInt?,
  val day: UInt?,
  val dayOfWeek: UInt?,
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterDateStruct {\n")
    append("\tyear : $year\n")
    append("\tmonth : $month\n")
    append("\tday : $day\n")
    append("\tdayOfWeek : $dayOfWeek\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (year != null) {
        put(ContextSpecificTag(TAG_YEAR), year)
      } else {
        putNull(ContextSpecificTag(TAG_YEAR))
      }
      if (month != null) {
        put(ContextSpecificTag(TAG_MONTH), month)
      } else {
        putNull(ContextSpecificTag(TAG_MONTH))
      }
      if (day != null) {
        put(ContextSpecificTag(TAG_DAY), day)
      } else {
        putNull(ContextSpecificTag(TAG_DAY))
      }
      if (dayOfWeek != null) {
        put(ContextSpecificTag(TAG_DAY_OF_WEEK), dayOfWeek)
      } else {
        putNull(ContextSpecificTag(TAG_DAY_OF_WEEK))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_YEAR = 0
    private const val TAG_MONTH = 1
    private const val TAG_DAY = 2
    private const val TAG_DAY_OF_WEEK = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterDateStruct {
      tlvReader.enterStructure(tlvTag)
      val year =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_YEAR))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_YEAR))
          null
        }
      val month =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_MONTH))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MONTH))
          null
        }
      val day =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_DAY))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DAY))
          null
        }
      val dayOfWeek =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DAY_OF_WEEK))
          null
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterDateStruct(year, month, day, dayOfWeek)
    }
  }
}
