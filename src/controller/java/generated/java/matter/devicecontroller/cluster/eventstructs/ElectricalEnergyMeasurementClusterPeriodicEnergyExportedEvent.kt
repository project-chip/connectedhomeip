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
package matter.devicecontroller.cluster.eventstructs

import matter.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ElectricalEnergyMeasurementClusterPeriodicEnergyExportedEvent (
    val periodStart: UInt,
    val periodEnd: UInt,
    val energyExported: ULong) {
  override fun toString(): String  = buildString {
    append("ElectricalEnergyMeasurementClusterPeriodicEnergyExportedEvent {\n")
    append("\tperiodStart : $periodStart\n")
    append("\tperiodEnd : $periodEnd\n")
    append("\tenergyExported : $energyExported\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PERIOD_START), periodStart)
      put(ContextSpecificTag(TAG_PERIOD_END), periodEnd)
      put(ContextSpecificTag(TAG_ENERGY_EXPORTED), energyExported)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PERIOD_START = 0
    private const val TAG_PERIOD_END = 1
    private const val TAG_ENERGY_EXPORTED = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ElectricalEnergyMeasurementClusterPeriodicEnergyExportedEvent {
      tlvReader.enterStructure(tlvTag)
      val periodStart = tlvReader.getUInt(ContextSpecificTag(TAG_PERIOD_START))
      val periodEnd = tlvReader.getUInt(ContextSpecificTag(TAG_PERIOD_END))
      val energyExported = tlvReader.getULong(ContextSpecificTag(TAG_ENERGY_EXPORTED))
      
      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterPeriodicEnergyExportedEvent(periodStart, periodEnd, energyExported)
    }
  }
}
