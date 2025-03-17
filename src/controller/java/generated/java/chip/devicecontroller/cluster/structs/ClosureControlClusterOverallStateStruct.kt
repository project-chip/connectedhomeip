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

class ClosureControlClusterOverallStateStruct(
  val positioning: Optional<UInt>,
  val latching: Optional<UInt>,
  val speed: Optional<UInt>,
  val extraInfo: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ClosureControlClusterOverallStateStruct {\n")
    append("\tpositioning : $positioning\n")
    append("\tlatching : $latching\n")
    append("\tspeed : $speed\n")
    append("\textraInfo : $extraInfo\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (positioning.isPresent) {
        val optpositioning = positioning.get()
        put(ContextSpecificTag(TAG_POSITIONING), optpositioning)
      }
      if (latching.isPresent) {
        val optlatching = latching.get()
        put(ContextSpecificTag(TAG_LATCHING), optlatching)
      }
      if (speed.isPresent) {
        val optspeed = speed.get()
        put(ContextSpecificTag(TAG_SPEED), optspeed)
      }
      if (extraInfo.isPresent) {
        val optextraInfo = extraInfo.get()
        put(ContextSpecificTag(TAG_EXTRA_INFO), optextraInfo)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_POSITIONING = 0
    private const val TAG_LATCHING = 1
    private const val TAG_SPEED = 2
    private const val TAG_EXTRA_INFO = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ClosureControlClusterOverallStateStruct {
      tlvReader.enterStructure(tlvTag)
      val positioning =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSITIONING))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_POSITIONING)))
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
      val extraInfo =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTRA_INFO))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EXTRA_INFO)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ClosureControlClusterOverallStateStruct(positioning, latching, speed, extraInfo)
    }
  }
}
