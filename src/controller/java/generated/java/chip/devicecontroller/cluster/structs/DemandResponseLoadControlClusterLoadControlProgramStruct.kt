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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandResponseLoadControlClusterLoadControlProgramStruct(
  val programID: ByteArray,
  val name: String,
  val enrollmentGroup: UInt?,
  val randomStartMinutes: UInt?,
  val randomDurationMinutes: UInt?,
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterLoadControlProgramStruct {\n")
    append("\tprogramID : $programID\n")
    append("\tname : $name\n")
    append("\tenrollmentGroup : $enrollmentGroup\n")
    append("\trandomStartMinutes : $randomStartMinutes\n")
    append("\trandomDurationMinutes : $randomDurationMinutes\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PROGRAM_ID), programID)
      put(ContextSpecificTag(TAG_NAME), name)
      if (enrollmentGroup != null) {
        put(ContextSpecificTag(TAG_ENROLLMENT_GROUP), enrollmentGroup)
      } else {
        putNull(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
      }
      if (randomStartMinutes != null) {
        put(ContextSpecificTag(TAG_RANDOM_START_MINUTES), randomStartMinutes)
      } else {
        putNull(ContextSpecificTag(TAG_RANDOM_START_MINUTES))
      }
      if (randomDurationMinutes != null) {
        put(ContextSpecificTag(TAG_RANDOM_DURATION_MINUTES), randomDurationMinutes)
      } else {
        putNull(ContextSpecificTag(TAG_RANDOM_DURATION_MINUTES))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PROGRAM_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_ENROLLMENT_GROUP = 2
    private const val TAG_RANDOM_START_MINUTES = 3
    private const val TAG_RANDOM_DURATION_MINUTES = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): DemandResponseLoadControlClusterLoadControlProgramStruct {
      tlvReader.enterStructure(tlvTag)
      val programID = tlvReader.getByteArray(ContextSpecificTag(TAG_PROGRAM_ID))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val enrollmentGroup =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
          null
        }
      val randomStartMinutes =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_RANDOM_START_MINUTES))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RANDOM_START_MINUTES))
          null
        }
      val randomDurationMinutes =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_RANDOM_DURATION_MINUTES))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_RANDOM_DURATION_MINUTES))
          null
        }

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterLoadControlProgramStruct(
        programID,
        name,
        enrollmentGroup,
        randomStartMinutes,
        randomDurationMinutes,
      )
    }
  }
}
