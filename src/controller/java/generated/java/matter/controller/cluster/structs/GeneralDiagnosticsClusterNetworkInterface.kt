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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GeneralDiagnosticsClusterNetworkInterface(
  val name: String,
  val isOperational: Boolean,
  val offPremiseServicesReachableIPv4: Boolean?,
  val offPremiseServicesReachableIPv6: Boolean?,
  val hardwareAddress: ByteArray,
  val IPv4Addresses: List<ByteArray>,
  val IPv6Addresses: List<ByteArray>,
  val type: UByte,
) {
  override fun toString(): String = buildString {
    append("GeneralDiagnosticsClusterNetworkInterface {\n")
    append("\tname : $name\n")
    append("\tisOperational : $isOperational\n")
    append("\toffPremiseServicesReachableIPv4 : $offPremiseServicesReachableIPv4\n")
    append("\toffPremiseServicesReachableIPv6 : $offPremiseServicesReachableIPv6\n")
    append("\thardwareAddress : $hardwareAddress\n")
    append("\tIPv4Addresses : $IPv4Addresses\n")
    append("\tIPv6Addresses : $IPv6Addresses\n")
    append("\ttype : $type\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_IS_OPERATIONAL), isOperational)
      if (offPremiseServicesReachableIPv4 != null) {
        put(
          ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV4),
          offPremiseServicesReachableIPv4,
        )
      } else {
        putNull(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV4))
      }
      if (offPremiseServicesReachableIPv6 != null) {
        put(
          ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV6),
          offPremiseServicesReachableIPv6,
        )
      } else {
        putNull(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV6))
      }
      put(ContextSpecificTag(TAG_HARDWARE_ADDRESS), hardwareAddress)
      startArray(ContextSpecificTag(TAG_I_PV4_ADDRESSES))
      for (item in IPv4Addresses.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_I_PV6_ADDRESSES))
      for (item in IPv6Addresses.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_TYPE), type)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_IS_OPERATIONAL = 1
    private const val TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV4 = 2
    private const val TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV6 = 3
    private const val TAG_HARDWARE_ADDRESS = 4
    private const val TAG_I_PV4_ADDRESSES = 5
    private const val TAG_I_PV6_ADDRESSES = 6
    private const val TAG_TYPE = 7

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GeneralDiagnosticsClusterNetworkInterface {
      tlvReader.enterStructure(tlvTag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val isOperational = tlvReader.getBoolean(ContextSpecificTag(TAG_IS_OPERATIONAL))
      val offPremiseServicesReachableIPv4 =
        if (!tlvReader.isNull()) {
          tlvReader.getBoolean(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV4))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV4))
          null
        }
      val offPremiseServicesReachableIPv6 =
        if (!tlvReader.isNull()) {
          tlvReader.getBoolean(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV6))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_OFF_PREMISE_SERVICES_REACHABLE_I_PV6))
          null
        }
      val hardwareAddress = tlvReader.getByteArray(ContextSpecificTag(TAG_HARDWARE_ADDRESS))
      val IPv4Addresses =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_I_PV4_ADDRESSES))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val IPv6Addresses =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_I_PV6_ADDRESSES))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val type = tlvReader.getUByte(ContextSpecificTag(TAG_TYPE))

      tlvReader.exitContainer()

      return GeneralDiagnosticsClusterNetworkInterface(
        name,
        isOperational,
        offPremiseServicesReachableIPv4,
        offPremiseServicesReachableIPv6,
        hardwareAddress,
        IPv4Addresses,
        IPv6Addresses,
        type,
      )
    }
  }
}
