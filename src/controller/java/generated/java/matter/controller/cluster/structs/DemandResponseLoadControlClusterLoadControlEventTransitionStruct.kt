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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandResponseLoadControlClusterLoadControlEventTransitionStruct(
  val duration: UShort,
  val control: UShort,
  val temperatureControl: Optional<DemandResponseLoadControlClusterTemperatureControlStruct>,
  val averageLoadControl: Optional<DemandResponseLoadControlClusterAverageLoadControlStruct>,
  val dutyCycleControl: Optional<DemandResponseLoadControlClusterDutyCycleControlStruct>,
  val powerSavingsControl: Optional<DemandResponseLoadControlClusterPowerSavingsControlStruct>,
  val heatingSourceControl: Optional<DemandResponseLoadControlClusterHeatingSourceControlStruct>
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterLoadControlEventTransitionStruct {\n")
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
      if (temperatureControl.isPresent) {
        val opttemperatureControl = temperatureControl.get()
        opttemperatureControl.toTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), this)
      }
      if (averageLoadControl.isPresent) {
        val optaverageLoadControl = averageLoadControl.get()
        optaverageLoadControl.toTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), this)
      }
      if (dutyCycleControl.isPresent) {
        val optdutyCycleControl = dutyCycleControl.get()
        optdutyCycleControl.toTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), this)
      }
      if (powerSavingsControl.isPresent) {
        val optpowerSavingsControl = powerSavingsControl.get()
        optpowerSavingsControl.toTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), this)
      }
      if (heatingSourceControl.isPresent) {
        val optheatingSourceControl = heatingSourceControl.get()
        optheatingSourceControl.toTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), this)
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

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DemandResponseLoadControlClusterLoadControlEventTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val duration = tlvReader.getUShort(ContextSpecificTag(TAG_DURATION))
      val control = tlvReader.getUShort(ContextSpecificTag(TAG_CONTROL))
      val temperatureControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))) {
          Optional.of(
            DemandResponseLoadControlClusterTemperatureControlStruct.fromTlv(
              ContextSpecificTag(TAG_TEMPERATURE_CONTROL),
              tlvReader
            )
          )
        } else {
          Optional.empty()
        }
      val averageLoadControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))) {
          Optional.of(
            DemandResponseLoadControlClusterAverageLoadControlStruct.fromTlv(
              ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL),
              tlvReader
            )
          )
        } else {
          Optional.empty()
        }
      val dutyCycleControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))) {
          Optional.of(
            DemandResponseLoadControlClusterDutyCycleControlStruct.fromTlv(
              ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL),
              tlvReader
            )
          )
        } else {
          Optional.empty()
        }
      val powerSavingsControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))) {
          Optional.of(
            DemandResponseLoadControlClusterPowerSavingsControlStruct.fromTlv(
              ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL),
              tlvReader
            )
          )
        } else {
          Optional.empty()
        }
      val heatingSourceControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))) {
          Optional.of(
            DemandResponseLoadControlClusterHeatingSourceControlStruct.fromTlv(
              ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL),
              tlvReader
            )
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterLoadControlEventTransitionStruct(
        duration,
        control,
        temperatureControl,
        averageLoadControl,
        dutyCycleControl,
        powerSavingsControl,
        heatingSourceControl
      )
    }
  }
}
