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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ProximityRangingClusterRangingCapabilitiesStruct(
  val technology: UByte,
  val frequencyBand: UShort,
  val periodicRangingSupport: Boolean,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingCapabilitiesStruct {\n")
    append("\ttechnology : $technology\n")
    append("\tfrequencyBand : $frequencyBand\n")
    append("\tperiodicRangingSupport : $periodicRangingSupport\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TECHNOLOGY), technology)
      put(ContextSpecificTag(TAG_FREQUENCY_BAND), frequencyBand)
      put(ContextSpecificTag(TAG_PERIODIC_RANGING_SUPPORT), periodicRangingSupport)
      endStructure()
    }
  }

  companion object {
    private const val TAG_TECHNOLOGY = 0
    private const val TAG_FREQUENCY_BAND = 1
    private const val TAG_PERIODIC_RANGING_SUPPORT = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterRangingCapabilitiesStruct {
      tlvReader.enterStructure(tlvTag)
      val technology = tlvReader.getUByte(ContextSpecificTag(TAG_TECHNOLOGY))
      val frequencyBand = tlvReader.getUShort(ContextSpecificTag(TAG_FREQUENCY_BAND))
      val periodicRangingSupport =
        tlvReader.getBoolean(ContextSpecificTag(TAG_PERIODIC_RANGING_SUPPORT))

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingCapabilitiesStruct(
        technology,
        frequencyBand,
        periodicRangingSupport,
      )
    }
  }
}
