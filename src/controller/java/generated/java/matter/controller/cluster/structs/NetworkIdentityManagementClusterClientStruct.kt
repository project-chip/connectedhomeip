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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class NetworkIdentityManagementClusterClientStruct(
  val clientIndex: UShort,
  val clientIdentifier: ByteArray,
  val clientIdentityType: UByte,
  val networkIdentityIndex: UShort?,
) {
  override fun toString(): String = buildString {
    append("NetworkIdentityManagementClusterClientStruct {\n")
    append("\tclientIndex : $clientIndex\n")
    append("\tclientIdentifier : $clientIdentifier\n")
    append("\tclientIdentityType : $clientIdentityType\n")
    append("\tnetworkIdentityIndex : $networkIdentityIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CLIENT_INDEX), clientIndex)
      put(ContextSpecificTag(TAG_CLIENT_IDENTIFIER), clientIdentifier)
      put(ContextSpecificTag(TAG_CLIENT_IDENTITY_TYPE), clientIdentityType)
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
    private const val TAG_CLIENT_IDENTITY_TYPE = 2
    private const val TAG_NETWORK_IDENTITY_INDEX = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): NetworkIdentityManagementClusterClientStruct {
      tlvReader.enterStructure(tlvTag)
      val clientIndex = tlvReader.getUShort(ContextSpecificTag(TAG_CLIENT_INDEX))
      val clientIdentifier = tlvReader.getByteArray(ContextSpecificTag(TAG_CLIENT_IDENTIFIER))
      val clientIdentityType = tlvReader.getUByte(ContextSpecificTag(TAG_CLIENT_IDENTITY_TYPE))
      val networkIdentityIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NETWORK_IDENTITY_INDEX))
          null
        }

      tlvReader.exitContainer()

      return NetworkIdentityManagementClusterClientStruct(
        clientIndex,
        clientIdentifier,
        clientIdentityType,
        networkIdentityIndex,
      )
    }
  }
}
