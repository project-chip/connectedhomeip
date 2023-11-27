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
package matter.devicecontroller.cluster.structs

import java.util.Optional
import matter.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterThermostatScheduleTransitionStruct(
  val transitionTime: UShort,
  val heatSetpoint: Short,
  val coolSetpoint: Short
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterThermostatScheduleTransitionStruct {\n")
    append("\ttransitionTime : $transitionTime\n")
    append("\theatSetpoint : $heatSetpoint\n")
    append("\tcoolSetpoint : $coolSetpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
      put(ContextSpecificTag(TAG_HEAT_SETPOINT), heatSetpoint)
      put(ContextSpecificTag(TAG_COOL_SETPOINT), coolSetpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRANSITION_TIME = 0
    private const val TAG_HEAT_SETPOINT = 1
    private const val TAG_COOL_SETPOINT = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterThermostatScheduleTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val transitionTime = tlvReader.getUShort(ContextSpecificTag(TAG_TRANSITION_TIME))
      val heatSetpoint = tlvReader.getShort(ContextSpecificTag(TAG_HEAT_SETPOINT))
      val coolSetpoint = tlvReader.getShort(ContextSpecificTag(TAG_COOL_SETPOINT))
      
      tlvReader.exitContainer()

      return ThermostatClusterThermostatScheduleTransitionStruct(transitionTime, heatSetpoint, coolSetpoint)
    }
  }
}
