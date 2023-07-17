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
package chip.devicecontroller.cluster

import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ChipStructs {
    class ScenesClusterAttributeValuePair (
        val attributeID: Long,
        val attributeValue: Long) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ScenesClusterAttributeValuePair {\n")
        builder.append("\tattributeID : $attributeID\n")
        builder.append("\tattributeValue : $attributeValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), attributeID)
        tlvWriter.put(ContextSpecificTag(1), attributeValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ScenesClusterAttributeValuePair {
          tlvReader.enterStructure(tag)
          val attributeID: Long = tlvReader.getLong(ContextSpecificTag(0))
          val attributeValue: Long = tlvReader.getLong(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ScenesClusterAttributeValuePair(attributeID, attributeValue)
        }
      }
    }
    
    class ScenesClusterExtensionFieldSet (
        val clusterID: Long,
        val attributeValueList: List<ScenesClusterAttributeValuePair>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ScenesClusterExtensionFieldSet {\n")
        builder.append("\tclusterID : $clusterID\n")
        builder.append("\tattributeValueList : $attributeValueList\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), clusterID)
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_attributeValueList = attributeValueList.iterator()
      while(iter_attributeValueList.hasNext()) {
        val next = iter_attributeValueList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ScenesClusterExtensionFieldSet {
          tlvReader.enterStructure(tag)
          val clusterID: Long = tlvReader.getLong(ContextSpecificTag(0))
          val attributeValueList: List<ScenesClusterAttributeValuePair> = mutableListOf<ScenesClusterAttributeValuePair>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(ScenesClusterAttributeValuePair.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return ScenesClusterExtensionFieldSet(clusterID, attributeValueList)
        }
      }
    }
    
    class DescriptorClusterDeviceTypeStruct (
        val deviceType: Long,
        val revision: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DescriptorClusterDeviceTypeStruct {\n")
        builder.append("\tdeviceType : $deviceType\n")
        builder.append("\trevision : $revision\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), deviceType)
        tlvWriter.put(ContextSpecificTag(1), revision)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DescriptorClusterDeviceTypeStruct {
          tlvReader.enterStructure(tag)
          val deviceType: Long = tlvReader.getLong(ContextSpecificTag(0))
          val revision: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return DescriptorClusterDeviceTypeStruct(deviceType, revision)
        }
      }
    }
    
    class BindingClusterTargetStruct (
        val node: Optional<Long>,
        val group: Optional<Int>,
        val endpoint: Optional<Int>,
        val cluster: Optional<Long>,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BindingClusterTargetStruct {\n")
        builder.append("\tnode : $node\n")
        builder.append("\tgroup : $group\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("\tcluster : $cluster\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (node.isPresent) {
      val opt_node = node.get()
      tlvWriter.put(ContextSpecificTag(1), opt_node)
    }
        if (group.isPresent) {
      val opt_group = group.get()
      tlvWriter.put(ContextSpecificTag(2), opt_group)
    }
        if (endpoint.isPresent) {
      val opt_endpoint = endpoint.get()
      tlvWriter.put(ContextSpecificTag(3), opt_endpoint)
    }
        if (cluster.isPresent) {
      val opt_cluster = cluster.get()
      tlvWriter.put(ContextSpecificTag(4), opt_cluster)
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BindingClusterTargetStruct {
          tlvReader.enterStructure(tag)
          val node: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val group: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val endpoint: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val cluster: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return BindingClusterTargetStruct(node, group, endpoint, cluster, fabricIndex)
        }
      }
    }
    
    class AccessControlClusterAccessControlTargetStruct (
        val cluster: Long?,
        val endpoint: Int?,
        val deviceType: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AccessControlClusterAccessControlTargetStruct {\n")
        builder.append("\tcluster : $cluster\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("\tdeviceType : $deviceType\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (cluster == null) { tlvWriter.putNull(ContextSpecificTag(0)) }
    else {
      tlvWriter.put(ContextSpecificTag(0), cluster)
    }
        if (endpoint == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), endpoint)
    }
        if (deviceType == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), deviceType)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlTargetStruct {
          tlvReader.enterStructure(tag)
          val cluster: Long? = try {
      tlvReader.getLong(ContextSpecificTag(0))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(0))
      null
    }
          val endpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          val deviceType: Long? = try {
      tlvReader.getLong(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          
          tlvReader.exitContainer()

          return AccessControlClusterAccessControlTargetStruct(cluster, endpoint, deviceType)
        }
      }
    }
    
    class AccessControlClusterAccessControlEntryStruct (
        val privilege: Int,
        val authMode: Int,
        val subjects: List<Long>?,
        val targets: List<AccessControlClusterAccessControlTargetStruct>?,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AccessControlClusterAccessControlEntryStruct {\n")
        builder.append("\tprivilege : $privilege\n")
        builder.append("\tauthMode : $authMode\n")
        builder.append("\tsubjects : $subjects\n")
        builder.append("\ttargets : $targets\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), privilege)
        tlvWriter.put(ContextSpecificTag(2), authMode)
        if (subjects == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.startList(ContextSpecificTag(3))
      val iter_subjects = subjects.iterator()
      while(iter_subjects.hasNext()) {
        val next = iter_subjects.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
        if (targets == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.startList(ContextSpecificTag(4))
      val iter_targets = targets.iterator()
      while(iter_targets.hasNext()) {
        val next = iter_targets.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlEntryStruct {
          tlvReader.enterStructure(tag)
          val privilege: Int = tlvReader.getInt(ContextSpecificTag(1))
          val authMode: Int = tlvReader.getInt(ContextSpecificTag(2))
          val subjects: List<Long>? = try {
      mutableListOf<Long>().apply {
      tlvReader.enterList(ContextSpecificTag(3))
      while(true) {
        try {
          this.add(tlvReader.getLong(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val targets: List<AccessControlClusterAccessControlTargetStruct>? = try {
      mutableListOf<AccessControlClusterAccessControlTargetStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(4))
      while(true) {
        try {
          this.add(AccessControlClusterAccessControlTargetStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return AccessControlClusterAccessControlEntryStruct(privilege, authMode, subjects, targets, fabricIndex)
        }
      }
    }
    
    class AccessControlClusterAccessControlExtensionStruct (
        val data: ByteArray,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AccessControlClusterAccessControlExtensionStruct {\n")
        builder.append("\tdata : $data\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), data)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlExtensionStruct {
          tlvReader.enterStructure(tag)
          val data: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return AccessControlClusterAccessControlExtensionStruct(data, fabricIndex)
        }
      }
    }
    
    class ActionsClusterActionStruct (
        val actionID: Int,
        val name: String,
        val type: Int,
        val endpointListID: Int,
        val supportedCommands: Int,
        val state: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ActionsClusterActionStruct {\n")
        builder.append("\tactionID : $actionID\n")
        builder.append("\tname : $name\n")
        builder.append("\ttype : $type\n")
        builder.append("\tendpointListID : $endpointListID\n")
        builder.append("\tsupportedCommands : $supportedCommands\n")
        builder.append("\tstate : $state\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), actionID)
        tlvWriter.put(ContextSpecificTag(1), name)
        tlvWriter.put(ContextSpecificTag(2), type)
        tlvWriter.put(ContextSpecificTag(3), endpointListID)
        tlvWriter.put(ContextSpecificTag(4), supportedCommands)
        tlvWriter.put(ContextSpecificTag(5), state)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterActionStruct {
          tlvReader.enterStructure(tag)
          val actionID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val name: String = tlvReader.getString(ContextSpecificTag(1))
          val type: Int = tlvReader.getInt(ContextSpecificTag(2))
          val endpointListID: Int = tlvReader.getInt(ContextSpecificTag(3))
          val supportedCommands: Int = tlvReader.getInt(ContextSpecificTag(4))
          val state: Int = tlvReader.getInt(ContextSpecificTag(5))
          
          tlvReader.exitContainer()

          return ActionsClusterActionStruct(actionID, name, type, endpointListID, supportedCommands, state)
        }
      }
    }
    
    class ActionsClusterEndpointListStruct (
        val endpointListID: Int,
        val name: String,
        val type: Int,
        val endpoints: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ActionsClusterEndpointListStruct {\n")
        builder.append("\tendpointListID : $endpointListID\n")
        builder.append("\tname : $name\n")
        builder.append("\ttype : $type\n")
        builder.append("\tendpoints : $endpoints\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), endpointListID)
        tlvWriter.put(ContextSpecificTag(1), name)
        tlvWriter.put(ContextSpecificTag(2), type)
        tlvWriter.startList(ContextSpecificTag(3))
      val iter_endpoints = endpoints.iterator()
      while(iter_endpoints.hasNext()) {
        val next = iter_endpoints.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterEndpointListStruct {
          tlvReader.enterStructure(tag)
          val endpointListID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val name: String = tlvReader.getString(ContextSpecificTag(1))
          val type: Int = tlvReader.getInt(ContextSpecificTag(2))
          val endpoints: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(3))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return ActionsClusterEndpointListStruct(endpointListID, name, type, endpoints)
        }
      }
    }
    
    class BasicInformationClusterCapabilityMinimaStruct (
        val caseSessionsPerFabric: Int,
        val subscriptionsPerFabric: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterCapabilityMinimaStruct {\n")
        builder.append("\tcaseSessionsPerFabric : $caseSessionsPerFabric\n")
        builder.append("\tsubscriptionsPerFabric : $subscriptionsPerFabric\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), caseSessionsPerFabric)
        tlvWriter.put(ContextSpecificTag(1), subscriptionsPerFabric)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterCapabilityMinimaStruct {
          tlvReader.enterStructure(tag)
          val caseSessionsPerFabric: Int = tlvReader.getInt(ContextSpecificTag(0))
          val subscriptionsPerFabric: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return BasicInformationClusterCapabilityMinimaStruct(caseSessionsPerFabric, subscriptionsPerFabric)
        }
      }
    }
    
    class BasicInformationClusterProductAppearanceStruct (
        val finish: Int,
        val primaryColor: Int?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BasicInformationClusterProductAppearanceStruct {\n")
        builder.append("\tfinish : $finish\n")
        builder.append("\tprimaryColor : $primaryColor\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), finish)
        if (primaryColor == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), primaryColor)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterProductAppearanceStruct {
          tlvReader.enterStructure(tag)
          val finish: Int = tlvReader.getInt(ContextSpecificTag(0))
          val primaryColor: Int? = try {
      tlvReader.getInt(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          
          tlvReader.exitContainer()

          return BasicInformationClusterProductAppearanceStruct(finish, primaryColor)
        }
      }
    }
    
    class OtaSoftwareUpdateRequestorClusterProviderLocation (
        val providerNodeID: Long,
        val endpoint: Int,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OtaSoftwareUpdateRequestorClusterProviderLocation {\n")
        builder.append("\tproviderNodeID : $providerNodeID\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), providerNodeID)
        tlvWriter.put(ContextSpecificTag(2), endpoint)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterProviderLocation {
          tlvReader.enterStructure(tag)
          val providerNodeID: Long = tlvReader.getLong(ContextSpecificTag(1))
          val endpoint: Int = tlvReader.getInt(ContextSpecificTag(2))
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return OtaSoftwareUpdateRequestorClusterProviderLocation(providerNodeID, endpoint, fabricIndex)
        }
      }
    }
    
    class PowerSourceClusterBatChargeFaultChangeType (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterBatChargeFaultChangeType {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterBatChargeFaultChangeType {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterBatChargeFaultChangeType(current, previous)
        }
      }
    }
    
    class PowerSourceClusterBatFaultChangeType (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterBatFaultChangeType {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterBatFaultChangeType {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterBatFaultChangeType(current, previous)
        }
      }
    }
    
    class PowerSourceClusterWiredFaultChangeType (
        val current: List<Int>,
        val previous: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("PowerSourceClusterWiredFaultChangeType {\n")
        builder.append("\tcurrent : $current\n")
        builder.append("\tprevious : $previous\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_current = current.iterator()
      while(iter_current.hasNext()) {
        val next = iter_current.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(1))
      val iter_previous = previous.iterator()
      while(iter_previous.hasNext()) {
        val next = iter_previous.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterWiredFaultChangeType {
          tlvReader.enterStructure(tag)
          val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return PowerSourceClusterWiredFaultChangeType(current, previous)
        }
      }
    }
    
    class GeneralCommissioningClusterBasicCommissioningInfo (
        val failSafeExpiryLengthSeconds: Int,
        val maxCumulativeFailsafeSeconds: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GeneralCommissioningClusterBasicCommissioningInfo {\n")
        builder.append("\tfailSafeExpiryLengthSeconds : $failSafeExpiryLengthSeconds\n")
        builder.append("\tmaxCumulativeFailsafeSeconds : $maxCumulativeFailsafeSeconds\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), failSafeExpiryLengthSeconds)
        tlvWriter.put(ContextSpecificTag(1), maxCumulativeFailsafeSeconds)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GeneralCommissioningClusterBasicCommissioningInfo {
          tlvReader.enterStructure(tag)
          val failSafeExpiryLengthSeconds: Int = tlvReader.getInt(ContextSpecificTag(0))
          val maxCumulativeFailsafeSeconds: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return GeneralCommissioningClusterBasicCommissioningInfo(failSafeExpiryLengthSeconds, maxCumulativeFailsafeSeconds)
        }
      }
    }
    
    class NetworkCommissioningClusterNetworkInfoStruct (
        val networkID: ByteArray,
        val connected: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("NetworkCommissioningClusterNetworkInfoStruct {\n")
        builder.append("\tnetworkID : $networkID\n")
        builder.append("\tconnected : $connected\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), networkID)
        tlvWriter.put(ContextSpecificTag(1), connected)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterNetworkInfoStruct {
          tlvReader.enterStructure(tag)
          val networkID: ByteArray = tlvReader.getByteArray(ContextSpecificTag(0))
          val connected: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return NetworkCommissioningClusterNetworkInfoStruct(networkID, connected)
        }
      }
    }
    
    class NetworkCommissioningClusterThreadInterfaceScanResultStruct (
        val panId: Int,
        val extendedPanId: Long,
        val networkName: String,
        val channel: Int,
        val version: Int,
        val extendedAddress: ByteArray,
        val rssi: Int,
        val lqi: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("NetworkCommissioningClusterThreadInterfaceScanResultStruct {\n")
        builder.append("\tpanId : $panId\n")
        builder.append("\textendedPanId : $extendedPanId\n")
        builder.append("\tnetworkName : $networkName\n")
        builder.append("\tchannel : $channel\n")
        builder.append("\tversion : $version\n")
        builder.append("\textendedAddress : $extendedAddress\n")
        builder.append("\trssi : $rssi\n")
        builder.append("\tlqi : $lqi\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), panId)
        tlvWriter.put(ContextSpecificTag(1), extendedPanId)
        tlvWriter.put(ContextSpecificTag(2), networkName)
        tlvWriter.put(ContextSpecificTag(3), channel)
        tlvWriter.put(ContextSpecificTag(4), version)
        tlvWriter.put(ContextSpecificTag(5), extendedAddress)
        tlvWriter.put(ContextSpecificTag(6), rssi)
        tlvWriter.put(ContextSpecificTag(7), lqi)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterThreadInterfaceScanResultStruct {
          tlvReader.enterStructure(tag)
          val panId: Int = tlvReader.getInt(ContextSpecificTag(0))
          val extendedPanId: Long = tlvReader.getLong(ContextSpecificTag(1))
          val networkName: String = tlvReader.getString(ContextSpecificTag(2))
          val channel: Int = tlvReader.getInt(ContextSpecificTag(3))
          val version: Int = tlvReader.getInt(ContextSpecificTag(4))
          val extendedAddress: ByteArray = tlvReader.getByteArray(ContextSpecificTag(5))
          val rssi: Int = tlvReader.getInt(ContextSpecificTag(6))
          val lqi: Int = tlvReader.getInt(ContextSpecificTag(7))
          
          tlvReader.exitContainer()

          return NetworkCommissioningClusterThreadInterfaceScanResultStruct(panId, extendedPanId, networkName, channel, version, extendedAddress, rssi, lqi)
        }
      }
    }
    
    class NetworkCommissioningClusterWiFiInterfaceScanResultStruct (
        val security: Int,
        val ssid: ByteArray,
        val bssid: ByteArray,
        val channel: Int,
        val wiFiBand: Int,
        val rssi: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("NetworkCommissioningClusterWiFiInterfaceScanResultStruct {\n")
        builder.append("\tsecurity : $security\n")
        builder.append("\tssid : $ssid\n")
        builder.append("\tbssid : $bssid\n")
        builder.append("\tchannel : $channel\n")
        builder.append("\twiFiBand : $wiFiBand\n")
        builder.append("\trssi : $rssi\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), security)
        tlvWriter.put(ContextSpecificTag(1), ssid)
        tlvWriter.put(ContextSpecificTag(2), bssid)
        tlvWriter.put(ContextSpecificTag(3), channel)
        tlvWriter.put(ContextSpecificTag(4), wiFiBand)
        tlvWriter.put(ContextSpecificTag(5), rssi)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterWiFiInterfaceScanResultStruct {
          tlvReader.enterStructure(tag)
          val security: Int = tlvReader.getInt(ContextSpecificTag(0))
          val ssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
          val bssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(2))
          val channel: Int = tlvReader.getInt(ContextSpecificTag(3))
          val wiFiBand: Int = tlvReader.getInt(ContextSpecificTag(4))
          val rssi: Int = tlvReader.getInt(ContextSpecificTag(5))
          
          tlvReader.exitContainer()

          return NetworkCommissioningClusterWiFiInterfaceScanResultStruct(security, ssid, bssid, channel, wiFiBand, rssi)
        }
      }
    }
    
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
      val iter_IPv4Addresses = IPv4Addresses.iterator()
      while(iter_IPv4Addresses.hasNext()) {
        val next = iter_IPv4Addresses.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(6))
      val iter_IPv6Addresses = IPv6Addresses.iterator()
      while(iter_IPv6Addresses.hasNext()) {
        val next = iter_IPv6Addresses.next()
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
    
    class SoftwareDiagnosticsClusterThreadMetricsStruct (
        val id: Long,
        val name: Optional<String>,
        val stackFreeCurrent: Optional<Long>,
        val stackFreeMinimum: Optional<Long>,
        val stackSize: Optional<Long>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("SoftwareDiagnosticsClusterThreadMetricsStruct {\n")
        builder.append("\tid : $id\n")
        builder.append("\tname : $name\n")
        builder.append("\tstackFreeCurrent : $stackFreeCurrent\n")
        builder.append("\tstackFreeMinimum : $stackFreeMinimum\n")
        builder.append("\tstackSize : $stackSize\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), id)
        if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(1), opt_name)
    }
        if (stackFreeCurrent.isPresent) {
      val opt_stackFreeCurrent = stackFreeCurrent.get()
      tlvWriter.put(ContextSpecificTag(2), opt_stackFreeCurrent)
    }
        if (stackFreeMinimum.isPresent) {
      val opt_stackFreeMinimum = stackFreeMinimum.get()
      tlvWriter.put(ContextSpecificTag(3), opt_stackFreeMinimum)
    }
        if (stackSize.isPresent) {
      val opt_stackSize = stackSize.get()
      tlvWriter.put(ContextSpecificTag(4), opt_stackSize)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : SoftwareDiagnosticsClusterThreadMetricsStruct {
          tlvReader.enterStructure(tag)
          val id: Long = tlvReader.getLong(ContextSpecificTag(0))
          val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val stackFreeCurrent: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val stackFreeMinimum: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val stackSize: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return SoftwareDiagnosticsClusterThreadMetricsStruct(id, name, stackFreeCurrent, stackFreeMinimum, stackSize)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterNeighborTableStruct (
        val extAddress: Long,
        val age: Long,
        val rloc16: Int,
        val linkFrameCounter: Long,
        val mleFrameCounter: Long,
        val lqi: Int,
        val averageRssi: Int?,
        val lastRssi: Int?,
        val frameErrorRate: Int,
        val messageErrorRate: Int,
        val rxOnWhenIdle: Boolean,
        val fullThreadDevice: Boolean,
        val fullNetworkData: Boolean,
        val isChild: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterNeighborTableStruct {\n")
        builder.append("\textAddress : $extAddress\n")
        builder.append("\tage : $age\n")
        builder.append("\trloc16 : $rloc16\n")
        builder.append("\tlinkFrameCounter : $linkFrameCounter\n")
        builder.append("\tmleFrameCounter : $mleFrameCounter\n")
        builder.append("\tlqi : $lqi\n")
        builder.append("\taverageRssi : $averageRssi\n")
        builder.append("\tlastRssi : $lastRssi\n")
        builder.append("\tframeErrorRate : $frameErrorRate\n")
        builder.append("\tmessageErrorRate : $messageErrorRate\n")
        builder.append("\trxOnWhenIdle : $rxOnWhenIdle\n")
        builder.append("\tfullThreadDevice : $fullThreadDevice\n")
        builder.append("\tfullNetworkData : $fullNetworkData\n")
        builder.append("\tisChild : $isChild\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), extAddress)
        tlvWriter.put(ContextSpecificTag(1), age)
        tlvWriter.put(ContextSpecificTag(2), rloc16)
        tlvWriter.put(ContextSpecificTag(3), linkFrameCounter)
        tlvWriter.put(ContextSpecificTag(4), mleFrameCounter)
        tlvWriter.put(ContextSpecificTag(5), lqi)
        if (averageRssi == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      tlvWriter.put(ContextSpecificTag(6), averageRssi)
    }
        if (lastRssi == null) { tlvWriter.putNull(ContextSpecificTag(7)) }
    else {
      tlvWriter.put(ContextSpecificTag(7), lastRssi)
    }
        tlvWriter.put(ContextSpecificTag(8), frameErrorRate)
        tlvWriter.put(ContextSpecificTag(9), messageErrorRate)
        tlvWriter.put(ContextSpecificTag(10), rxOnWhenIdle)
        tlvWriter.put(ContextSpecificTag(11), fullThreadDevice)
        tlvWriter.put(ContextSpecificTag(12), fullNetworkData)
        tlvWriter.put(ContextSpecificTag(13), isChild)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterNeighborTableStruct {
          tlvReader.enterStructure(tag)
          val extAddress: Long = tlvReader.getLong(ContextSpecificTag(0))
          val age: Long = tlvReader.getLong(ContextSpecificTag(1))
          val rloc16: Int = tlvReader.getInt(ContextSpecificTag(2))
          val linkFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(3))
          val mleFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(4))
          val lqi: Int = tlvReader.getInt(ContextSpecificTag(5))
          val averageRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(6))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
          val lastRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(7))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(7))
      null
    }
          val frameErrorRate: Int = tlvReader.getInt(ContextSpecificTag(8))
          val messageErrorRate: Int = tlvReader.getInt(ContextSpecificTag(9))
          val rxOnWhenIdle: Boolean = tlvReader.getBoolean(ContextSpecificTag(10))
          val fullThreadDevice: Boolean = tlvReader.getBoolean(ContextSpecificTag(11))
          val fullNetworkData: Boolean = tlvReader.getBoolean(ContextSpecificTag(12))
          val isChild: Boolean = tlvReader.getBoolean(ContextSpecificTag(13))
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterNeighborTableStruct(extAddress, age, rloc16, linkFrameCounter, mleFrameCounter, lqi, averageRssi, lastRssi, frameErrorRate, messageErrorRate, rxOnWhenIdle, fullThreadDevice, fullNetworkData, isChild)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterOperationalDatasetComponents (
        val activeTimestampPresent: Boolean,
        val pendingTimestampPresent: Boolean,
        val masterKeyPresent: Boolean,
        val networkNamePresent: Boolean,
        val extendedPanIdPresent: Boolean,
        val meshLocalPrefixPresent: Boolean,
        val delayPresent: Boolean,
        val panIdPresent: Boolean,
        val channelPresent: Boolean,
        val pskcPresent: Boolean,
        val securityPolicyPresent: Boolean,
        val channelMaskPresent: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {\n")
        builder.append("\tactiveTimestampPresent : $activeTimestampPresent\n")
        builder.append("\tpendingTimestampPresent : $pendingTimestampPresent\n")
        builder.append("\tmasterKeyPresent : $masterKeyPresent\n")
        builder.append("\tnetworkNamePresent : $networkNamePresent\n")
        builder.append("\textendedPanIdPresent : $extendedPanIdPresent\n")
        builder.append("\tmeshLocalPrefixPresent : $meshLocalPrefixPresent\n")
        builder.append("\tdelayPresent : $delayPresent\n")
        builder.append("\tpanIdPresent : $panIdPresent\n")
        builder.append("\tchannelPresent : $channelPresent\n")
        builder.append("\tpskcPresent : $pskcPresent\n")
        builder.append("\tsecurityPolicyPresent : $securityPolicyPresent\n")
        builder.append("\tchannelMaskPresent : $channelMaskPresent\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), activeTimestampPresent)
        tlvWriter.put(ContextSpecificTag(1), pendingTimestampPresent)
        tlvWriter.put(ContextSpecificTag(2), masterKeyPresent)
        tlvWriter.put(ContextSpecificTag(3), networkNamePresent)
        tlvWriter.put(ContextSpecificTag(4), extendedPanIdPresent)
        tlvWriter.put(ContextSpecificTag(5), meshLocalPrefixPresent)
        tlvWriter.put(ContextSpecificTag(6), delayPresent)
        tlvWriter.put(ContextSpecificTag(7), panIdPresent)
        tlvWriter.put(ContextSpecificTag(8), channelPresent)
        tlvWriter.put(ContextSpecificTag(9), pskcPresent)
        tlvWriter.put(ContextSpecificTag(10), securityPolicyPresent)
        tlvWriter.put(ContextSpecificTag(11), channelMaskPresent)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {
          tlvReader.enterStructure(tag)
          val activeTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(0))
          val pendingTimestampPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
          val masterKeyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(2))
          val networkNamePresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(3))
          val extendedPanIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(4))
          val meshLocalPrefixPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(5))
          val delayPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(6))
          val panIdPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(7))
          val channelPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(8))
          val pskcPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(9))
          val securityPolicyPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(10))
          val channelMaskPresent: Boolean = tlvReader.getBoolean(ContextSpecificTag(11))
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(activeTimestampPresent, pendingTimestampPresent, masterKeyPresent, networkNamePresent, extendedPanIdPresent, meshLocalPrefixPresent, delayPresent, panIdPresent, channelPresent, pskcPresent, securityPolicyPresent, channelMaskPresent)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterRouteTableStruct (
        val extAddress: Long,
        val rloc16: Int,
        val routerId: Int,
        val nextHop: Int,
        val pathCost: Int,
        val LQIIn: Int,
        val LQIOut: Int,
        val age: Int,
        val allocated: Boolean,
        val linkEstablished: Boolean) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterRouteTableStruct {\n")
        builder.append("\textAddress : $extAddress\n")
        builder.append("\trloc16 : $rloc16\n")
        builder.append("\trouterId : $routerId\n")
        builder.append("\tnextHop : $nextHop\n")
        builder.append("\tpathCost : $pathCost\n")
        builder.append("\tLQIIn : $LQIIn\n")
        builder.append("\tLQIOut : $LQIOut\n")
        builder.append("\tage : $age\n")
        builder.append("\tallocated : $allocated\n")
        builder.append("\tlinkEstablished : $linkEstablished\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), extAddress)
        tlvWriter.put(ContextSpecificTag(1), rloc16)
        tlvWriter.put(ContextSpecificTag(2), routerId)
        tlvWriter.put(ContextSpecificTag(3), nextHop)
        tlvWriter.put(ContextSpecificTag(4), pathCost)
        tlvWriter.put(ContextSpecificTag(5), LQIIn)
        tlvWriter.put(ContextSpecificTag(6), LQIOut)
        tlvWriter.put(ContextSpecificTag(7), age)
        tlvWriter.put(ContextSpecificTag(8), allocated)
        tlvWriter.put(ContextSpecificTag(9), linkEstablished)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterRouteTableStruct {
          tlvReader.enterStructure(tag)
          val extAddress: Long = tlvReader.getLong(ContextSpecificTag(0))
          val rloc16: Int = tlvReader.getInt(ContextSpecificTag(1))
          val routerId: Int = tlvReader.getInt(ContextSpecificTag(2))
          val nextHop: Int = tlvReader.getInt(ContextSpecificTag(3))
          val pathCost: Int = tlvReader.getInt(ContextSpecificTag(4))
          val LQIIn: Int = tlvReader.getInt(ContextSpecificTag(5))
          val LQIOut: Int = tlvReader.getInt(ContextSpecificTag(6))
          val age: Int = tlvReader.getInt(ContextSpecificTag(7))
          val allocated: Boolean = tlvReader.getBoolean(ContextSpecificTag(8))
          val linkEstablished: Boolean = tlvReader.getBoolean(ContextSpecificTag(9))
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterRouteTableStruct(extAddress, rloc16, routerId, nextHop, pathCost, LQIIn, LQIOut, age, allocated, linkEstablished)
        }
      }
    }
    
    class ThreadNetworkDiagnosticsClusterSecurityPolicy (
        val rotationTime: Int,
        val flags: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThreadNetworkDiagnosticsClusterSecurityPolicy {\n")
        builder.append("\trotationTime : $rotationTime\n")
        builder.append("\tflags : $flags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), rotationTime)
        tlvWriter.put(ContextSpecificTag(1), flags)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterSecurityPolicy {
          tlvReader.enterStructure(tag)
          val rotationTime: Int = tlvReader.getInt(ContextSpecificTag(0))
          val flags: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ThreadNetworkDiagnosticsClusterSecurityPolicy(rotationTime, flags)
        }
      }
    }
    
    class TimeSynchronizationClusterDSTOffsetStruct (
        val offset: Long,
        val validStarting: Long,
        val validUntil: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterDSTOffsetStruct {\n")
        builder.append("\toffset : $offset\n")
        builder.append("\tvalidStarting : $validStarting\n")
        builder.append("\tvalidUntil : $validUntil\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), offset)
        tlvWriter.put(ContextSpecificTag(1), validStarting)
        if (validUntil == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), validUntil)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterDSTOffsetStruct {
          tlvReader.enterStructure(tag)
          val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
          val validStarting: Long = tlvReader.getLong(ContextSpecificTag(1))
          val validUntil: Long? = try {
      tlvReader.getLong(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterDSTOffsetStruct(offset, validStarting, validUntil)
        }
      }
    }
    
    class TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct (
        val nodeID: Long,
        val endpoint: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {\n")
        builder.append("\tnodeID : $nodeID\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), nodeID)
        tlvWriter.put(ContextSpecificTag(1), endpoint)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {
          tlvReader.enterStructure(tag)
          val nodeID: Long = tlvReader.getLong(ContextSpecificTag(0))
          val endpoint: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(nodeID, endpoint)
        }
      }
    }
    
    class TimeSynchronizationClusterTimeZoneStruct (
        val offset: Long,
        val validAt: Long,
        val name: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterTimeZoneStruct {\n")
        builder.append("\toffset : $offset\n")
        builder.append("\tvalidAt : $validAt\n")
        builder.append("\tname : $name\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), offset)
        tlvWriter.put(ContextSpecificTag(1), validAt)
        if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(2), opt_name)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTimeZoneStruct {
          tlvReader.enterStructure(tag)
          val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
          val validAt: Long = tlvReader.getLong(ContextSpecificTag(1))
          val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterTimeZoneStruct(offset, validAt, name)
        }
      }
    }
    
    class TimeSynchronizationClusterTrustedTimeSourceStruct (
        val fabricIndex: Int,
        val nodeID: Long,
        val endpoint: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TimeSynchronizationClusterTrustedTimeSourceStruct {\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("\tnodeID : $nodeID\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), fabricIndex)
        tlvWriter.put(ContextSpecificTag(1), nodeID)
        tlvWriter.put(ContextSpecificTag(2), endpoint)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTrustedTimeSourceStruct {
          tlvReader.enterStructure(tag)
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(0))
          val nodeID: Long = tlvReader.getLong(ContextSpecificTag(1))
          val endpoint: Int = tlvReader.getInt(ContextSpecificTag(2))
          
          tlvReader.exitContainer()

          return TimeSynchronizationClusterTrustedTimeSourceStruct(fabricIndex, nodeID, endpoint)
        }
      }
    }
    
    class BridgedDeviceBasicInformationClusterProductAppearanceStruct (
        val finish: Int,
        val primaryColor: Int?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("BridgedDeviceBasicInformationClusterProductAppearanceStruct {\n")
        builder.append("\tfinish : $finish\n")
        builder.append("\tprimaryColor : $primaryColor\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), finish)
        if (primaryColor == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), primaryColor)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : BridgedDeviceBasicInformationClusterProductAppearanceStruct {
          tlvReader.enterStructure(tag)
          val finish: Int = tlvReader.getInt(ContextSpecificTag(0))
          val primaryColor: Int? = try {
      tlvReader.getInt(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          
          tlvReader.exitContainer()

          return BridgedDeviceBasicInformationClusterProductAppearanceStruct(finish, primaryColor)
        }
      }
    }
    
    class OperationalCredentialsClusterFabricDescriptorStruct (
        val rootPublicKey: ByteArray,
        val vendorID: Int,
        val fabricID: Long,
        val nodeID: Long,
        val label: String,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalCredentialsClusterFabricDescriptorStruct {\n")
        builder.append("\trootPublicKey : $rootPublicKey\n")
        builder.append("\tvendorID : $vendorID\n")
        builder.append("\tfabricID : $fabricID\n")
        builder.append("\tnodeID : $nodeID\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), rootPublicKey)
        tlvWriter.put(ContextSpecificTag(2), vendorID)
        tlvWriter.put(ContextSpecificTag(3), fabricID)
        tlvWriter.put(ContextSpecificTag(4), nodeID)
        tlvWriter.put(ContextSpecificTag(5), label)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalCredentialsClusterFabricDescriptorStruct {
          tlvReader.enterStructure(tag)
          val rootPublicKey: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
          val vendorID: Int = tlvReader.getInt(ContextSpecificTag(2))
          val fabricID: Long = tlvReader.getLong(ContextSpecificTag(3))
          val nodeID: Long = tlvReader.getLong(ContextSpecificTag(4))
          val label: String = tlvReader.getString(ContextSpecificTag(5))
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return OperationalCredentialsClusterFabricDescriptorStruct(rootPublicKey, vendorID, fabricID, nodeID, label, fabricIndex)
        }
      }
    }
    
    class OperationalCredentialsClusterNOCStruct (
        val noc: ByteArray,
        val icac: ByteArray?,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalCredentialsClusterNOCStruct {\n")
        builder.append("\tnoc : $noc\n")
        builder.append("\ticac : $icac\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), noc)
        if (icac == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), icac)
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalCredentialsClusterNOCStruct {
          tlvReader.enterStructure(tag)
          val noc: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
          val icac: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return OperationalCredentialsClusterNOCStruct(noc, icac, fabricIndex)
        }
      }
    }
    
    class GroupKeyManagementClusterGroupInfoMapStruct (
        val groupId: Int,
        val endpoints: List<Int>,
        val groupName: Optional<String>,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GroupKeyManagementClusterGroupInfoMapStruct {\n")
        builder.append("\tgroupId : $groupId\n")
        builder.append("\tendpoints : $endpoints\n")
        builder.append("\tgroupName : $groupName\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), groupId)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_endpoints = endpoints.iterator()
      while(iter_endpoints.hasNext()) {
        val next = iter_endpoints.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        if (groupName.isPresent) {
      val opt_groupName = groupName.get()
      tlvWriter.put(ContextSpecificTag(3), opt_groupName)
    }
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GroupKeyManagementClusterGroupInfoMapStruct {
          tlvReader.enterStructure(tag)
          val groupId: Int = tlvReader.getInt(ContextSpecificTag(1))
          val endpoints: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val groupName: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return GroupKeyManagementClusterGroupInfoMapStruct(groupId, endpoints, groupName, fabricIndex)
        }
      }
    }
    
    class GroupKeyManagementClusterGroupKeyMapStruct (
        val groupId: Int,
        val groupKeySetID: Int,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GroupKeyManagementClusterGroupKeyMapStruct {\n")
        builder.append("\tgroupId : $groupId\n")
        builder.append("\tgroupKeySetID : $groupKeySetID\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), groupId)
        tlvWriter.put(ContextSpecificTag(2), groupKeySetID)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GroupKeyManagementClusterGroupKeyMapStruct {
          tlvReader.enterStructure(tag)
          val groupId: Int = tlvReader.getInt(ContextSpecificTag(1))
          val groupKeySetID: Int = tlvReader.getInt(ContextSpecificTag(2))
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return GroupKeyManagementClusterGroupKeyMapStruct(groupId, groupKeySetID, fabricIndex)
        }
      }
    }
    
    class GroupKeyManagementClusterGroupKeySetStruct (
        val groupKeySetID: Int,
        val groupKeySecurityPolicy: Int,
        val epochKey0: ByteArray?,
        val epochStartTime0: Long?,
        val epochKey1: ByteArray?,
        val epochStartTime1: Long?,
        val epochKey2: ByteArray?,
        val epochStartTime2: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("GroupKeyManagementClusterGroupKeySetStruct {\n")
        builder.append("\tgroupKeySetID : $groupKeySetID\n")
        builder.append("\tgroupKeySecurityPolicy : $groupKeySecurityPolicy\n")
        builder.append("\tepochKey0 : $epochKey0\n")
        builder.append("\tepochStartTime0 : $epochStartTime0\n")
        builder.append("\tepochKey1 : $epochKey1\n")
        builder.append("\tepochStartTime1 : $epochStartTime1\n")
        builder.append("\tepochKey2 : $epochKey2\n")
        builder.append("\tepochStartTime2 : $epochStartTime2\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), groupKeySetID)
        tlvWriter.put(ContextSpecificTag(1), groupKeySecurityPolicy)
        if (epochKey0 == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), epochKey0)
    }
        if (epochStartTime0 == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), epochStartTime0)
    }
        if (epochKey1 == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.put(ContextSpecificTag(4), epochKey1)
    }
        if (epochStartTime1 == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      tlvWriter.put(ContextSpecificTag(5), epochStartTime1)
    }
        if (epochKey2 == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      tlvWriter.put(ContextSpecificTag(6), epochKey2)
    }
        if (epochStartTime2 == null) { tlvWriter.putNull(ContextSpecificTag(7)) }
    else {
      tlvWriter.put(ContextSpecificTag(7), epochStartTime2)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : GroupKeyManagementClusterGroupKeySetStruct {
          tlvReader.enterStructure(tag)
          val groupKeySetID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val groupKeySecurityPolicy: Int = tlvReader.getInt(ContextSpecificTag(1))
          val epochKey0: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val epochStartTime0: Long? = try {
      tlvReader.getLong(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val epochKey1: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(4))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val epochStartTime1: Long? = try {
      tlvReader.getLong(ContextSpecificTag(5))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
          val epochKey2: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(6))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
          val epochStartTime2: Long? = try {
      tlvReader.getLong(ContextSpecificTag(7))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(7))
      null
    }
          
          tlvReader.exitContainer()

          return GroupKeyManagementClusterGroupKeySetStruct(groupKeySetID, groupKeySecurityPolicy, epochKey0, epochStartTime0, epochKey1, epochStartTime1, epochKey2, epochStartTime2)
        }
      }
    }
    
    class FixedLabelClusterLabelStruct (
        val label: String,
        val value: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("FixedLabelClusterLabelStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : FixedLabelClusterLabelStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val value: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return FixedLabelClusterLabelStruct(label, value)
        }
      }
    }
    
    class UserLabelClusterLabelStruct (
        val label: String,
        val value: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UserLabelClusterLabelStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UserLabelClusterLabelStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val value: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return UserLabelClusterLabelStruct(label, value)
        }
      }
    }
    
    class IcdManagementClusterMonitoringRegistrationStruct (
        val checkInNodeID: Long,
        val monitoredSubject: Long,
        val key: ByteArray,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("IcdManagementClusterMonitoringRegistrationStruct {\n")
        builder.append("\tcheckInNodeID : $checkInNodeID\n")
        builder.append("\tmonitoredSubject : $monitoredSubject\n")
        builder.append("\tkey : $key\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), checkInNodeID)
        tlvWriter.put(ContextSpecificTag(2), monitoredSubject)
        tlvWriter.put(ContextSpecificTag(3), key)
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : IcdManagementClusterMonitoringRegistrationStruct {
          tlvReader.enterStructure(tag)
          val checkInNodeID: Long = tlvReader.getLong(ContextSpecificTag(1))
          val monitoredSubject: Long = tlvReader.getLong(ContextSpecificTag(2))
          val key: ByteArray = tlvReader.getByteArray(ContextSpecificTag(3))
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return IcdManagementClusterMonitoringRegistrationStruct(checkInNodeID, monitoredSubject, key, fabricIndex)
        }
      }
    }
    
    class ModeSelectClusterSemanticTagStruct (
        val mfgCode: Int,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ModeSelectClusterSemanticTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), mfgCode)
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ModeSelectClusterSemanticTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Int = tlvReader.getInt(ContextSpecificTag(0))
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ModeSelectClusterSemanticTagStruct(mfgCode, value)
        }
      }
    }
    
    class ModeSelectClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val semanticTags: List<ModeSelectClusterSemanticTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ModeSelectClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tsemanticTags : $semanticTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_semanticTags = semanticTags.iterator()
      while(iter_semanticTags.hasNext()) {
        val next = iter_semanticTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ModeSelectClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val semanticTags: List<ModeSelectClusterSemanticTagStruct> = mutableListOf<ModeSelectClusterSemanticTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(ModeSelectClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return ModeSelectClusterModeOptionStruct(label, mode, semanticTags)
        }
      }
    }
    
    class LaundryWasherModeClusterModeTagStruct (
        val mfgCode: Optional<Int>,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("LaundryWasherModeClusterModeTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (mfgCode.isPresent) {
      val opt_mfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(0), opt_mfgCode)
    }
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : LaundryWasherModeClusterModeTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return LaundryWasherModeClusterModeTagStruct(mfgCode, value)
        }
      }
    }
    
    class LaundryWasherModeClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val modeTags: List<LaundryWasherModeClusterModeTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("LaundryWasherModeClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tmodeTags : $modeTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : LaundryWasherModeClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val modeTags: List<LaundryWasherModeClusterModeTagStruct> = mutableListOf<LaundryWasherModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(LaundryWasherModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return LaundryWasherModeClusterModeOptionStruct(label, mode, modeTags)
        }
      }
    }
    
    class RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct (
        val mfgCode: Optional<Int>,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (mfgCode.isPresent) {
      val opt_mfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(0), opt_mfgCode)
    }
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct(mfgCode, value)
        }
      }
    }
    
    class RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val modeTags: List<RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tmodeTags : $modeTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val modeTags: List<RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct> = mutableListOf<RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct(label, mode, modeTags)
        }
      }
    }
    
    class RvcRunModeClusterModeTagStruct (
        val mfgCode: Optional<Int>,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcRunModeClusterModeTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (mfgCode.isPresent) {
      val opt_mfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(0), opt_mfgCode)
    }
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcRunModeClusterModeTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return RvcRunModeClusterModeTagStruct(mfgCode, value)
        }
      }
    }
    
    class RvcRunModeClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val modeTags: List<RvcRunModeClusterModeTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcRunModeClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tmodeTags : $modeTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcRunModeClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val modeTags: List<RvcRunModeClusterModeTagStruct> = mutableListOf<RvcRunModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(RvcRunModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return RvcRunModeClusterModeOptionStruct(label, mode, modeTags)
        }
      }
    }
    
    class RvcCleanModeClusterModeTagStruct (
        val mfgCode: Optional<Int>,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcCleanModeClusterModeTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (mfgCode.isPresent) {
      val opt_mfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(0), opt_mfgCode)
    }
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcCleanModeClusterModeTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return RvcCleanModeClusterModeTagStruct(mfgCode, value)
        }
      }
    }
    
    class RvcCleanModeClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val modeTags: List<RvcCleanModeClusterModeTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcCleanModeClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tmodeTags : $modeTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcCleanModeClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val modeTags: List<RvcCleanModeClusterModeTagStruct> = mutableListOf<RvcCleanModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(RvcCleanModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return RvcCleanModeClusterModeOptionStruct(label, mode, modeTags)
        }
      }
    }
    
    class DishwasherModeClusterModeTagStruct (
        val mfgCode: Optional<Int>,
        val value: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DishwasherModeClusterModeTagStruct {\n")
        builder.append("\tmfgCode : $mfgCode\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (mfgCode.isPresent) {
      val opt_mfgCode = mfgCode.get()
      tlvWriter.put(ContextSpecificTag(0), opt_mfgCode)
    }
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DishwasherModeClusterModeTagStruct {
          tlvReader.enterStructure(tag)
          val mfgCode: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val value: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return DishwasherModeClusterModeTagStruct(mfgCode, value)
        }
      }
    }
    
    class DishwasherModeClusterModeOptionStruct (
        val label: String,
        val mode: Int,
        val modeTags: List<DishwasherModeClusterModeTagStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DishwasherModeClusterModeOptionStruct {\n")
        builder.append("\tlabel : $label\n")
        builder.append("\tmode : $mode\n")
        builder.append("\tmodeTags : $modeTags\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), label)
        tlvWriter.put(ContextSpecificTag(1), mode)
        tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DishwasherModeClusterModeOptionStruct {
          tlvReader.enterStructure(tag)
          val label: String = tlvReader.getString(ContextSpecificTag(0))
          val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
          val modeTags: List<DishwasherModeClusterModeTagStruct> = mutableListOf<DishwasherModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(DishwasherModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return DishwasherModeClusterModeOptionStruct(label, mode, modeTags)
        }
      }
    }
    
    class OperationalStateClusterErrorStateStruct (
        val errorStateID: Int,
        val errorStateLabel: Optional<String>,
        val errorStateDetails: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalStateClusterErrorStateStruct {\n")
        builder.append("\terrorStateID : $errorStateID\n")
        builder.append("\terrorStateLabel : $errorStateLabel\n")
        builder.append("\terrorStateDetails : $errorStateDetails\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), errorStateID)
        if (errorStateLabel.isPresent) {
      val opt_errorStateLabel = errorStateLabel.get()
      tlvWriter.put(ContextSpecificTag(1), opt_errorStateLabel)
    }
        if (errorStateDetails.isPresent) {
      val opt_errorStateDetails = errorStateDetails.get()
      tlvWriter.put(ContextSpecificTag(2), opt_errorStateDetails)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterErrorStateStruct {
          tlvReader.enterStructure(tag)
          val errorStateID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val errorStateLabel: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val errorStateDetails: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return OperationalStateClusterErrorStateStruct(errorStateID, errorStateLabel, errorStateDetails)
        }
      }
    }
    
    class OperationalStateClusterOperationalStateStruct (
        val operationalStateID: Int,
        val operationalStateLabel: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OperationalStateClusterOperationalStateStruct {\n")
        builder.append("\toperationalStateID : $operationalStateID\n")
        builder.append("\toperationalStateLabel : $operationalStateLabel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), operationalStateID)
        if (operationalStateLabel.isPresent) {
      val opt_operationalStateLabel = operationalStateLabel.get()
      tlvWriter.put(ContextSpecificTag(1), opt_operationalStateLabel)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterOperationalStateStruct {
          tlvReader.enterStructure(tag)
          val operationalStateID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val operationalStateLabel: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return OperationalStateClusterOperationalStateStruct(operationalStateID, operationalStateLabel)
        }
      }
    }
    
    class RvcOperationalStateClusterErrorStateStruct (
        val errorStateID: Int,
        val errorStateLabel: Optional<String>,
        val errorStateDetails: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcOperationalStateClusterErrorStateStruct {\n")
        builder.append("\terrorStateID : $errorStateID\n")
        builder.append("\terrorStateLabel : $errorStateLabel\n")
        builder.append("\terrorStateDetails : $errorStateDetails\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), errorStateID)
        if (errorStateLabel.isPresent) {
      val opt_errorStateLabel = errorStateLabel.get()
      tlvWriter.put(ContextSpecificTag(1), opt_errorStateLabel)
    }
        if (errorStateDetails.isPresent) {
      val opt_errorStateDetails = errorStateDetails.get()
      tlvWriter.put(ContextSpecificTag(2), opt_errorStateDetails)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcOperationalStateClusterErrorStateStruct {
          tlvReader.enterStructure(tag)
          val errorStateID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val errorStateLabel: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val errorStateDetails: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return RvcOperationalStateClusterErrorStateStruct(errorStateID, errorStateLabel, errorStateDetails)
        }
      }
    }
    
    class RvcOperationalStateClusterOperationalStateStruct (
        val operationalStateID: Int,
        val operationalStateLabel: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("RvcOperationalStateClusterOperationalStateStruct {\n")
        builder.append("\toperationalStateID : $operationalStateID\n")
        builder.append("\toperationalStateLabel : $operationalStateLabel\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), operationalStateID)
        if (operationalStateLabel.isPresent) {
      val opt_operationalStateLabel = operationalStateLabel.get()
      tlvWriter.put(ContextSpecificTag(1), opt_operationalStateLabel)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcOperationalStateClusterOperationalStateStruct {
          tlvReader.enterStructure(tag)
          val operationalStateID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val operationalStateLabel: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return RvcOperationalStateClusterOperationalStateStruct(operationalStateID, operationalStateLabel)
        }
      }
    }
    
    class HepaFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("HepaFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : HepaFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return HepaFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class ActivatedCarbonFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ActivatedCarbonFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActivatedCarbonFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ActivatedCarbonFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class CeramicFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("CeramicFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : CeramicFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return CeramicFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class ElectrostaticFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ElectrostaticFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ElectrostaticFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ElectrostaticFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class UvFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UvFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UvFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return UvFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class IonizingFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("IonizingFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : IonizingFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return IonizingFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class ZeoliteFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ZeoliteFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ZeoliteFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ZeoliteFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class OzoneFilterMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("OzoneFilterMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : OzoneFilterMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return OzoneFilterMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class WaterTankMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("WaterTankMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : WaterTankMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return WaterTankMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class FuelTankMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("FuelTankMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : FuelTankMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return FuelTankMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class InkCartridgeMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("InkCartridgeMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : InkCartridgeMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return InkCartridgeMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class TonerCartridgeMonitoringClusterReplacementProductStruct (
        val productIdentifierType: Int,
        val productIdentifierValue: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TonerCartridgeMonitoringClusterReplacementProductStruct {\n")
        builder.append("\tproductIdentifierType : $productIdentifierType\n")
        builder.append("\tproductIdentifierValue : $productIdentifierValue\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), productIdentifierType)
        tlvWriter.put(ContextSpecificTag(1), productIdentifierValue)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TonerCartridgeMonitoringClusterReplacementProductStruct {
          tlvReader.enterStructure(tag)
          val productIdentifierType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val productIdentifierValue: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return TonerCartridgeMonitoringClusterReplacementProductStruct(productIdentifierType, productIdentifierValue)
        }
      }
    }
    
    class DoorLockClusterCredentialStruct (
        val credentialType: Int,
        val credentialIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("DoorLockClusterCredentialStruct {\n")
        builder.append("\tcredentialType : $credentialType\n")
        builder.append("\tcredentialIndex : $credentialIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), credentialType)
        tlvWriter.put(ContextSpecificTag(1), credentialIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterCredentialStruct {
          tlvReader.enterStructure(tag)
          val credentialType: Int = tlvReader.getInt(ContextSpecificTag(0))
          val credentialIndex: Int = tlvReader.getInt(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return DoorLockClusterCredentialStruct(credentialType, credentialIndex)
        }
      }
    }
    
    class ThermostatClusterThermostatScheduleTransition (
        val transitionTime: Int,
        val heatSetpoint: Int?,
        val coolSetpoint: Int?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ThermostatClusterThermostatScheduleTransition {\n")
        builder.append("\ttransitionTime : $transitionTime\n")
        builder.append("\theatSetpoint : $heatSetpoint\n")
        builder.append("\tcoolSetpoint : $coolSetpoint\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), transitionTime)
        if (heatSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), heatSetpoint)
    }
        if (coolSetpoint == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), coolSetpoint)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThermostatClusterThermostatScheduleTransition {
          tlvReader.enterStructure(tag)
          val transitionTime: Int = tlvReader.getInt(ContextSpecificTag(0))
          val heatSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          val coolSetpoint: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          
          tlvReader.exitContainer()

          return ThermostatClusterThermostatScheduleTransition(transitionTime, heatSetpoint, coolSetpoint)
        }
      }
    }
    
    class ChannelClusterChannelInfoStruct (
        val majorNumber: Int,
        val minorNumber: Int,
        val name: Optional<String>,
        val callSign: Optional<String>,
        val affiliateCallSign: Optional<String>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ChannelClusterChannelInfoStruct {\n")
        builder.append("\tmajorNumber : $majorNumber\n")
        builder.append("\tminorNumber : $minorNumber\n")
        builder.append("\tname : $name\n")
        builder.append("\tcallSign : $callSign\n")
        builder.append("\taffiliateCallSign : $affiliateCallSign\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), majorNumber)
        tlvWriter.put(ContextSpecificTag(1), minorNumber)
        if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(2), opt_name)
    }
        if (callSign.isPresent) {
      val opt_callSign = callSign.get()
      tlvWriter.put(ContextSpecificTag(3), opt_callSign)
    }
        if (affiliateCallSign.isPresent) {
      val opt_affiliateCallSign = affiliateCallSign.get()
      tlvWriter.put(ContextSpecificTag(4), opt_affiliateCallSign)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ChannelClusterChannelInfoStruct {
          tlvReader.enterStructure(tag)
          val majorNumber: Int = tlvReader.getInt(ContextSpecificTag(0))
          val minorNumber: Int = tlvReader.getInt(ContextSpecificTag(1))
          val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val callSign: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val affiliateCallSign: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return ChannelClusterChannelInfoStruct(majorNumber, minorNumber, name, callSign, affiliateCallSign)
        }
      }
    }
    
    class ChannelClusterLineupInfoStruct (
        val operatorName: String,
        val lineupName: Optional<String>,
        val postalCode: Optional<String>,
        val lineupInfoType: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ChannelClusterLineupInfoStruct {\n")
        builder.append("\toperatorName : $operatorName\n")
        builder.append("\tlineupName : $lineupName\n")
        builder.append("\tpostalCode : $postalCode\n")
        builder.append("\tlineupInfoType : $lineupInfoType\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), operatorName)
        if (lineupName.isPresent) {
      val opt_lineupName = lineupName.get()
      tlvWriter.put(ContextSpecificTag(1), opt_lineupName)
    }
        if (postalCode.isPresent) {
      val opt_postalCode = postalCode.get()
      tlvWriter.put(ContextSpecificTag(2), opt_postalCode)
    }
        tlvWriter.put(ContextSpecificTag(3), lineupInfoType)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ChannelClusterLineupInfoStruct {
          tlvReader.enterStructure(tag)
          val operatorName: String = tlvReader.getString(ContextSpecificTag(0))
          val lineupName: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val postalCode: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val lineupInfoType: Int = tlvReader.getInt(ContextSpecificTag(3))
          
          tlvReader.exitContainer()

          return ChannelClusterLineupInfoStruct(operatorName, lineupName, postalCode, lineupInfoType)
        }
      }
    }
    
    class TargetNavigatorClusterTargetInfoStruct (
        val identifier: Int,
        val name: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("TargetNavigatorClusterTargetInfoStruct {\n")
        builder.append("\tidentifier : $identifier\n")
        builder.append("\tname : $name\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), identifier)
        tlvWriter.put(ContextSpecificTag(1), name)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : TargetNavigatorClusterTargetInfoStruct {
          tlvReader.enterStructure(tag)
          val identifier: Int = tlvReader.getInt(ContextSpecificTag(0))
          val name: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return TargetNavigatorClusterTargetInfoStruct(identifier, name)
        }
      }
    }
    
    class MediaPlaybackClusterPlaybackPositionStruct (
        val updatedAt: Long,
        val position: Long?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("MediaPlaybackClusterPlaybackPositionStruct {\n")
        builder.append("\tupdatedAt : $updatedAt\n")
        builder.append("\tposition : $position\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), updatedAt)
        if (position == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), position)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : MediaPlaybackClusterPlaybackPositionStruct {
          tlvReader.enterStructure(tag)
          val updatedAt: Long = tlvReader.getLong(ContextSpecificTag(0))
          val position: Long? = try {
      tlvReader.getLong(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
          
          tlvReader.exitContainer()

          return MediaPlaybackClusterPlaybackPositionStruct(updatedAt, position)
        }
      }
    }
    
    class MediaInputClusterInputInfoStruct (
        val index: Int,
        val inputType: Int,
        val name: String,
        val description: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("MediaInputClusterInputInfoStruct {\n")
        builder.append("\tindex : $index\n")
        builder.append("\tinputType : $inputType\n")
        builder.append("\tname : $name\n")
        builder.append("\tdescription : $description\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), index)
        tlvWriter.put(ContextSpecificTag(1), inputType)
        tlvWriter.put(ContextSpecificTag(2), name)
        tlvWriter.put(ContextSpecificTag(3), description)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : MediaInputClusterInputInfoStruct {
          tlvReader.enterStructure(tag)
          val index: Int = tlvReader.getInt(ContextSpecificTag(0))
          val inputType: Int = tlvReader.getInt(ContextSpecificTag(1))
          val name: String = tlvReader.getString(ContextSpecificTag(2))
          val description: String = tlvReader.getString(ContextSpecificTag(3))
          
          tlvReader.exitContainer()

          return MediaInputClusterInputInfoStruct(index, inputType, name, description)
        }
      }
    }
    
    class ContentLauncherClusterDimensionStruct (
        val width: Double,
        val height: Double,
        val metric: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterDimensionStruct {\n")
        builder.append("\twidth : $width\n")
        builder.append("\theight : $height\n")
        builder.append("\tmetric : $metric\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), width)
        tlvWriter.put(ContextSpecificTag(1), height)
        tlvWriter.put(ContextSpecificTag(2), metric)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterDimensionStruct {
          tlvReader.enterStructure(tag)
          val width: Double = tlvReader.getDouble(ContextSpecificTag(0))
          val height: Double = tlvReader.getDouble(ContextSpecificTag(1))
          val metric: Int = tlvReader.getInt(ContextSpecificTag(2))
          
          tlvReader.exitContainer()

          return ContentLauncherClusterDimensionStruct(width, height, metric)
        }
      }
    }
    
    class ContentLauncherClusterAdditionalInfoStruct (
        val name: String,
        val value: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterAdditionalInfoStruct {\n")
        builder.append("\tname : $name\n")
        builder.append("\tvalue : $value\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), name)
        tlvWriter.put(ContextSpecificTag(1), value)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterAdditionalInfoStruct {
          tlvReader.enterStructure(tag)
          val name: String = tlvReader.getString(ContextSpecificTag(0))
          val value: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ContentLauncherClusterAdditionalInfoStruct(name, value)
        }
      }
    }
    
    class ContentLauncherClusterParameterStruct (
        val type: Int,
        val value: String,
        val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterParameterStruct {\n")
        builder.append("\ttype : $type\n")
        builder.append("\tvalue : $value\n")
        builder.append("\texternalIDList : $externalIDList\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), type)
        tlvWriter.put(ContextSpecificTag(1), value)
        if (externalIDList.isPresent) {
      val opt_externalIDList = externalIDList.get()
      tlvWriter.startList(ContextSpecificTag(2))
      val iter_opt_externalIDList = opt_externalIDList.iterator()
      while(iter_opt_externalIDList.hasNext()) {
        val next = iter_opt_externalIDList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterParameterStruct {
          tlvReader.enterStructure(tag)
          val type: Int = tlvReader.getInt(ContextSpecificTag(0))
          val value: String = tlvReader.getString(ContextSpecificTag(1))
          val externalIDList: Optional<List<ContentLauncherClusterAdditionalInfoStruct>> = try {
      Optional.of(mutableListOf<ContentLauncherClusterAdditionalInfoStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(ContentLauncherClusterAdditionalInfoStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return ContentLauncherClusterParameterStruct(type, value, externalIDList)
        }
      }
    }
    
    class ContentLauncherClusterContentSearchStruct (
        val parameterList: List<ContentLauncherClusterParameterStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterContentSearchStruct {\n")
        builder.append("\tparameterList : $parameterList\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_parameterList = parameterList.iterator()
      while(iter_parameterList.hasNext()) {
        val next = iter_parameterList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterContentSearchStruct {
          tlvReader.enterStructure(tag)
          val parameterList: List<ContentLauncherClusterParameterStruct> = mutableListOf<ContentLauncherClusterParameterStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(ContentLauncherClusterParameterStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return ContentLauncherClusterContentSearchStruct(parameterList)
        }
      }
    }
    
    class ContentLauncherClusterStyleInformationStruct (
        val imageURL: Optional<String>,
        val color: Optional<String>,
        val size: Optional<ContentLauncherClusterDimensionStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterStyleInformationStruct {\n")
        builder.append("\timageURL : $imageURL\n")
        builder.append("\tcolor : $color\n")
        builder.append("\tsize : $size\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (imageURL.isPresent) {
      val opt_imageURL = imageURL.get()
      tlvWriter.put(ContextSpecificTag(0), opt_imageURL)
    }
        if (color.isPresent) {
      val opt_color = color.get()
      tlvWriter.put(ContextSpecificTag(1), opt_color)
    }
        if (size.isPresent) {
      val opt_size = size.get()
      opt_size.toTlv(ContextSpecificTag(2), tlvWriter)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterStyleInformationStruct {
          tlvReader.enterStructure(tag)
          val imageURL: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val color: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val size: Optional<ContentLauncherClusterDimensionStruct> = try {
      Optional.of(ContentLauncherClusterDimensionStruct.fromTlv(ContextSpecificTag(2), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return ContentLauncherClusterStyleInformationStruct(imageURL, color, size)
        }
      }
    }
    
    class ContentLauncherClusterBrandingInformationStruct (
        val providerName: String,
        val background: Optional<ContentLauncherClusterStyleInformationStruct>,
        val logo: Optional<ContentLauncherClusterStyleInformationStruct>,
        val progressBar: Optional<ContentLauncherClusterStyleInformationStruct>,
        val splash: Optional<ContentLauncherClusterStyleInformationStruct>,
        val waterMark: Optional<ContentLauncherClusterStyleInformationStruct>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ContentLauncherClusterBrandingInformationStruct {\n")
        builder.append("\tproviderName : $providerName\n")
        builder.append("\tbackground : $background\n")
        builder.append("\tlogo : $logo\n")
        builder.append("\tprogressBar : $progressBar\n")
        builder.append("\tsplash : $splash\n")
        builder.append("\twaterMark : $waterMark\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), providerName)
        if (background.isPresent) {
      val opt_background = background.get()
      opt_background.toTlv(ContextSpecificTag(1), tlvWriter)
    }
        if (logo.isPresent) {
      val opt_logo = logo.get()
      opt_logo.toTlv(ContextSpecificTag(2), tlvWriter)
    }
        if (progressBar.isPresent) {
      val opt_progressBar = progressBar.get()
      opt_progressBar.toTlv(ContextSpecificTag(3), tlvWriter)
    }
        if (splash.isPresent) {
      val opt_splash = splash.get()
      opt_splash.toTlv(ContextSpecificTag(4), tlvWriter)
    }
        if (waterMark.isPresent) {
      val opt_waterMark = waterMark.get()
      opt_waterMark.toTlv(ContextSpecificTag(5), tlvWriter)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterBrandingInformationStruct {
          tlvReader.enterStructure(tag)
          val providerName: String = tlvReader.getString(ContextSpecificTag(0))
          val background: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(1), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val logo: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(2), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val progressBar: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(3), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val splash: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(4), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val waterMark: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(5), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return ContentLauncherClusterBrandingInformationStruct(providerName, background, logo, progressBar, splash, waterMark)
        }
      }
    }
    
    class AudioOutputClusterOutputInfoStruct (
        val index: Int,
        val outputType: Int,
        val name: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("AudioOutputClusterOutputInfoStruct {\n")
        builder.append("\tindex : $index\n")
        builder.append("\toutputType : $outputType\n")
        builder.append("\tname : $name\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), index)
        tlvWriter.put(ContextSpecificTag(1), outputType)
        tlvWriter.put(ContextSpecificTag(2), name)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : AudioOutputClusterOutputInfoStruct {
          tlvReader.enterStructure(tag)
          val index: Int = tlvReader.getInt(ContextSpecificTag(0))
          val outputType: Int = tlvReader.getInt(ContextSpecificTag(1))
          val name: String = tlvReader.getString(ContextSpecificTag(2))
          
          tlvReader.exitContainer()

          return AudioOutputClusterOutputInfoStruct(index, outputType, name)
        }
      }
    }
    
    class ApplicationLauncherClusterApplicationStruct (
        val catalogVendorID: Int,
        val applicationID: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ApplicationLauncherClusterApplicationStruct {\n")
        builder.append("\tcatalogVendorID : $catalogVendorID\n")
        builder.append("\tapplicationID : $applicationID\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), catalogVendorID)
        tlvWriter.put(ContextSpecificTag(1), applicationID)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ApplicationLauncherClusterApplicationStruct {
          tlvReader.enterStructure(tag)
          val catalogVendorID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val applicationID: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ApplicationLauncherClusterApplicationStruct(catalogVendorID, applicationID)
        }
      }
    }
    
    class ApplicationLauncherClusterApplicationEPStruct (
        val application: ApplicationLauncherClusterApplicationStruct,
        val endpoint: Optional<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ApplicationLauncherClusterApplicationEPStruct {\n")
        builder.append("\tapplication : $application\n")
        builder.append("\tendpoint : $endpoint\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        application.toTlv(ContextSpecificTag(0), tlvWriter)
        if (endpoint.isPresent) {
      val opt_endpoint = endpoint.get()
      tlvWriter.put(ContextSpecificTag(1), opt_endpoint)
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ApplicationLauncherClusterApplicationEPStruct {
          tlvReader.enterStructure(tag)
          val application: ApplicationLauncherClusterApplicationStruct = ApplicationLauncherClusterApplicationStruct.fromTlv(ContextSpecificTag(0), tlvReader)
          val endpoint: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          
          tlvReader.exitContainer()

          return ApplicationLauncherClusterApplicationEPStruct(application, endpoint)
        }
      }
    }
    
    class ApplicationBasicClusterApplicationStruct (
        val catalogVendorID: Int,
        val applicationID: String) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("ApplicationBasicClusterApplicationStruct {\n")
        builder.append("\tcatalogVendorID : $catalogVendorID\n")
        builder.append("\tapplicationID : $applicationID\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), catalogVendorID)
        tlvWriter.put(ContextSpecificTag(1), applicationID)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : ApplicationBasicClusterApplicationStruct {
          tlvReader.enterStructure(tag)
          val catalogVendorID: Int = tlvReader.getInt(ContextSpecificTag(0))
          val applicationID: String = tlvReader.getString(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return ApplicationBasicClusterApplicationStruct(catalogVendorID, applicationID)
        }
      }
    }
    
    class UnitTestingClusterSimpleStruct (
        val a: Int,
        val b: Boolean,
        val c: Int,
        val d: ByteArray,
        val e: String,
        val f: Int,
        val g: Float,
        val h: Double) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterSimpleStruct {\n")
        builder.append("\ta : $a\n")
        builder.append("\tb : $b\n")
        builder.append("\tc : $c\n")
        builder.append("\td : $d\n")
        builder.append("\te : $e\n")
        builder.append("\tf : $f\n")
        builder.append("\tg : $g\n")
        builder.append("\th : $h\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), a)
        tlvWriter.put(ContextSpecificTag(1), b)
        tlvWriter.put(ContextSpecificTag(2), c)
        tlvWriter.put(ContextSpecificTag(3), d)
        tlvWriter.put(ContextSpecificTag(4), e)
        tlvWriter.put(ContextSpecificTag(5), f)
        tlvWriter.put(ContextSpecificTag(6), g)
        tlvWriter.put(ContextSpecificTag(7), h)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterSimpleStruct {
          tlvReader.enterStructure(tag)
          val a: Int = tlvReader.getInt(ContextSpecificTag(0))
          val b: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
          val c: Int = tlvReader.getInt(ContextSpecificTag(2))
          val d: ByteArray = tlvReader.getByteArray(ContextSpecificTag(3))
          val e: String = tlvReader.getString(ContextSpecificTag(4))
          val f: Int = tlvReader.getInt(ContextSpecificTag(5))
          val g: Float = tlvReader.getFloat(ContextSpecificTag(6))
          val h: Double = tlvReader.getDouble(ContextSpecificTag(7))
          
          tlvReader.exitContainer()

          return UnitTestingClusterSimpleStruct(a, b, c, d, e, f, g, h)
        }
      }
    }
    
    class UnitTestingClusterTestFabricScoped (
        val fabricSensitiveInt8u: Int,
        val optionalFabricSensitiveInt8u: Optional<Int>,
        val nullableFabricSensitiveInt8u: Int?,
        val nullableOptionalFabricSensitiveInt8u: Optional<Int>?,
        val fabricSensitiveCharString: String,
        val fabricSensitiveStruct: UnitTestingClusterSimpleStruct,
        val fabricSensitiveInt8uList: List<Int>,
        val fabricIndex: Int) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterTestFabricScoped {\n")
        builder.append("\tfabricSensitiveInt8u : $fabricSensitiveInt8u\n")
        builder.append("\toptionalFabricSensitiveInt8u : $optionalFabricSensitiveInt8u\n")
        builder.append("\tnullableFabricSensitiveInt8u : $nullableFabricSensitiveInt8u\n")
        builder.append("\tnullableOptionalFabricSensitiveInt8u : $nullableOptionalFabricSensitiveInt8u\n")
        builder.append("\tfabricSensitiveCharString : $fabricSensitiveCharString\n")
        builder.append("\tfabricSensitiveStruct : $fabricSensitiveStruct\n")
        builder.append("\tfabricSensitiveInt8uList : $fabricSensitiveInt8uList\n")
        builder.append("\tfabricIndex : $fabricIndex\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(1), fabricSensitiveInt8u)
        if (optionalFabricSensitiveInt8u.isPresent) {
      val opt_optionalFabricSensitiveInt8u = optionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(2), opt_optionalFabricSensitiveInt8u)
    }
        if (nullableFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), nullableFabricSensitiveInt8u)
    }
        if (nullableOptionalFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      if (nullableOptionalFabricSensitiveInt8u.isPresent) {
      val opt_nullableOptionalFabricSensitiveInt8u = nullableOptionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(4), opt_nullableOptionalFabricSensitiveInt8u)
    }
    }
        tlvWriter.put(ContextSpecificTag(5), fabricSensitiveCharString)
        fabricSensitiveStruct.toTlv(ContextSpecificTag(6), tlvWriter)
        tlvWriter.startList(ContextSpecificTag(7))
      val iter_fabricSensitiveInt8uList = fabricSensitiveInt8uList.iterator()
      while(iter_fabricSensitiveInt8uList.hasNext()) {
        val next = iter_fabricSensitiveInt8uList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.put(ContextSpecificTag(254), fabricIndex)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestFabricScoped {
          tlvReader.enterStructure(tag)
          val fabricSensitiveInt8u: Int = tlvReader.getInt(ContextSpecificTag(1))
          val optionalFabricSensitiveInt8u: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val nullableFabricSensitiveInt8u: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val nullableOptionalFabricSensitiveInt8u: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
          val fabricSensitiveCharString: String = tlvReader.getString(ContextSpecificTag(5))
          val fabricSensitiveStruct: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(6), tlvReader)
          val fabricSensitiveInt8uList: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(7))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
          
          tlvReader.exitContainer()

          return UnitTestingClusterTestFabricScoped(fabricSensitiveInt8u, optionalFabricSensitiveInt8u, nullableFabricSensitiveInt8u, nullableOptionalFabricSensitiveInt8u, fabricSensitiveCharString, fabricSensitiveStruct, fabricSensitiveInt8uList, fabricIndex)
        }
      }
    }
    
    class UnitTestingClusterNullablesAndOptionalsStruct (
        val nullableInt: Int?,
        val optionalInt: Optional<Int>,
        val nullableOptionalInt: Optional<Int>?,
        val nullableString: String?,
        val optionalString: Optional<String>,
        val nullableOptionalString: Optional<String>?,
        val nullableStruct: UnitTestingClusterSimpleStruct?,
        val optionalStruct: Optional<UnitTestingClusterSimpleStruct>,
        val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>?,
        val nullableList: List<Int>?,
        val optionalList: Optional<List<Int>>,
        val nullableOptionalList: Optional<List<Int>>?) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterNullablesAndOptionalsStruct {\n")
        builder.append("\tnullableInt : $nullableInt\n")
        builder.append("\toptionalInt : $optionalInt\n")
        builder.append("\tnullableOptionalInt : $nullableOptionalInt\n")
        builder.append("\tnullableString : $nullableString\n")
        builder.append("\toptionalString : $optionalString\n")
        builder.append("\tnullableOptionalString : $nullableOptionalString\n")
        builder.append("\tnullableStruct : $nullableStruct\n")
        builder.append("\toptionalStruct : $optionalStruct\n")
        builder.append("\tnullableOptionalStruct : $nullableOptionalStruct\n")
        builder.append("\tnullableList : $nullableList\n")
        builder.append("\toptionalList : $optionalList\n")
        builder.append("\tnullableOptionalList : $nullableOptionalList\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        if (nullableInt == null) { tlvWriter.putNull(ContextSpecificTag(0)) }
    else {
      tlvWriter.put(ContextSpecificTag(0), nullableInt)
    }
        if (optionalInt.isPresent) {
      val opt_optionalInt = optionalInt.get()
      tlvWriter.put(ContextSpecificTag(1), opt_optionalInt)
    }
        if (nullableOptionalInt == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      if (nullableOptionalInt.isPresent) {
      val opt_nullableOptionalInt = nullableOptionalInt.get()
      tlvWriter.put(ContextSpecificTag(2), opt_nullableOptionalInt)
    }
    }
        if (nullableString == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), nullableString)
    }
        if (optionalString.isPresent) {
      val opt_optionalString = optionalString.get()
      tlvWriter.put(ContextSpecificTag(4), opt_optionalString)
    }
        if (nullableOptionalString == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      if (nullableOptionalString.isPresent) {
      val opt_nullableOptionalString = nullableOptionalString.get()
      tlvWriter.put(ContextSpecificTag(5), opt_nullableOptionalString)
    }
    }
        if (nullableStruct == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      nullableStruct.toTlv(ContextSpecificTag(6), tlvWriter)
    }
        if (optionalStruct.isPresent) {
      val opt_optionalStruct = optionalStruct.get()
      opt_optionalStruct.toTlv(ContextSpecificTag(7), tlvWriter)
    }
        if (nullableOptionalStruct == null) { tlvWriter.putNull(ContextSpecificTag(8)) }
    else {
      if (nullableOptionalStruct.isPresent) {
      val opt_nullableOptionalStruct = nullableOptionalStruct.get()
      opt_nullableOptionalStruct.toTlv(ContextSpecificTag(8), tlvWriter)
    }
    }
        if (nullableList == null) { tlvWriter.putNull(ContextSpecificTag(9)) }
    else {
      tlvWriter.startList(ContextSpecificTag(9))
      val iter_nullableList = nullableList.iterator()
      while(iter_nullableList.hasNext()) {
        val next = iter_nullableList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
        if (optionalList.isPresent) {
      val opt_optionalList = optionalList.get()
      tlvWriter.startList(ContextSpecificTag(10))
      val iter_opt_optionalList = opt_optionalList.iterator()
      while(iter_opt_optionalList.hasNext()) {
        val next = iter_opt_optionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
        if (nullableOptionalList == null) { tlvWriter.putNull(ContextSpecificTag(11)) }
    else {
      if (nullableOptionalList.isPresent) {
      val opt_nullableOptionalList = nullableOptionalList.get()
      tlvWriter.startList(ContextSpecificTag(11))
      val iter_opt_nullableOptionalList = opt_nullableOptionalList.iterator()
      while(iter_opt_nullableOptionalList.hasNext()) {
        val next = iter_opt_nullableOptionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    }
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNullablesAndOptionalsStruct {
          tlvReader.enterStructure(tag)
          val nullableInt: Int? = try {
      tlvReader.getInt(ContextSpecificTag(0))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(0))
      null
    }
          val optionalInt: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val nullableOptionalInt: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
          val nullableString: String? = try {
      tlvReader.getString(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
          val optionalString: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val nullableOptionalString: Optional<String>? = try {
      try {
      Optional.of(tlvReader.getString(ContextSpecificTag(5)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
          val nullableStruct: UnitTestingClusterSimpleStruct? = try {
      UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(6), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
          val optionalStruct: Optional<UnitTestingClusterSimpleStruct> = try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(7), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>? = try {
      try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(8), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(8))
      null
    }
          val nullableList: List<Int>? = try {
      mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(9))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(9))
      null
    }
          val optionalList: Optional<List<Int>> = try {
      Optional.of(mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(10))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
          val nullableOptionalList: Optional<List<Int>>? = try {
      try {
      Optional.of(mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(11))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(11))
      null
    }
          
          tlvReader.exitContainer()

          return UnitTestingClusterNullablesAndOptionalsStruct(nullableInt, optionalInt, nullableOptionalInt, nullableString, optionalString, nullableOptionalString, nullableStruct, optionalStruct, nullableOptionalStruct, nullableList, optionalList, nullableOptionalList)
        }
      }
    }
    
    class UnitTestingClusterNestedStruct (
        val a: Int,
        val b: Boolean,
        val c: UnitTestingClusterSimpleStruct) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterNestedStruct {\n")
        builder.append("\ta : $a\n")
        builder.append("\tb : $b\n")
        builder.append("\tc : $c\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), a)
        tlvWriter.put(ContextSpecificTag(1), b)
        c.toTlv(ContextSpecificTag(2), tlvWriter)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNestedStruct {
          tlvReader.enterStructure(tag)
          val a: Int = tlvReader.getInt(ContextSpecificTag(0))
          val b: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
          val c: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(2), tlvReader)
          
          tlvReader.exitContainer()

          return UnitTestingClusterNestedStruct(a, b, c)
        }
      }
    }
    
    class UnitTestingClusterNestedStructList (
        val a: Int,
        val b: Boolean,
        val c: UnitTestingClusterSimpleStruct,
        val d: List<UnitTestingClusterSimpleStruct>,
        val e: List<Long>,
        val f: List<ByteArray>,
        val g: List<Int>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterNestedStructList {\n")
        builder.append("\ta : $a\n")
        builder.append("\tb : $b\n")
        builder.append("\tc : $c\n")
        builder.append("\td : $d\n")
        builder.append("\te : $e\n")
        builder.append("\tf : $f\n")
        builder.append("\tg : $g\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), a)
        tlvWriter.put(ContextSpecificTag(1), b)
        c.toTlv(ContextSpecificTag(2), tlvWriter)
        tlvWriter.startList(ContextSpecificTag(3))
      val iter_d = d.iterator()
      while(iter_d.hasNext()) {
        val next = iter_d.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(4))
      val iter_e = e.iterator()
      while(iter_e.hasNext()) {
        val next = iter_e.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(5))
      val iter_f = f.iterator()
      while(iter_f.hasNext()) {
        val next = iter_f.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.startList(ContextSpecificTag(6))
      val iter_g = g.iterator()
      while(iter_g.hasNext()) {
        val next = iter_g.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNestedStructList {
          tlvReader.enterStructure(tag)
          val a: Int = tlvReader.getInt(ContextSpecificTag(0))
          val b: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
          val c: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(2), tlvReader)
          val d: List<UnitTestingClusterSimpleStruct> = mutableListOf<UnitTestingClusterSimpleStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(3))
      while(true) {
        try {
          this.add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val e: List<Long> = mutableListOf<Long>().apply {
      tlvReader.enterList(ContextSpecificTag(4))
      while(true) {
        try {
          this.add(tlvReader.getLong(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          val f: List<ByteArray> = mutableListOf<ByteArray>().apply {
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
          val g: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return UnitTestingClusterNestedStructList(a, b, c, d, e, f, g)
        }
      }
    }
    
    class UnitTestingClusterDoubleNestedStructList (
        val a: List<UnitTestingClusterNestedStructList>) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterDoubleNestedStructList {\n")
        builder.append("\ta : $a\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.startList(ContextSpecificTag(0))
      val iter_a = a.iterator()
      while(iter_a.hasNext()) {
        val next = iter_a.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterDoubleNestedStructList {
          tlvReader.enterStructure(tag)
          val a: List<UnitTestingClusterNestedStructList> = mutableListOf<UnitTestingClusterNestedStructList>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(UnitTestingClusterNestedStructList.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
          
          tlvReader.exitContainer()

          return UnitTestingClusterDoubleNestedStructList(a)
        }
      }
    }
    
    class UnitTestingClusterTestListStructOctet (
        val member1: Long,
        val member2: ByteArray) {
      override fun toString() : String {
        val builder: StringBuilder = StringBuilder()
        builder.append("UnitTestingClusterTestListStructOctet {\n")
        builder.append("\tmember1 : $member1\n")
        builder.append("\tmember2 : $member2\n")
        builder.append("}\n")
        return builder.toString()
      }

      fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
        tlvWriter.startStructure(tag)
        tlvWriter.put(ContextSpecificTag(0), member1)
        tlvWriter.put(ContextSpecificTag(1), member2)
        tlvWriter.endStructure()
      }

      companion object {
        fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestListStructOctet {
          tlvReader.enterStructure(tag)
          val member1: Long = tlvReader.getLong(ContextSpecificTag(0))
          val member2: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
          
          tlvReader.exitContainer()

          return UnitTestingClusterTestListStructOctet(member1, member2)
        }
      }
    }
    }
