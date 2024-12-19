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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterSetpointChangeEvent(
  val systemMode: UInt,
  val occupancy: UInt,
  val previousSetpoint: Int,
  val currentSetpoint: Int,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterSetpointChangeEvent {\n")
    append("\tsystemMode : $systemMode\n")
    append("\toccupancy : $occupancy\n")
    append("\tpreviousSetpoint : $previousSetpoint\n")
    append("\tcurrentSetpoint : $currentSetpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SYSTEM_MODE), systemMode)
      put(ContextSpecificTag(TAG_OCCUPANCY), occupancy)
      put(ContextSpecificTag(TAG_PREVIOUS_SETPOINT), previousSetpoint)
      put(ContextSpecificTag(TAG_CURRENT_SETPOINT), currentSetpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SYSTEM_MODE = 0
    private const val TAG_OCCUPANCY = 1
    private const val TAG_PREVIOUS_SETPOINT = 2
    private const val TAG_CURRENT_SETPOINT = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterSetpointChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val systemMode = tlvReader.getUInt(ContextSpecificTag(TAG_SYSTEM_MODE))
      val occupancy = tlvReader.getUInt(ContextSpecificTag(TAG_OCCUPANCY))
      val previousSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_SETPOINT))
      val currentSetpoint = tlvReader.getInt(ContextSpecificTag(TAG_CURRENT_SETPOINT))

      tlvReader.exitContainer()

      return ThermostatClusterSetpointChangeEvent(
        systemMode,
        occupancy,
        previousSetpoint,
        currentSetpoint,
      )
    }
  }
}
