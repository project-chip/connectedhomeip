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

class ThermostatClusterPresetTypeStruct (
    val presetScenario: UInt,
    val numberOfPresets: UInt,
    val presetTypeFeatures: UInt) {
  override fun toString(): String  = buildString {
    append("ThermostatClusterPresetTypeStruct {\n")
    append("\tpresetScenario : $presetScenario\n")
    append("\tnumberOfPresets : $numberOfPresets\n")
    append("\tpresetTypeFeatures : $presetTypeFeatures\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PRESET_SCENARIO), presetScenario)
      put(ContextSpecificTag(TAG_NUMBER_OF_PRESETS), numberOfPresets)
      put(ContextSpecificTag(TAG_PRESET_TYPE_FEATURES), presetTypeFeatures)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRESET_SCENARIO = 0
    private const val TAG_NUMBER_OF_PRESETS = 1
    private const val TAG_PRESET_TYPE_FEATURES = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ThermostatClusterPresetTypeStruct {
      tlvReader.enterStructure(tlvTag)
      val presetScenario = tlvReader.getUInt(ContextSpecificTag(TAG_PRESET_SCENARIO))
      val numberOfPresets = tlvReader.getUInt(ContextSpecificTag(TAG_NUMBER_OF_PRESETS))
      val presetTypeFeatures = tlvReader.getUInt(ContextSpecificTag(TAG_PRESET_TYPE_FEATURES))
      
      tlvReader.exitContainer()

      return ThermostatClusterPresetTypeStruct(presetScenario, numberOfPresets, presetTypeFeatures)
    }
  }
}
