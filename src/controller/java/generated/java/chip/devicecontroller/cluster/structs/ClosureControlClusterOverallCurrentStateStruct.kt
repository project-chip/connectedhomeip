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

class ClosureControlClusterOverallCurrentStateStruct(
  val position: Optional<UInt>?,
  val latch: Optional<Boolean>?,
  val speed: Optional<UInt>,
  val secureState: Optional<Boolean>?,
) {
  override fun toString(): String = buildString {
    append("ClosureControlClusterOverallCurrentStateStruct {\n")
    append("\tposition : $position\n")
    append("\tlatch : $latch\n")
    append("\tspeed : $speed\n")
    append("\tsecureState : $secureState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (position != null) {
        if (position.isPresent) {
          val optposition = position.get()
          put(ContextSpecificTag(TAG_POSITION), optposition)
        }
      } else {
        putNull(ContextSpecificTag(TAG_POSITION))
      }
      if (latch != null) {
        if (latch.isPresent) {
          val optlatch = latch.get()
          put(ContextSpecificTag(TAG_LATCH), optlatch)
        }
      } else {
        putNull(ContextSpecificTag(TAG_LATCH))
      }
      if (speed.isPresent) {
        val optspeed = speed.get()
        put(ContextSpecificTag(TAG_SPEED), optspeed)
      }
      if (secureState != null) {
        if (secureState.isPresent) {
          val optsecureState = secureState.get()
          put(ContextSpecificTag(TAG_SECURE_STATE), optsecureState)
        }
      } else {
        putNull(ContextSpecificTag(TAG_SECURE_STATE))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_POSITION = 0
    private const val TAG_LATCH = 1
    private const val TAG_SPEED = 2
    private const val TAG_SECURE_STATE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ClosureControlClusterOverallCurrentStateStruct {
      tlvReader.enterStructure(tlvTag)
      val position =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSITION))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_POSITION)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_POSITION))
          null
        }
      val latch =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_LATCH))) {
            Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_LATCH)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LATCH))
          null
        }
      val speed =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SPEED))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SPEED)))
        } else {
          Optional.empty()
        }
      val secureState =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_SECURE_STATE))) {
            Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_SECURE_STATE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SECURE_STATE))
          null
        }

      tlvReader.exitContainer()

      return ClosureControlClusterOverallCurrentStateStruct(position, latch, speed, secureState)
    }
  }
}
