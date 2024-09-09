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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OvenCavityOperationalStateClusterOperationalErrorEvent(
  val errorState:
    chip.devicecontroller.cluster.structs.OvenCavityOperationalStateClusterErrorStateStruct
) {
  override fun toString(): String = buildString {
    append("OvenCavityOperationalStateClusterOperationalErrorEvent {\n")
    append("\terrorState : $errorState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      errorState.toTlv(ContextSpecificTag(TAG_ERROR_STATE), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ERROR_STATE = 0

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): OvenCavityOperationalStateClusterOperationalErrorEvent {
      tlvReader.enterStructure(tlvTag)
      val errorState =
        chip.devicecontroller.cluster.structs.OvenCavityOperationalStateClusterErrorStateStruct
          .fromTlv(ContextSpecificTag(TAG_ERROR_STATE), tlvReader)

      tlvReader.exitContainer()

      return OvenCavityOperationalStateClusterOperationalErrorEvent(errorState)
    }
  }
}
