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

class ElectricalEnergyMeasurementClusterCumulativeEnergyImportedEvent (
    val importedTimestamp: UInt,
    val energyImported: ULong) {
  override fun toString(): String  = buildString {
    append("ElectricalEnergyMeasurementClusterCumulativeEnergyImportedEvent {\n")
    append("\timportedTimestamp : $importedTimestamp\n")
    append("\tenergyImported : $energyImported\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_IMPORTED_TIMESTAMP), importedTimestamp)
      put(ContextSpecificTag(TAG_ENERGY_IMPORTED), energyImported)
      endStructure()
    }
  }

  companion object {
    private const val TAG_IMPORTED_TIMESTAMP = 0
    private const val TAG_ENERGY_IMPORTED = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ElectricalEnergyMeasurementClusterCumulativeEnergyImportedEvent {
      tlvReader.enterStructure(tlvTag)
      val importedTimestamp = tlvReader.getUInt(ContextSpecificTag(TAG_IMPORTED_TIMESTAMP))
      val energyImported = tlvReader.getULong(ContextSpecificTag(TAG_ENERGY_IMPORTED))
      
      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterCumulativeEnergyImportedEvent(importedTimestamp, energyImported)
    }
  }
}
