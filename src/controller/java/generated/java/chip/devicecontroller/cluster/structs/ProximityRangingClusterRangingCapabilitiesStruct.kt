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

class ProximityRangingClusterRangingCapabilitiesStruct(
  val technology: UInt,
  val frequencyBand: UInt,
  val bandwidth: ULong,
  val supportedRangingRoles: UInt,
  val RDRCapability: UInt,
  val periodicRangingSupport: Boolean,
  val maxConcurrentSessions: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingCapabilitiesStruct {\n")
    append("\ttechnology : $technology\n")
    append("\tfrequencyBand : $frequencyBand\n")
    append("\tbandwidth : $bandwidth\n")
    append("\tsupportedRangingRoles : $supportedRangingRoles\n")
    append("\tRDRCapability : $RDRCapability\n")
    append("\tperiodicRangingSupport : $periodicRangingSupport\n")
    append("\tmaxConcurrentSessions : $maxConcurrentSessions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TECHNOLOGY), technology)
      put(ContextSpecificTag(TAG_FREQUENCY_BAND), frequencyBand)
      put(ContextSpecificTag(TAG_BANDWIDTH), bandwidth)
      put(ContextSpecificTag(TAG_SUPPORTED_RANGING_ROLES), supportedRangingRoles)
      put(ContextSpecificTag(TAG_RDR_CAPABILITY), RDRCapability)
      put(ContextSpecificTag(TAG_PERIODIC_RANGING_SUPPORT), periodicRangingSupport)
      if (maxConcurrentSessions.isPresent) {
        val optmaxConcurrentSessions = maxConcurrentSessions.get()
        put(ContextSpecificTag(TAG_MAX_CONCURRENT_SESSIONS), optmaxConcurrentSessions)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TECHNOLOGY = 0
    private const val TAG_FREQUENCY_BAND = 1
    private const val TAG_BANDWIDTH = 2
    private const val TAG_SUPPORTED_RANGING_ROLES = 3
    private const val TAG_RDR_CAPABILITY = 4
    private const val TAG_PERIODIC_RANGING_SUPPORT = 5
    private const val TAG_MAX_CONCURRENT_SESSIONS = 6

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterRangingCapabilitiesStruct {
      tlvReader.enterStructure(tlvTag)
      val technology = tlvReader.getUInt(ContextSpecificTag(TAG_TECHNOLOGY))
      val frequencyBand = tlvReader.getUInt(ContextSpecificTag(TAG_FREQUENCY_BAND))
      val bandwidth = tlvReader.getULong(ContextSpecificTag(TAG_BANDWIDTH))
      val supportedRangingRoles = tlvReader.getUInt(ContextSpecificTag(TAG_SUPPORTED_RANGING_ROLES))
      val RDRCapability = tlvReader.getUInt(ContextSpecificTag(TAG_RDR_CAPABILITY))
      val periodicRangingSupport =
        tlvReader.getBoolean(ContextSpecificTag(TAG_PERIODIC_RANGING_SUPPORT))
      val maxConcurrentSessions =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_CONCURRENT_SESSIONS))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_CONCURRENT_SESSIONS)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingCapabilitiesStruct(
        technology,
        frequencyBand,
        bandwidth,
        supportedRangingRoles,
        RDRCapability,
        periodicRangingSupport,
        maxConcurrentSessions,
      )
    }
  }
}
