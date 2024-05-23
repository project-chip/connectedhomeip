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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterScheduleTransitionStruct(
  val dayOfWeek: UInt,
  val transitionTime: UInt,
  val presetHandle: Optional<ByteArray>,
  val systemMode: Optional<UInt>,
  val coolingSetpoint: Optional<Int>,
  val heatingSetpoint: Optional<Int>
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterScheduleTransitionStruct {\n")
    append("\tdayOfWeek : $dayOfWeek\n")
    append("\ttransitionTime : $transitionTime\n")
    append("\tpresetHandle : $presetHandle\n")
    append("\tsystemMode : $systemMode\n")
    append("\tcoolingSetpoint : $coolingSetpoint\n")
    append("\theatingSetpoint : $heatingSetpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DAY_OF_WEEK), dayOfWeek)
      put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
      if (presetHandle.isPresent) {
        val optpresetHandle = presetHandle.get()
        put(ContextSpecificTag(TAG_PRESET_HANDLE), optpresetHandle)
      }
      if (systemMode.isPresent) {
        val optsystemMode = systemMode.get()
        put(ContextSpecificTag(TAG_SYSTEM_MODE), optsystemMode)
      }
      if (coolingSetpoint.isPresent) {
        val optcoolingSetpoint = coolingSetpoint.get()
        put(ContextSpecificTag(TAG_COOLING_SETPOINT), optcoolingSetpoint)
      }
      if (heatingSetpoint.isPresent) {
        val optheatingSetpoint = heatingSetpoint.get()
        put(ContextSpecificTag(TAG_HEATING_SETPOINT), optheatingSetpoint)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_OF_WEEK = 0
    private const val TAG_TRANSITION_TIME = 1
    private const val TAG_PRESET_HANDLE = 2
    private const val TAG_SYSTEM_MODE = 3
    private const val TAG_COOLING_SETPOINT = 4
    private const val TAG_HEATING_SETPOINT = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterScheduleTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val dayOfWeek = tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK))
      val transitionTime = tlvReader.getUInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val presetHandle =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRESET_HANDLE))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE)))
        } else {
          Optional.empty()
        }
      val systemMode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SYSTEM_MODE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SYSTEM_MODE)))
        } else {
          Optional.empty()
        }
      val coolingSetpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_COOLING_SETPOINT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_COOLING_SETPOINT)))
        } else {
          Optional.empty()
        }
      val heatingSetpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_HEATING_SETPOINT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_HEATING_SETPOINT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ThermostatClusterScheduleTransitionStruct(
        dayOfWeek,
        transitionTime,
        presetHandle,
        systemMode,
        coolingSetpoint,
        heatingSetpoint
      )
    }
  }
}
