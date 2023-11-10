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

class DemandReponseLoadControlClusterLoadControlEventTransitionStruct(
  val duration: UShort,
  val control: UInt,
  val temperatureControl: DemandReponseLoadControlClusterTemperatureControlStruct?,
  val averageLoadControl: DemandReponseLoadControlClusterAverageLoadControlStruct?,
  val dutyCycleControl: DemandReponseLoadControlClusterDutyCycleControlStruct?,
  val powerSavingsControl: DemandReponseLoadControlClusterPowerSavingsControlStruct?,
  val heatingSourceControl: DemandReponseLoadControlClusterHeatingSourceControlStruct?
) {
  override fun toString(): String = buildString {
    append("DemandReponseLoadControlClusterLoadControlEventTransitionStruct {\n")
    append("\tduration : $duration\n")
    append("\tcontrol : $control\n")
    append("\ttemperatureControl : $temperatureControl\n")
    append("\taverageLoadControl : $averageLoadControl\n")
    append("\tdutyCycleControl : $dutyCycleControl\n")
    append("\tpowerSavingsControl : $powerSavingsControl\n")
    append("\theatingSourceControl : $heatingSourceControl\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DURATION), duration)
      put(ContextSpecificTag(TAG_CONTROL), control)
      if (temperatureControl != null) {
        temperatureControl.toTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), this)
      } else {
        putNull(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))
      }
      if (averageLoadControl != null) {
        averageLoadControl.toTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), this)
      } else {
        putNull(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))
      }
      if (dutyCycleControl != null) {
        dutyCycleControl.toTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), this)
      } else {
        putNull(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))
      }
      if (powerSavingsControl != null) {
        powerSavingsControl.toTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), this)
      } else {
        putNull(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))
      }
      if (heatingSourceControl != null) {
        heatingSourceControl.toTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), this)
      } else {
        putNull(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DURATION = 0
    private const val TAG_CONTROL = 1
    private const val TAG_TEMPERATURE_CONTROL = 2
    private const val TAG_AVERAGE_LOAD_CONTROL = 3
    private const val TAG_DUTY_CYCLE_CONTROL = 4
    private const val TAG_POWER_SAVINGS_CONTROL = 5
    private const val TAG_HEATING_SOURCE_CONTROL = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DemandReponseLoadControlClusterLoadControlEventTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val duration = tlvReader.getUShort(ContextSpecificTag(TAG_DURATION))
      val control = tlvReader.getUInt(ContextSpecificTag(TAG_CONTROL))
      val temperatureControl = if (!tlvReader.isNull()) {
      DemandReponseLoadControlClusterTemperatureControlStruct.fromTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))
      null
    }
      val averageLoadControl = if (!tlvReader.isNull()) {
      DemandReponseLoadControlClusterAverageLoadControlStruct.fromTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))
      null
    }
      val dutyCycleControl = if (!tlvReader.isNull()) {
      DemandReponseLoadControlClusterDutyCycleControlStruct.fromTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))
      null
    }
      val powerSavingsControl = if (!tlvReader.isNull()) {
      DemandReponseLoadControlClusterPowerSavingsControlStruct.fromTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))
      null
    }
      val heatingSourceControl = if (!tlvReader.isNull()) {
      DemandReponseLoadControlClusterHeatingSourceControlStruct.fromTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))
      null
    }
      
      tlvReader.exitContainer()

      return DemandReponseLoadControlClusterLoadControlEventTransitionStruct(duration, control, temperatureControl, averageLoadControl, dutyCycleControl, powerSavingsControl, heatingSourceControl)
    }
  }
}
