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

class DemandResponseLoadControlClusterTemperatureControlStruct(
  val coolingTempOffset: Optional<UInt>?,
  val heatingtTempOffset: Optional<UInt>?,
  val coolingTempSetpoint: Optional<Int>?,
  val heatingTempSetpoint: Optional<Int>?
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterTemperatureControlStruct {\n")
    append("\tcoolingTempOffset : $coolingTempOffset\n")
    append("\theatingtTempOffset : $heatingtTempOffset\n")
    append("\tcoolingTempSetpoint : $coolingTempSetpoint\n")
    append("\theatingTempSetpoint : $heatingTempSetpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (coolingTempOffset != null) {
        if (coolingTempOffset.isPresent) {
          val optcoolingTempOffset = coolingTempOffset.get()
          put(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET), optcoolingTempOffset)
        }
      } else {
        putNull(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))
      }
      if (heatingtTempOffset != null) {
        if (heatingtTempOffset.isPresent) {
          val optheatingtTempOffset = heatingtTempOffset.get()
          put(ContextSpecificTag(TAG_HEATINGT_TEMP_OFFSET), optheatingtTempOffset)
        }
      } else {
        putNull(ContextSpecificTag(TAG_HEATINGT_TEMP_OFFSET))
      }
      if (coolingTempSetpoint != null) {
        if (coolingTempSetpoint.isPresent) {
          val optcoolingTempSetpoint = coolingTempSetpoint.get()
          put(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT), optcoolingTempSetpoint)
        }
      } else {
        putNull(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))
      }
      if (heatingTempSetpoint != null) {
        if (heatingTempSetpoint.isPresent) {
          val optheatingTempSetpoint = heatingTempSetpoint.get()
          put(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT), optheatingTempSetpoint)
        }
      } else {
        putNull(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_COOLING_TEMP_OFFSET = 0
    private const val TAG_HEATINGT_TEMP_OFFSET = 1
    private const val TAG_COOLING_TEMP_SETPOINT = 2
    private const val TAG_HEATING_TEMP_SETPOINT = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DemandResponseLoadControlClusterTemperatureControlStruct {
      tlvReader.enterStructure(tlvTag)
      val coolingTempOffset =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_COOLING_TEMP_OFFSET))
          null
        }
      val heatingtTempOffset =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_HEATINGT_TEMP_OFFSET))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_HEATINGT_TEMP_OFFSET)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HEATINGT_TEMP_OFFSET))
          null
        }
      val coolingTempSetpoint =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))) {
            Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_COOLING_TEMP_SETPOINT))
          null
        }
      val heatingTempSetpoint =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))) {
            Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HEATING_TEMP_SETPOINT))
          null
        }

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterTemperatureControlStruct(
        coolingTempOffset,
        heatingtTempOffset,
        coolingTempSetpoint,
        heatingTempSetpoint
      )
    }
  }
}
