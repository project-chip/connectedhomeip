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

class ElectricalGridConditionsClusterElectricalGridConditionsStruct (
    val periodStart: ULong,
    val periodEnd: ULong?,
    val gridCarbonIntensity: Int,
    val gridCarbonLevel: UInt,
    val localCarbonIntensity: Int,
    val localCarbonLevel: UInt) {
  override fun toString(): String  = buildString {
    append("ElectricalGridConditionsClusterElectricalGridConditionsStruct {\n")
    append("\tperiodStart : $periodStart\n")
    append("\tperiodEnd : $periodEnd\n")
    append("\tgridCarbonIntensity : $gridCarbonIntensity\n")
    append("\tgridCarbonLevel : $gridCarbonLevel\n")
    append("\tlocalCarbonIntensity : $localCarbonIntensity\n")
    append("\tlocalCarbonLevel : $localCarbonLevel\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PERIOD_START), periodStart)
      if (periodEnd != null) {
      put(ContextSpecificTag(TAG_PERIOD_END), periodEnd)
    } else {
      putNull(ContextSpecificTag(TAG_PERIOD_END))
    }
      put(ContextSpecificTag(TAG_GRID_CARBON_INTENSITY), gridCarbonIntensity)
      put(ContextSpecificTag(TAG_GRID_CARBON_LEVEL), gridCarbonLevel)
      put(ContextSpecificTag(TAG_LOCAL_CARBON_INTENSITY), localCarbonIntensity)
      put(ContextSpecificTag(TAG_LOCAL_CARBON_LEVEL), localCarbonLevel)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PERIOD_START = 0
    private const val TAG_PERIOD_END = 1
    private const val TAG_GRID_CARBON_INTENSITY = 2
    private const val TAG_GRID_CARBON_LEVEL = 3
    private const val TAG_LOCAL_CARBON_INTENSITY = 4
    private const val TAG_LOCAL_CARBON_LEVEL = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ElectricalGridConditionsClusterElectricalGridConditionsStruct {
      tlvReader.enterStructure(tlvTag)
      val periodStart = tlvReader.getULong(ContextSpecificTag(TAG_PERIOD_START))
      val periodEnd = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_PERIOD_END))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_PERIOD_END))
      null
    }
      val gridCarbonIntensity = tlvReader.getInt(ContextSpecificTag(TAG_GRID_CARBON_INTENSITY))
      val gridCarbonLevel = tlvReader.getUInt(ContextSpecificTag(TAG_GRID_CARBON_LEVEL))
      val localCarbonIntensity = tlvReader.getInt(ContextSpecificTag(TAG_LOCAL_CARBON_INTENSITY))
      val localCarbonLevel = tlvReader.getUInt(ContextSpecificTag(TAG_LOCAL_CARBON_LEVEL))
      
      tlvReader.exitContainer()

      return ElectricalGridConditionsClusterElectricalGridConditionsStruct(periodStart, periodEnd, gridCarbonIntensity, gridCarbonLevel, localCarbonIntensity, localCarbonLevel)
    }
  }
}
