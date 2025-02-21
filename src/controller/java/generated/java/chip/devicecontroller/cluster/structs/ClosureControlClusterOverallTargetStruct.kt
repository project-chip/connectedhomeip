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

class ClosureControlClusterOverallTargetStruct(
  val tagPosition: Optional<UInt>,
  val tagLatch: Optional<UInt>,
  val speed: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ClosureControlClusterOverallTargetStruct {\n")
    append("\ttagPosition : $tagPosition\n")
    append("\ttagLatch : $tagLatch\n")
    append("\tspeed : $speed\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tagPosition.isPresent) {
        val opttagPosition = tagPosition.get()
        put(ContextSpecificTag(TAG_TAG_POSITION), opttagPosition)
      }
      if (tagLatch.isPresent) {
        val opttagLatch = tagLatch.get()
        put(ContextSpecificTag(TAG_TAG_LATCH), opttagLatch)
      }
      if (speed.isPresent) {
        val optspeed = speed.get()
        put(ContextSpecificTag(TAG_SPEED), optspeed)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TAG_POSITION = 0
    private const val TAG_TAG_LATCH = 1
    private const val TAG_SPEED = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ClosureControlClusterOverallTargetStruct {
      tlvReader.enterStructure(tlvTag)
      val tagPosition =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TAG_POSITION))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TAG_POSITION)))
        } else {
          Optional.empty()
        }
      val tagLatch =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TAG_LATCH))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TAG_LATCH)))
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

      return ClosureControlClusterOverallTargetStruct(tagPosition, tagLatch, speed)
    }
  }
}
