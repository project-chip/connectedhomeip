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

class CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct (
    val number: UInt,
    val requiredState: UInt) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct {\n")
    append("\tnumber : $number\n")
    append("\trequiredState : $requiredState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NUMBER), number)
      put(ContextSpecificTag(TAG_REQUIRED_STATE), requiredState)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NUMBER = 0
    private const val TAG_REQUIRED_STATE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct {
      tlvReader.enterStructure(tlvTag)
      val number = tlvReader.getUInt(ContextSpecificTag(TAG_NUMBER))
      val requiredState = tlvReader.getUInt(ContextSpecificTag(TAG_REQUIRED_STATE))
      
      tlvReader.exitContainer()

      return CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct(number, requiredState)
    }
  }
}
