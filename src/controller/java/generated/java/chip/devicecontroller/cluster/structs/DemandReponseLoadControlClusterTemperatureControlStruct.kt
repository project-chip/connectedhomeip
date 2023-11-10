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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class DemandReponseLoadControlClusterTemperatureControlStruct (
    val coolingTempOffset: UInt?,
    val heatTempOffset: UInt?,
    val coolingTempSetpoint: UInt?,
    val heatingTempSetpoint: UInt?) {
  override fun toString(): String  = buildString {
    append("DemandReponseLoadControlClusterTemperatureControlStruct {\n")
    append("\tcoolingTempOffset : $coolingTempOffset\n")
    append("\theatTempOffset : $heatTempOffset\n")
    append("\tcoolingTempSetpoint : $coolingTempSetpoint\n")
    append("\theatingTempSetpoint : $heatingTempSetpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (coolingTempOffset != null) {
      put(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET), coolingTempOffset)
    } else {
      putNull(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))
    }
      if (heatTempOffset != null) {
      put(ContextSpecificTag(TAG_HEAT_TEMP_OFFSET), heatTempOffset)
    } else {
      putNull(ContextSpecificTag(TAG_HEAT_TEMP_OFFSET))
    }
      if (coolingTempSetpoint != null) {
      put(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT), coolingTempSetpoint)
    } else {
      putNull(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))
    }
      if (heatingTempSetpoint != null) {
      put(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT), heatingTempSetpoint)
    } else {
      putNull(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_COOLING_TEMP_OFFSET = 0
    private const val TAG_HEAT_TEMP_OFFSET = 1
    private const val TAG_COOLING_TEMP_SETPOINT = 2
    private const val TAG_HEATING_TEMP_SETPOINT = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : DemandReponseLoadControlClusterTemperatureControlStruct {
      tlvReader.enterStructure(tlvTag)
      val coolingTempOffset = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))
      null
    }
      val heatTempOffset = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_HEAT_TEMP_OFFSET))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_HEAT_TEMP_OFFSET))
      null
    }
      val coolingTempSetpoint = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))
      null
    }
      val heatingTempSetpoint = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))
      null
    }
      
      tlvReader.exitContainer()

      return DemandReponseLoadControlClusterTemperatureControlStruct(coolingTempOffset, heatTempOffset, coolingTempSetpoint, heatingTempSetpoint)
    }
  }
}
