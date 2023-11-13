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

class ThermostatClusterPresetStruct (
    val presetHandle: ByteArray,
    val presetScenario: UInt,
    val name: String?,
    val coolingSetpoint: Int,
    val heatingSetpoint: Int,
    val builtIn: Boolean) {
  override fun toString(): String  = buildString {
    append("ThermostatClusterPresetStruct {\n")
    append("\tpresetHandle : $presetHandle\n")
    append("\tpresetScenario : $presetScenario\n")
    append("\tname : $name\n")
    append("\tcoolingSetpoint : $coolingSetpoint\n")
    append("\theatingSetpoint : $heatingSetpoint\n")
    append("\tbuiltIn : $builtIn\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      put(ContextSpecificTag(TAG_PRESET_SCENARIO), presetScenario)
      if (name != null) {
      put(ContextSpecificTag(TAG_NAME), name)
    } else {
      putNull(ContextSpecificTag(TAG_NAME))
    }
      put(ContextSpecificTag(TAG_COOLING_SETPOINT), coolingSetpoint)
      put(ContextSpecificTag(TAG_HEATING_SETPOINT), heatingSetpoint)
      put(ContextSpecificTag(TAG_BUILT_IN), builtIn)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRESET_HANDLE = 0
    private const val TAG_PRESET_SCENARIO = 1
    private const val TAG_NAME = 2
    private const val TAG_COOLING_SETPOINT = 3
    private const val TAG_HEATING_SETPOINT = 4
    private const val TAG_BUILT_IN = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ThermostatClusterPresetStruct {
      tlvReader.enterStructure(tlvTag)
      val presetHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
      val presetScenario = tlvReader.getUInt(ContextSpecificTag(TAG_PRESET_SCENARIO))
      val name = if (!tlvReader.isNull()) {
      tlvReader.getString(ContextSpecificTag(TAG_NAME))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_NAME))
      null
    }
      val coolingSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_COOLING_SETPOINT))
      val heatingSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_HEATING_SETPOINT))
      val builtIn = tlvReader.getBoolean(ContextSpecificTag(TAG_BUILT_IN))
      
      tlvReader.exitContainer()

      return ThermostatClusterPresetStruct(presetHandle, presetScenario, name, coolingSetpoint, heatingSetpoint, builtIn)
    }
  }
}
