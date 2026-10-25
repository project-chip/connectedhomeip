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

class ProximityRangingClusterRangingConstraintStruct(
  val technology: UInt,
  val role: UInt,
  val enabled: Optional<Boolean>,
  val minRangingInterval: Optional<ULong>,
  val maxSessionDuration: Optional<ULong>,
  val maxRangingInstances: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingConstraintStruct {\n")
    append("\ttechnology : $technology\n")
    append("\trole : $role\n")
    append("\tenabled : $enabled\n")
    append("\tminRangingInterval : $minRangingInterval\n")
    append("\tmaxSessionDuration : $maxSessionDuration\n")
    append("\tmaxRangingInstances : $maxRangingInstances\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TECHNOLOGY), technology)
      put(ContextSpecificTag(TAG_ROLE), role)
      if (enabled.isPresent) {
        val optenabled = enabled.get()
        put(ContextSpecificTag(TAG_ENABLED), optenabled)
      }
      if (minRangingInterval.isPresent) {
        val optminRangingInterval = minRangingInterval.get()
        put(ContextSpecificTag(TAG_MIN_RANGING_INTERVAL), optminRangingInterval)
      }
      if (maxSessionDuration.isPresent) {
        val optmaxSessionDuration = maxSessionDuration.get()
        put(ContextSpecificTag(TAG_MAX_SESSION_DURATION), optmaxSessionDuration)
      }
      if (maxRangingInstances.isPresent) {
        val optmaxRangingInstances = maxRangingInstances.get()
        put(ContextSpecificTag(TAG_MAX_RANGING_INSTANCES), optmaxRangingInstances)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TECHNOLOGY = 0
    private const val TAG_ROLE = 1
    private const val TAG_ENABLED = 3
    private const val TAG_MIN_RANGING_INTERVAL = 4
    private const val TAG_MAX_SESSION_DURATION = 5
    private const val TAG_MAX_RANGING_INSTANCES = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ProximityRangingClusterRangingConstraintStruct {
      tlvReader.enterStructure(tlvTag)
      val technology = tlvReader.getUInt(ContextSpecificTag(TAG_TECHNOLOGY))
      val role = tlvReader.getUInt(ContextSpecificTag(TAG_ROLE))
      val enabled =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENABLED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_ENABLED)))
        } else {
          Optional.empty()
        }
      val minRangingInterval =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_RANGING_INTERVAL))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MIN_RANGING_INTERVAL)))
        } else {
          Optional.empty()
        }
      val maxSessionDuration =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_SESSION_DURATION))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MAX_SESSION_DURATION)))
        } else {
          Optional.empty()
        }
      val maxRangingInstances =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_RANGING_INSTANCES))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_RANGING_INSTANCES)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingConstraintStruct(
        technology,
        role,
        enabled,
        minRangingInterval,
        maxSessionDuration,
        maxRangingInstances,
      )
    }
  }
}
