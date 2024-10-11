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

class ThermostatClusterPresetStruct(
  val presetHandle: ByteArray?,
  val presetScenario: UInt,
  val name: Optional<String>?,
  val coolingSetpoint: Optional<Int>,
  val heatingSetpoint: Optional<Int>,
  val builtIn: Boolean?,
) {
  override fun toString(): String = buildString {
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
      if (presetHandle != null) {
        put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      } else {
        putNull(ContextSpecificTag(TAG_PRESET_HANDLE))
      }
      put(ContextSpecificTag(TAG_PRESET_SCENARIO), presetScenario)
      if (name != null) {
        if (name.isPresent) {
          val optname = name.get()
          put(ContextSpecificTag(TAG_NAME), optname)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NAME))
      }
      if (coolingSetpoint.isPresent) {
        val optcoolingSetpoint = coolingSetpoint.get()
        put(ContextSpecificTag(TAG_COOLING_SETPOINT), optcoolingSetpoint)
      }
      if (heatingSetpoint.isPresent) {
        val optheatingSetpoint = heatingSetpoint.get()
        put(ContextSpecificTag(TAG_HEATING_SETPOINT), optheatingSetpoint)
      }
      if (builtIn != null) {
        put(ContextSpecificTag(TAG_BUILT_IN), builtIn)
      } else {
        putNull(ContextSpecificTag(TAG_BUILT_IN))
      }
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterPresetStruct {
      tlvReader.enterStructure(tlvTag)
      val presetHandle =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PRESET_HANDLE))
          null
        }
      val presetScenario = tlvReader.getUInt(ContextSpecificTag(TAG_PRESET_SCENARIO))
      val name =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NAME))
          null
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
      val builtIn =
        if (!tlvReader.isNull()) {
          tlvReader.getBoolean(ContextSpecificTag(TAG_BUILT_IN))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_BUILT_IN))
          null
        }

      tlvReader.exitContainer()

      return ThermostatClusterPresetStruct(
        presetHandle,
        presetScenario,
        name,
        coolingSetpoint,
        heatingSetpoint,
        builtIn,
      )
    }
  }
}
