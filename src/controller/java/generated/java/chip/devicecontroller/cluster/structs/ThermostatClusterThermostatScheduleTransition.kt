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
    tlvWriter.put(ContextSpecificTag(0), transitionTime)
    if (heatSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), heatSetpoint)
    }
    if (coolSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), coolSetpoint)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThermostatClusterThermostatScheduleTransition {
      tlvReader.enterStructure(tag)
      val transitionTime: Int = tlvReader.getInt(ContextSpecificTag(0))
      val heatSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
      val coolSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      
      tlvReader.exitContainer()

      return ThermostatClusterThermostatScheduleTransition(transitionTime, heatSetpoint, coolSetpoint)
    }
  }
}
