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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ClosureControlClusterOverallStateStruct(
  val positioning: Optional<UByte>?,
  val latch: Optional<Boolean>?,
  val speed: Optional<UByte>?,
  val secureState: Optional<Boolean>?,
) {
  override fun toString(): String = buildString {
    append("ClosureControlClusterOverallStateStruct {\n")
    append("\tpositioning : $positioning\n")
    append("\tlatch : $latch\n")
    append("\tspeed : $speed\n")
    append("\tsecureState : $secureState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (positioning != null) {
        if (positioning.isPresent) {
          val optpositioning = positioning.get()
          put(ContextSpecificTag(TAG_POSITIONING), optpositioning)
        }
      } else {
        putNull(ContextSpecificTag(TAG_POSITIONING))
      }
      if (latch != null) {
        if (latch.isPresent) {
          val optlatch = latch.get()
          put(ContextSpecificTag(TAG_LATCH), optlatch)
        }
      } else {
        putNull(ContextSpecificTag(TAG_LATCH))
      }
      if (speed != null) {
        if (speed.isPresent) {
          val optspeed = speed.get()
          put(ContextSpecificTag(TAG_SPEED), optspeed)
        }
      } else {
        putNull(ContextSpecificTag(TAG_SPEED))
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
    private const val TAG_POSITIONING = 0
    private const val TAG_LATCH = 1
    private const val TAG_SPEED = 2
    private const val TAG_SECURE_STATE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ClosureControlClusterOverallStateStruct {
      tlvReader.enterStructure(tlvTag)
      val positioning =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSITIONING))) {
            Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_POSITIONING)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_POSITIONING))
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
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_SPEED))) {
            Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_SPEED)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SPEED))
          null
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

      return ClosureControlClusterOverallStateStruct(positioning, latch, speed, secureState)
    }
  }
}
