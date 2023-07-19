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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class GeneralDiagnosticsClusterNetworkInterface (
    val name: String,
    val isOperational: Boolean,
    val offPremiseServicesReachableIPv4: Boolean?,
    val offPremiseServicesReachableIPv6: Boolean?,
    val hardwareAddress: ByteArray,
    val IPv4Addresses: List<ByteArray>,
    val IPv6Addresses: List<ByteArray>,
    val type: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("GeneralDiagnosticsClusterNetworkInterface {\n")
    builder.append("\tname : $name\n")
    builder.append("\tisOperational : $isOperational\n")
    builder.append("\toffPremiseServicesReachableIPv4 : $offPremiseServicesReachableIPv4\n")
    builder.append("\toffPremiseServicesReachableIPv6 : $offPremiseServicesReachableIPv6\n")
    builder.append("\thardwareAddress : $hardwareAddress\n")
    builder.append("\tIPv4Addresses : $IPv4Addresses\n")
    builder.append("\tIPv6Addresses : $IPv6Addresses\n")
    builder.append("\ttype : $type\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), name)
    tlvWriter.put(ContextSpecificTag(1), isOperational)
    if (offPremiseServicesReachableIPv4 == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), offPremiseServicesReachableIPv4)
    }
    if (offPremiseServicesReachableIPv6 == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), offPremiseServicesReachableIPv6)
    }
    tlvWriter.put(ContextSpecificTag(4), hardwareAddress)
    tlvWriter.startList(ContextSpecificTag(5))
      val iterIPv4Addresses = IPv4Addresses.iterator()
      while(iterIPv4Addresses.hasNext()) {
        val next = iterIPv4Addresses.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(6))
      val iterIPv6Addresses = IPv6Addresses.iterator()
      while(iterIPv6Addresses.hasNext()) {
        val next = iterIPv6Addresses.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.put(ContextSpecificTag(7), type)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralDiagnosticsClusterNetworkInterface {
      tlvReader.enterStructure(tag)
      val name: String = tlvReader.getString(ContextSpecificTag(0))
      val isOperational: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
      val offPremiseServicesReachableIPv4: Boolean? = try {
      tlvReader.getBoolean(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      val offPremiseServicesReachableIPv6: Boolean? = try {
      tlvReader.getBoolean(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      val hardwareAddress: ByteArray = tlvReader.getByteArray(ContextSpecificTag(4))
      val IPv4Addresses: List<ByteArray> = mutableListOf<ByteArray>().apply {
      tlvReader.enterList(ContextSpecificTag(5))
      while(true) {
        try {
          this.add(tlvReader.getByteArray(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val IPv6Addresses: List<ByteArray> = mutableListOf<ByteArray>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
      while(true) {
        try {
          this.add(tlvReader.getByteArray(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val type: Int = tlvReader.getInt(ContextSpecificTag(7))
      
      tlvReader.exitContainer()

      return GeneralDiagnosticsClusterNetworkInterface(name, isOperational, offPremiseServicesReachableIPv4, offPremiseServicesReachableIPv6, hardwareAddress, IPv4Addresses, IPv6Addresses, type)
    }
  }
}
