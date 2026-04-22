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

class NetworkIdentityManagementClusterClientStruct(
  val clientIndex: UInt,
  val clientIdentifier: ByteArray,
  val networkIdentityIndex: UInt?,
) {
  override fun toString(): String = buildString {
    append("NetworkIdentityManagementClusterClientStruct {\n")
    append("\tclientIndex : $clientIndex\n")
    append("\tclientIdentifier : $clientIdentifier\n")
    append("\tnetworkIdentityIndex : $networkIdentityIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CLIENT_INDEX), clientIndex)
      put(ContextSpecificTag(TAG_CLIENT_IDENTIFIER), clientIdentifier)
      if (networkIdentityIndex != null) {
        put(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX), networkIdentityIndex)
      } else {
        putNull(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CLIENT_INDEX = 0
    private const val TAG_CLIENT_IDENTIFIER = 1
    private const val TAG_NETWORK_IDENTITY_INDEX = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): NetworkIdentityManagementClusterClientStruct {
      tlvReader.enterStructure(tlvTag)
      val clientIndex = tlvReader.getUInt(ContextSpecificTag(TAG_CLIENT_INDEX))
      val clientIdentifier = tlvReader.getByteArray(ContextSpecificTag(TAG_CLIENT_IDENTIFIER))
      val networkIdentityIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX))
          null
        }

      tlvReader.exitContainer()

      return NetworkIdentityManagementClusterClientStruct(
        clientIndex,
        clientIdentifier,
        networkIdentityIndex,
      )
    }
  }
}
