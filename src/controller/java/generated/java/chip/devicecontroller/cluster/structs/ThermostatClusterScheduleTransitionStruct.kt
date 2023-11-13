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

class ThermostatClusterScheduleTransitionStruct (
    val dayOfWeek: UInt,
    val transitionTime: UInt,
    val presetHandle: ByteArray,
    val systemMode: UInt,
    val coolingSetpoint: Int,
    val heatingSetpoint: Int) {
  override fun toString(): String  = buildString {
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
      put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      put(ContextSpecificTag(TAG_SYSTEM_MODE), systemMode)
      put(ContextSpecificTag(TAG_COOLING_SETPOINT), coolingSetpoint)
      put(ContextSpecificTag(TAG_HEATING_SETPOINT), heatingSetpoint)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ThermostatClusterScheduleTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val dayOfWeek = tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK))
      val transitionTime = tlvReader.getUInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val presetHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
      val systemMode = tlvReader.getUInt(ContextSpecificTag(TAG_SYSTEM_MODE))
      val coolingSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_COOLING_SETPOINT))
      val heatingSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_HEATING_SETPOINT))
      
      tlvReader.exitContainer()

      return ThermostatClusterScheduleTransitionStruct(dayOfWeek, transitionTime, presetHandle, systemMode, coolingSetpoint, heatingSetpoint)
    }
  }
}
