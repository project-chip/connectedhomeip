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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DemandResponseLoadControlClusterScopedLoadControlEventStruct(
  val eventID: ByteArray,
  val programControl: ByteArray,
  val control: UInt,
  val deviceClass: ULong,
  val enrollmentGroup: UInt?,
  val criticality: UInt,
  val startTime: ULong?,
  val transitions: List<DemandResponseLoadControlClusterLoadControlEventTransitionStruct>,
  val fabricIndex: UInt
) {
  override fun toString(): String = buildString {
    append("DemandResponseLoadControlClusterScopedLoadControlEventStruct {\n")
    append("\teventID : $eventID\n")
    append("\tprogramControl : $programControl\n")
    append("\tcontrol : $control\n")
    append("\tdeviceClass : $deviceClass\n")
    append("\tenrollmentGroup : $enrollmentGroup\n")
    append("\tcriticality : $criticality\n")
    append("\tstartTime : $startTime\n")
    append("\ttransitions : $transitions\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EVENT_I_D), eventID)
      put(ContextSpecificTag(TAG_PROGRAM_CONTROL), programControl)
      put(ContextSpecificTag(TAG_CONTROL), control)
      put(ContextSpecificTag(TAG_DEVICE_CLASS), deviceClass)
      if (enrollmentGroup != null) {
        put(ContextSpecificTag(TAG_ENROLLMENT_GROUP), enrollmentGroup)
      } else {
        putNull(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
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
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EVENT_I_D = 0
    private const val TAG_PROGRAM_CONTROL = 1
    private const val TAG_CONTROL = 2
    private const val TAG_DEVICE_CLASS = 3
    private const val TAG_ENROLLMENT_GROUP = 4
    private const val TAG_CRITICALITY = 5
    private const val TAG_START_TIME = 6
    private const val TAG_TRANSITIONS = 7
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DemandResponseLoadControlClusterScopedLoadControlEventStruct {
      tlvReader.enterStructure(tlvTag)
      val eventID = tlvReader.getByteArray(ContextSpecificTag(TAG_EVENT_I_D))
      val programControl = tlvReader.getByteArray(ContextSpecificTag(TAG_PROGRAM_CONTROL))
      val control = tlvReader.getUInt(ContextSpecificTag(TAG_CONTROL))
      val deviceClass = tlvReader.getULong(ContextSpecificTag(TAG_DEVICE_CLASS))
      val enrollmentGroup =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ENROLLMENT_GROUP))
          null
        }
      val criticality = tlvReader.getUInt(ContextSpecificTag(TAG_CRITICALITY))
      val startTime =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
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
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return DemandResponseLoadControlClusterScopedLoadControlEventStruct(
        eventID,
        programControl,
        control,
        deviceClass,
        enrollmentGroup,
        criticality,
        startTime,
        transitions,
        fabricIndex
      )
    }
  }
}
