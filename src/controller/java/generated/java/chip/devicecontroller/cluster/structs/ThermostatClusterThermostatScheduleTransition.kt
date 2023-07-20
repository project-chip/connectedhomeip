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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ThermostatClusterThermostatScheduleTransition (
    val transitionTime: Int,
    val heatSetpoint: Int?,
    val coolSetpoint: Int?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ThermostatClusterThermostatScheduleTransition {\n")
    builder.append("\ttransitionTime : $transitionTime\n")
    builder.append("\theatSetpoint : $heatSetpoint\n")
    builder.append("\tcoolSetpoint : $coolSetpoint\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
    if (heatSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(TAG_HEAT_SETPOINT)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_HEAT_SETPOINT), heatSetpoint)
    }
    if (coolSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(TAG_COOL_SETPOINT)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_COOL_SETPOINT), coolSetpoint)
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_TRANSITION_TIME = 0
    private const val TAG_HEAT_SETPOINT = 1
    private const val TAG_COOL_SETPOINT = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThermostatClusterThermostatScheduleTransition {
      tlvReader.enterStructure(tag)
      val transitionTime: Int = tlvReader.getInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val heatSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_HEAT_SETPOINT))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_HEAT_SETPOINT))
      null
    }
      val coolSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_COOL_SETPOINT))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_COOL_SETPOINT))
      null
    }
      
      tlvReader.exitContainer()

      return ThermostatClusterThermostatScheduleTransition(transitionTime, heatSetpoint, coolSetpoint)
    }
  }
}
