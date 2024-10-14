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

class DeviceEnergyManagementClusterConstraintsStruct(
  val startTime: ULong,
  val duration: ULong,
  val nominalPower: Optional<Long>,
  val maximumEnergy: Optional<Long>,
  val loadControl: Optional<Int>,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterConstraintsStruct {\n")
    append("\tstartTime : $startTime\n")
    append("\tduration : $duration\n")
    append("\tnominalPower : $nominalPower\n")
    append("\tmaximumEnergy : $maximumEnergy\n")
    append("\tloadControl : $loadControl\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_DURATION), duration)
      if (nominalPower.isPresent) {
        val optnominalPower = nominalPower.get()
        put(ContextSpecificTag(TAG_NOMINAL_POWER), optnominalPower)
      }
      if (maximumEnergy.isPresent) {
        val optmaximumEnergy = maximumEnergy.get()
        put(ContextSpecificTag(TAG_MAXIMUM_ENERGY), optmaximumEnergy)
      }
      if (loadControl.isPresent) {
        val optloadControl = loadControl.get()
        put(ContextSpecificTag(TAG_LOAD_CONTROL), optloadControl)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_TIME = 0
    private const val TAG_DURATION = 1
    private const val TAG_NOMINAL_POWER = 2
    private const val TAG_MAXIMUM_ENERGY = 3
    private const val TAG_LOAD_CONTROL = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DeviceEnergyManagementClusterConstraintsStruct {
      tlvReader.enterStructure(tlvTag)
      val startTime = tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))
      val nominalPower =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NOMINAL_POWER))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_NOMINAL_POWER)))
        } else {
          Optional.empty()
        }
      val maximumEnergy =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAXIMUM_ENERGY))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAXIMUM_ENERGY)))
        } else {
          Optional.empty()
        }
      val loadControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOAD_CONTROL))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_LOAD_CONTROL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterConstraintsStruct(
        startTime,
        duration,
        nominalPower,
        maximumEnergy,
        loadControl,
      )
    }
  }
}
