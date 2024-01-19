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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandResponseLoadControlClusterLoadControlEventStruct(
  val eventID: ByteArray,
  val programID: ByteArray?,
  val control: UShort,
  val deviceClass: UInt,
  val enrollmentGroup: Optional<UByte>,
  val criticality: UByte,
  val startTime: UInt?,
  val transitions: List<DemandResponseLoadControlClusterLoadControlEventTransitionStruct>
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterLoadControlEventStruct {\n")
    append("\teventID : $eventID\n")
    append("\tprogramID : $programID\n")
    append("\tcontrol : $control\n")
    append("\tdeviceClass : $deviceClass\n")
    append("\tenrollmentGroup : $enrollmentGroup\n")
    append("\tcriticality : $criticality\n")
    append("\tstartTime : $startTime\n")
    append("\ttransitions : $transitions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EVENT_I_D), eventID)
      if (programID != null) {
        put(ContextSpecificTag(TAG_PROGRAM_I_D), programID)
      } else {
        putNull(ContextSpecificTag(TAG_PROGRAM_I_D))
      }
      put(ContextSpecificTag(TAG_CONTROL), control)
      put(ContextSpecificTag(TAG_DEVICE_CLASS), deviceClass)
      if (enrollmentGroup.isPresent) {
        val optenrollmentGroup = enrollmentGroup.get()
        put(ContextSpecificTag(TAG_ENROLLMENT_GROUP), optenrollmentGroup)
      }
      put(ContextSpecificTag(TAG_CRITICALITY), criticality)
      if (startTime != null) {
        put(ContextSpecificTag(TAG_START_TIME), startTime)
      } else {
        putNull(ContextSpecificTag(TAG_START_TIME))
      }
      startArray(ContextSpecificTag(TAG_TRANSITIONS))
      for (item in transitions.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_EVENT_I_D = 0
    private const val TAG_PROGRAM_I_D = 1
    private const val TAG_CONTROL = 2
    private const val TAG_DEVICE_CLASS = 3
    private const val TAG_ENROLLMENT_GROUP = 4
    private const val TAG_CRITICALITY = 5
    private const val TAG_START_TIME = 6
    private const val TAG_TRANSITIONS = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DemandResponseLoadControlClusterLoadControlEventStruct {
      tlvReader.enterStructure(tlvTag)
      val eventID = tlvReader.getByteArray(ContextSpecificTag(TAG_EVENT_I_D))
      val programID =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_PROGRAM_I_D))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PROGRAM_I_D))
          null
        }
      val control = tlvReader.getUShort(ContextSpecificTag(TAG_CONTROL))
      val deviceClass = tlvReader.getUInt(ContextSpecificTag(TAG_DEVICE_CLASS))
      val enrollmentGroup =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENROLLMENT_GROUP))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_ENROLLMENT_GROUP)))
        } else {
          Optional.empty()
        }
      val criticality = tlvReader.getUByte(ContextSpecificTag(TAG_CRITICALITY))
      val startTime =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_START_TIME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_START_TIME))
          null
        }
      val transitions =
        buildList<DemandResponseLoadControlClusterLoadControlEventTransitionStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_TRANSITIONS))
          while (!tlvReader.isEndOfContainer()) {
            add(
              DemandResponseLoadControlClusterLoadControlEventTransitionStruct.fromTlv(
                AnonymousTag,
                tlvReader
              )
            )
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterLoadControlEventStruct(
        eventID,
        programID,
        control,
        deviceClass,
        enrollmentGroup,
        criticality,
        startTime,
        transitions
      )
    }
  }
}
