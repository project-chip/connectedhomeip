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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent(
  val eventID: ByteArray,
  val transitionIndex: UByte?,
  val status: UByte,
  val criticality: UByte,
  val control: UShort,
  val temperatureControl:
    Optional<
      matter.controller.cluster.structs.DemandResponseLoadControlClusterTemperatureControlStruct
    >?,
  val averageLoadControl:
    Optional<
      matter.controller.cluster.structs.DemandResponseLoadControlClusterAverageLoadControlStruct
    >?,
  val dutyCycleControl:
    Optional<
      matter.controller.cluster.structs.DemandResponseLoadControlClusterDutyCycleControlStruct
    >?,
  val powerSavingsControl:
    Optional<
      matter.controller.cluster.structs.DemandResponseLoadControlClusterPowerSavingsControlStruct
    >?,
  val heatingSourceControl:
    Optional<
      matter.controller.cluster.structs.DemandResponseLoadControlClusterHeatingSourceControlStruct
    >?
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent {\n")
    append("\teventID : $eventID\n")
    append("\ttransitionIndex : $transitionIndex\n")
    append("\tstatus : $status\n")
    append("\tcriticality : $criticality\n")
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
      put(ContextSpecificTag(TAG_EVENT_I_D), eventID)
      if (transitionIndex != null) {
        put(ContextSpecificTag(TAG_TRANSITION_INDEX), transitionIndex)
      } else {
        putNull(ContextSpecificTag(TAG_TRANSITION_INDEX))
      }
      put(ContextSpecificTag(TAG_STATUS), status)
      put(ContextSpecificTag(TAG_CRITICALITY), criticality)
      put(ContextSpecificTag(TAG_CONTROL), control)
      if (temperatureControl != null) {
        if (temperatureControl.isPresent) {
          val opttemperatureControl = temperatureControl.get()
          opttemperatureControl.toTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))
      }
      if (averageLoadControl != null) {
        if (averageLoadControl.isPresent) {
          val optaverageLoadControl = averageLoadControl.get()
          optaverageLoadControl.toTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))
      }
      if (dutyCycleControl != null) {
        if (dutyCycleControl.isPresent) {
          val optdutyCycleControl = dutyCycleControl.get()
          optdutyCycleControl.toTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))
      }
      if (powerSavingsControl != null) {
        if (powerSavingsControl.isPresent) {
          val optpowerSavingsControl = powerSavingsControl.get()
          optpowerSavingsControl.toTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))
      }
      if (heatingSourceControl != null) {
        if (heatingSourceControl.isPresent) {
          val optheatingSourceControl = heatingSourceControl.get()
          optheatingSourceControl.toTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_EVENT_I_D = 0
    private const val TAG_TRANSITION_INDEX = 1
    private const val TAG_STATUS = 2
    private const val TAG_CRITICALITY = 3
    private const val TAG_CONTROL = 4
    private const val TAG_TEMPERATURE_CONTROL = 5
    private const val TAG_AVERAGE_LOAD_CONTROL = 6
    private const val TAG_DUTY_CYCLE_CONTROL = 7
    private const val TAG_POWER_SAVINGS_CONTROL = 8
    private const val TAG_HEATING_SOURCE_CONTROL = 9

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val eventID = tlvReader.getByteArray(ContextSpecificTag(TAG_EVENT_I_D))
      val transitionIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_TRANSITION_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TRANSITION_INDEX))
          null
        }
      val status = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))
      val criticality = tlvReader.getUByte(ContextSpecificTag(TAG_CRITICALITY))
      val control = tlvReader.getUShort(ContextSpecificTag(TAG_CONTROL))
      val temperatureControl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))) {
            Optional.of(
              matter.controller.cluster.structs
                .DemandResponseLoadControlClusterTemperatureControlStruct
                .fromTlv(ContextSpecificTag(TAG_TEMPERATURE_CONTROL), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TEMPERATURE_CONTROL))
          null
        }
      val averageLoadControl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))) {
            Optional.of(
              matter.controller.cluster.structs
                .DemandResponseLoadControlClusterAverageLoadControlStruct
                .fromTlv(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AVERAGE_LOAD_CONTROL))
          null
        }
      val dutyCycleControl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))) {
            Optional.of(
              matter.controller.cluster.structs
                .DemandResponseLoadControlClusterDutyCycleControlStruct
                .fromTlv(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DUTY_CYCLE_CONTROL))
          null
        }
      val powerSavingsControl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))) {
            Optional.of(
              matter.controller.cluster.structs
                .DemandResponseLoadControlClusterPowerSavingsControlStruct
                .fromTlv(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_POWER_SAVINGS_CONTROL))
          null
        }
      val heatingSourceControl =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))) {
            Optional.of(
              matter.controller.cluster.structs
                .DemandResponseLoadControlClusterHeatingSourceControlStruct
                .fromTlv(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HEATING_SOURCE_CONTROL))
          null
        }

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent(
        eventID,
        transitionIndex,
        status,
        criticality,
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
