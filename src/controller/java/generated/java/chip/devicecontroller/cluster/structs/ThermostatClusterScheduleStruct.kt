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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterScheduleStruct(
  val scheduleHandle: ByteArray?,
  val systemMode: UInt,
  val name: Optional<String>,
  val presetHandle: Optional<ByteArray>,
  val transitions: List<ThermostatClusterScheduleTransitionStruct>,
  val builtIn: Boolean?,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterScheduleStruct {\n")
    append("\tscheduleHandle : $scheduleHandle\n")
    append("\tsystemMode : $systemMode\n")
    append("\tname : $name\n")
    append("\tpresetHandle : $presetHandle\n")
    append("\ttransitions : $transitions\n")
    append("\tbuiltIn : $builtIn\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (scheduleHandle != null) {
        put(ContextSpecificTag(TAG_SCHEDULE_HANDLE), scheduleHandle)
      } else {
        putNull(ContextSpecificTag(TAG_SCHEDULE_HANDLE))
      }
      put(ContextSpecificTag(TAG_SYSTEM_MODE), systemMode)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      if (presetHandle.isPresent) {
        val optpresetHandle = presetHandle.get()
        put(ContextSpecificTag(TAG_PRESET_HANDLE), optpresetHandle)
      }
      startArray(ContextSpecificTag(TAG_TRANSITIONS))
      for (item in transitions.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      if (builtIn != null) {
        put(ContextSpecificTag(TAG_BUILT_IN), builtIn)
      } else {
        putNull(ContextSpecificTag(TAG_BUILT_IN))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SCHEDULE_HANDLE = 0
    private const val TAG_SYSTEM_MODE = 1
    private const val TAG_NAME = 2
    private const val TAG_PRESET_HANDLE = 3
    private const val TAG_TRANSITIONS = 4
    private const val TAG_BUILT_IN = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterScheduleStruct {
      tlvReader.enterStructure(tlvTag)
      val scheduleHandle =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_SCHEDULE_HANDLE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SCHEDULE_HANDLE))
          null
        }
      val systemMode = tlvReader.getUInt(ContextSpecificTag(TAG_SYSTEM_MODE))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }
      val presetHandle =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRESET_HANDLE))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE)))
        } else {
          Optional.empty()
        }
      val transitions =
        buildList<ThermostatClusterScheduleTransitionStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_TRANSITIONS))
          while (!tlvReader.isEndOfContainer()) {
            add(ThermostatClusterScheduleTransitionStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val builtIn =
        if (!tlvReader.isNull()) {
          tlvReader.getBoolean(ContextSpecificTag(TAG_BUILT_IN))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_BUILT_IN))
          null
        }

      tlvReader.exitContainer()

      return ThermostatClusterScheduleStruct(
        scheduleHandle,
        systemMode,
        name,
        presetHandle,
        transitions,
        builtIn,
      )
    }
  }
}
