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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ThermostatClusterScheduleStruct (
    val scheduleHandle: ByteArray,
    val systemMode: UInt,
    val name: String?,
    val presetHandle: ByteArray,
    val transitions: List<ThermostatClusterScheduleTransitionStruct>,
    val builtIn: Boolean) {
  override fun toString(): String  = buildString {
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
      put(ContextSpecificTag(TAG_SCHEDULE_HANDLE), scheduleHandle)
      put(ContextSpecificTag(TAG_SYSTEM_MODE), systemMode)
      if (name != null) {
      put(ContextSpecificTag(TAG_NAME), name)
    } else {
      putNull(ContextSpecificTag(TAG_NAME))
    }
      put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      startArray(ContextSpecificTag(TAG_TRANSITIONS))
      for (item in transitions.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_BUILT_IN), builtIn)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ThermostatClusterScheduleStruct {
      tlvReader.enterStructure(tlvTag)
      val scheduleHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_SCHEDULE_HANDLE))
      val systemMode = tlvReader.getUInt(ContextSpecificTag(TAG_SYSTEM_MODE))
      val name = if (!tlvReader.isNull()) {
      tlvReader.getString(ContextSpecificTag(TAG_NAME))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_NAME))
      null
    }
      val presetHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
      val transitions = buildList<ThermostatClusterScheduleTransitionStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_TRANSITIONS))
      while(!tlvReader.isEndOfContainer()) {
        add(ThermostatClusterScheduleTransitionStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      val builtIn = tlvReader.getBoolean(ContextSpecificTag(TAG_BUILT_IN))
      
      tlvReader.exitContainer()

      return ThermostatClusterScheduleStruct(scheduleHandle, systemMode, name, presetHandle, transitions, builtIn)
    }
  }
}
