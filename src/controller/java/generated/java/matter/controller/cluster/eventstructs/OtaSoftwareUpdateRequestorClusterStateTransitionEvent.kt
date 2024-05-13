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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OtaSoftwareUpdateRequestorClusterStateTransitionEvent(
  val previousState: UByte,
  val newState: UByte,
  val reason: UByte,
  val targetSoftwareVersion: UInt?
) {
  override fun toString(): String = buildString {
    append("OtaSoftwareUpdateRequestorClusterStateTransitionEvent {\n")
    append("\tpreviousState : $previousState\n")
    append("\tnewState : $newState\n")
    append("\treason : $reason\n")
    append("\ttargetSoftwareVersion : $targetSoftwareVersion\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PREVIOUS_STATE), previousState)
      put(ContextSpecificTag(TAG_NEW_STATE), newState)
      put(ContextSpecificTag(TAG_REASON), reason)
      if (targetSoftwareVersion != null) {
        put(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION), targetSoftwareVersion)
      } else {
        putNull(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_STATE = 0
    private const val TAG_NEW_STATE = 1
    private const val TAG_REASON = 2
    private const val TAG_TARGET_SOFTWARE_VERSION = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): OtaSoftwareUpdateRequestorClusterStateTransitionEvent {
      tlvReader.enterStructure(tlvTag)
      val previousState = tlvReader.getUByte(ContextSpecificTag(TAG_PREVIOUS_STATE))
      val newState = tlvReader.getUByte(ContextSpecificTag(TAG_NEW_STATE))
      val reason = tlvReader.getUByte(ContextSpecificTag(TAG_REASON))
      val targetSoftwareVersion =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
          null
        }

      tlvReader.exitContainer()

      return OtaSoftwareUpdateRequestorClusterStateTransitionEvent(
        previousState,
        newState,
        reason,
        targetSoftwareVersion
      )
    }
  }
}
