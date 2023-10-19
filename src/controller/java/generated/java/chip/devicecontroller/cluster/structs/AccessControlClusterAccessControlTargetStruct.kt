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

class AccessControlClusterAccessControlTargetStruct(
  val cluster: ULong?,
  val endpoint: UInt?,
  val deviceType: ULong?
) {
  override fun toString(): String = buildString {
    append("AccessControlClusterAccessControlTargetStruct {\n")
    append("\tcluster : $cluster\n")
    append("\tendpoint : $endpoint\n")
    append("\tdeviceType : $deviceType\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (cluster != null) {
        put(ContextSpecificTag(TAG_CLUSTER), cluster)
      } else {
        putNull(ContextSpecificTag(TAG_CLUSTER))
      }
      if (endpoint != null) {
        put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      } else {
        putNull(ContextSpecificTag(TAG_ENDPOINT))
      }
      if (deviceType != null) {
        put(ContextSpecificTag(TAG_DEVICE_TYPE), deviceType)
      } else {
        putNull(ContextSpecificTag(TAG_DEVICE_TYPE))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CLUSTER = 0
    private const val TAG_ENDPOINT = 1
    private const val TAG_DEVICE_TYPE = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AccessControlClusterAccessControlTargetStruct {
      tlvReader.enterStructure(tlvTag)
      val cluster =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CLUSTER))
          null
        }
      val endpoint =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ENDPOINT))
          null
        }
      val deviceType =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_DEVICE_TYPE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DEVICE_TYPE))
          null
        }

      tlvReader.exitContainer()

      return AccessControlClusterAccessControlTargetStruct(cluster, endpoint, deviceType)
    }
  }
}
