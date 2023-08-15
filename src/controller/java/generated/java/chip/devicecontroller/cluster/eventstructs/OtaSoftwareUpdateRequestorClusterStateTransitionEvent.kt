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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class OtaSoftwareUpdateRequestorClusterStateTransitionEvent(
  val previousState: Int,
  val newState: Int,
  val reason: Int,
  val targetSoftwareVersion: Long?
) {
  override fun toString(): String = buildString {
    append("OtaSoftwareUpdateRequestorClusterStateTransitionEvent {\n")
    append("\tpreviousState : $previousState\n")
    append("\tnewState : $newState\n")
    append("\treason : $reason\n")
    append("\ttargetSoftwareVersion : $targetSoftwareVersion\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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
      tag: Tag,
      tlvReader: TlvReader
    ): OtaSoftwareUpdateRequestorClusterStateTransitionEvent {
      tlvReader.enterStructure(tag)
      val previousState = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_STATE))
      val newState = tlvReader.getInt(ContextSpecificTag(TAG_NEW_STATE))
      val reason = tlvReader.getInt(ContextSpecificTag(TAG_REASON))
      val targetSoftwareVersion =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
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
