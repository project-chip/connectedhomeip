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

class OperationalCredentialsClusterPQCDeviceAttestationProfileStruct(
  val PAASupportedProfiles: UInt,
  val PAISupportedProfiles: UInt,
  val DACSupportedProfiles: UInt,
  val CDSupportedProfiles: UInt,
) {
  override fun toString(): String = buildString {
    append("OperationalCredentialsClusterPQCDeviceAttestationProfileStruct {\n")
    append("\tPAASupportedProfiles : $PAASupportedProfiles\n")
    append("\tPAISupportedProfiles : $PAISupportedProfiles\n")
    append("\tDACSupportedProfiles : $DACSupportedProfiles\n")
    append("\tCDSupportedProfiles : $CDSupportedProfiles\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PAA_SUPPORTED_PROFILES), PAASupportedProfiles)
      put(ContextSpecificTag(TAG_PAI_SUPPORTED_PROFILES), PAISupportedProfiles)
      put(ContextSpecificTag(TAG_DAC_SUPPORTED_PROFILES), DACSupportedProfiles)
      put(ContextSpecificTag(TAG_CD_SUPPORTED_PROFILES), CDSupportedProfiles)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PAA_SUPPORTED_PROFILES = 0
    private const val TAG_PAI_SUPPORTED_PROFILES = 1
    private const val TAG_DAC_SUPPORTED_PROFILES = 2
    private const val TAG_CD_SUPPORTED_PROFILES = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): OperationalCredentialsClusterPQCDeviceAttestationProfileStruct {
      tlvReader.enterStructure(tlvTag)
      val PAASupportedProfiles = tlvReader.getUInt(ContextSpecificTag(TAG_PAA_SUPPORTED_PROFILES))
      val PAISupportedProfiles = tlvReader.getUInt(ContextSpecificTag(TAG_PAI_SUPPORTED_PROFILES))
      val DACSupportedProfiles = tlvReader.getUInt(ContextSpecificTag(TAG_DAC_SUPPORTED_PROFILES))
      val CDSupportedProfiles = tlvReader.getUInt(ContextSpecificTag(TAG_CD_SUPPORTED_PROFILES))

      tlvReader.exitContainer()

      return OperationalCredentialsClusterPQCDeviceAttestationProfileStruct(
        PAASupportedProfiles,
        PAISupportedProfiles,
        DACSupportedProfiles,
        CDSupportedProfiles,
      )
    }
  }
}
