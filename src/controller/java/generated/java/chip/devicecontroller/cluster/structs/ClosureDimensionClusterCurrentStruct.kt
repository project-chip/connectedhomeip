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

class ClosureDimensionClusterCurrentStruct(
  val position: Optional<UInt>,
  val latching: Optional<UInt>,
  val speed: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ClosureDimensionClusterCurrentStruct {\n")
    append("\tposition : $position\n")
    append("\tlatching : $latching\n")
    append("\tspeed : $speed\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (position.isPresent) {
        val optposition = position.get()
        put(ContextSpecificTag(TAG_POSITION), optposition)
      }
      if (latching.isPresent) {
        val optlatching = latching.get()
        put(ContextSpecificTag(TAG_LATCHING), optlatching)
      }
      if (speed.isPresent) {
        val optspeed = speed.get()
        put(ContextSpecificTag(TAG_SPEED), optspeed)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_POSITION = 0
    private const val TAG_LATCHING = 1
    private const val TAG_SPEED = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ClosureDimensionClusterCurrentStruct {
      tlvReader.enterStructure(tlvTag)
      val position =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSITION))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_POSITION)))
        } else {
          Optional.empty()
        }
      val latching =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LATCHING))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_LATCHING)))
        } else {
          Optional.empty()
        }
      val speed =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SPEED))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SPEED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ClosureDimensionClusterCurrentStruct(position, latching, speed)
    }
  }
}
