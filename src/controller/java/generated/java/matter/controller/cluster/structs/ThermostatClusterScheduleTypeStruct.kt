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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterScheduleTypeStruct(
  val systemMode: UByte,
  val numberOfSchedules: UByte,
  val scheduleTypeFeatures: UShort,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterScheduleTypeStruct {\n")
    append("\tsystemMode : $systemMode\n")
    append("\tnumberOfSchedules : $numberOfSchedules\n")
    append("\tscheduleTypeFeatures : $scheduleTypeFeatures\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SYSTEM_MODE), systemMode)
      put(ContextSpecificTag(TAG_NUMBER_OF_SCHEDULES), numberOfSchedules)
      put(ContextSpecificTag(TAG_SCHEDULE_TYPE_FEATURES), scheduleTypeFeatures)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SYSTEM_MODE = 0
    private const val TAG_NUMBER_OF_SCHEDULES = 1
    private const val TAG_SCHEDULE_TYPE_FEATURES = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterScheduleTypeStruct {
      tlvReader.enterStructure(tlvTag)
      val systemMode = tlvReader.getUByte(ContextSpecificTag(TAG_SYSTEM_MODE))
      val numberOfSchedules = tlvReader.getUByte(ContextSpecificTag(TAG_NUMBER_OF_SCHEDULES))
      val scheduleTypeFeatures = tlvReader.getUShort(ContextSpecificTag(TAG_SCHEDULE_TYPE_FEATURES))

      tlvReader.exitContainer()

      return ThermostatClusterScheduleTypeStruct(
        systemMode,
        numberOfSchedules,
        scheduleTypeFeatures,
      )
    }
  }
}
