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

class ThermostatClusterSensorScheduleTransitionStruct(
  val dayOfWeek: UInt,
  val transitionTime: UInt,
  val enabledSensorHandles: List<ByteArray>,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterSensorScheduleTransitionStruct {\n")
    append("\tdayOfWeek : $dayOfWeek\n")
    append("\ttransitionTime : $transitionTime\n")
    append("\tenabledSensorHandles : $enabledSensorHandles\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DAY_OF_WEEK), dayOfWeek)
      put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
      startArray(ContextSpecificTag(TAG_ENABLED_SENSOR_HANDLES))
      for (item in enabledSensorHandles.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_OF_WEEK = 0
    private const val TAG_TRANSITION_TIME = 1
    private const val TAG_ENABLED_SENSOR_HANDLES = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ThermostatClusterSensorScheduleTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val dayOfWeek = tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK))
      val transitionTime = tlvReader.getUInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val enabledSensorHandles =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ENABLED_SENSOR_HANDLES))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ThermostatClusterSensorScheduleTransitionStruct(
        dayOfWeek,
        transitionTime,
        enabledSensorHandles,
      )
    }
  }
}
