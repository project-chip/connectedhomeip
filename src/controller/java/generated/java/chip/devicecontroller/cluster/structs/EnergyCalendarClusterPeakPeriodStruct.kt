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

class EnergyCalendarClusterPeakPeriodStruct(
  val severity: UInt,
  val peakPeriod: UInt,
  val startTime: ULong?,
  val endTime: ULong?
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterPeakPeriodStruct {\n")
    append("\tseverity : $severity\n")
    append("\tpeakPeriod : $peakPeriod\n")
    append("\tstartTime : $startTime\n")
    append("\tendTime : $endTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SEVERITY), severity)
      put(ContextSpecificTag(TAG_PEAK_PERIOD), peakPeriod)
      if (startTime != null) {
        put(ContextSpecificTag(TAG_START_TIME), startTime)
      } else {
        putNull(ContextSpecificTag(TAG_START_TIME))
      }
      if (endTime != null) {
        put(ContextSpecificTag(TAG_END_TIME), endTime)
      } else {
        putNull(ContextSpecificTag(TAG_END_TIME))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SEVERITY = 0
    private const val TAG_PEAK_PERIOD = 1
    private const val TAG_START_TIME = 2
    private const val TAG_END_TIME = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterPeakPeriodStruct {
      tlvReader.enterStructure(tlvTag)
      val severity = tlvReader.getUInt(ContextSpecificTag(TAG_SEVERITY))
      val peakPeriod = tlvReader.getUInt(ContextSpecificTag(TAG_PEAK_PERIOD))
      val startTime =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_START_TIME))
          null
        }
      val endTime =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_END_TIME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_END_TIME))
          null
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterPeakPeriodStruct(severity, peakPeriod, startTime, endTime)
    }
  }
}
