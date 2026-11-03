/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

import android.content.Context
import chip.devicecontroller.ChipStructs
import chip.devicecontroller.model.AttributeState
import chip.devicecontroller.model.NodeState
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import java.util.concurrent.ConcurrentHashMap
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader

/**
 * Standard Matter Device Type IDs mapped from the Matter SDK data model specification:
 * `src/app/zap-templates/zcl/data-model/chip/matter-devices.xml`.
 */
object MatterDeviceTypes {
  const val DOOR_LOCK = 0x000AL
  const val DOOR_LOCK_CONTROLLER = 0x000BL
  const val AGGREGATOR = 0x000EL
  const val GENERIC_SWITCH = 0x000FL
  const val POWER_SOURCE = 0x0011L
  const val OTA_REQUESTOR = 0x0012L
  const val BRIDGED_NODE = 0x0013L
  const val OTA_PROVIDER = 0x0014L
  const val CONTACT_SENSOR = 0x0015L
  const val ROOT_NODE = 0x0016L
  const val SPEAKER = 0x0022L
  const val CASTING_VIDEO_PLAYER = 0x0023L
  const val CONTENT_APP = 0x0024L
  const val MODE_SELECT = 0x0027L
  const val BASIC_VIDEO_PLAYER = 0x0028L
  const val CASTING_VIDEO_CLIENT = 0x0029L
  const val VIDEO_REMOTE_CONTROL = 0x002AL
  const val FAN = 0x002BL
  const val AIR_QUALITY_SENSOR = 0x002CL
  const val AIR_PURIFIER = 0x002DL
  const val WATER_FREEZE_DETECTOR = 0x0041L
  const val WATER_VALVE = 0x0042L
  const val WATER_LEAK_DETECTOR = 0x0043L
  const val RAIN_SENSOR = 0x0044L
  const val REFRIGERATOR = 0x0070L
  const val ROOM_AIR_CONDITIONER = 0x0072L
  const val LAUNDRY_WASHER = 0x0073L
  const val ROBOTIC_VACUUM_CLEANER = 0x0074L
  const val DISHWASHER = 0x0075L
  const val SMOKE_CO_ALARM = 0x0076L
  const val COOK_SURFACE = 0x0077L
  const val COOKTOP = 0x0078L
  const val MICROWAVE_OVEN = 0x0079L
  const val EXTRACTOR_HOOD = 0x007AL
  const val OVEN = 0x007BL
  const val LAUNDRY_DRYER = 0x007CL
  const val ON_OFF_LIGHT = 0x0100L
  const val DIMMABLE_LIGHT = 0x0101L
  const val COLOR_TEMPERATURE_LIGHT = 0x010CL
  const val EXTENDED_COLOR_LIGHT = 0x010DL
  const val ON_OFF_LIGHT_SWITCH = 0x0103L
  const val DIMMER_SWITCH = 0x0104L
  const val COLOR_DIMMER_SWITCH = 0x0105L
  const val LIGHT_SENSOR = 0x0106L
  const val OCCUPANCY_SENSOR = 0x0107L
  const val ON_OFF_PLUG_IN_UNIT = 0x010AL
  const val DIMMABLE_PLUG_IN_UNIT = 0x010BL
  const val PUMP = 0x0303L
  const val PUMP_CONTROLLER = 0x0304L
  const val PRESSURE_SENSOR = 0x0305L
  const val FLOW_SENSOR = 0x0306L
  const val WINDOW_COVERING = 0x0202L
  const val WINDOW_COVERING_CONTROLLER = 0x0203L
  const val HEATING_COOLING_UNIT = 0x0300L
  const val THERMOSTAT = 0x0301L
  const val TEMPERATURE_SENSOR = 0x0302L
  const val HUMIDITY_SENSOR = 0x0307L
  const val THERMOSTAT_CONTROLLER = 0x030AL
  const val EVSE = 0x050CL
  const val DEVICE_ENERGY_MANAGEMENT = 0x050DL
  const val WATER_HEATER = 0x050FL
  const val SOLAR_POWER = 0x0017L
  const val BATTERY_STORAGE = 0x0018L
  const val ELECTRICAL_SENSOR = 0x0510L

  private val utilityTypes =
    setOf(ROOT_NODE, POWER_SOURCE, OTA_REQUESTOR, BRIDGED_NODE, AGGREGATOR, OTA_PROVIDER)

  fun selectPrimaryDeviceType(deviceTypes: List<Long>): Long {
    return deviceTypes.lastOrNull { it !in utilityTypes } ?: deviceTypes.firstOrNull() ?: 0L
  }

  fun nameForId(deviceTypeId: Long): String =
    when (deviceTypeId) {
      DOOR_LOCK -> "Door Lock"
      DOOR_LOCK_CONTROLLER -> "Door Lock Controller"
      AGGREGATOR -> "Aggregator"
      GENERIC_SWITCH -> "Generic Switch"
      POWER_SOURCE -> "Power Source"
      OTA_REQUESTOR -> "OTA Requestor"
      BRIDGED_NODE -> "Bridged Node"
      OTA_PROVIDER -> "OTA Provider"
      CONTACT_SENSOR -> "Contact Sensor"
      ROOT_NODE -> "Root Node"
      SOLAR_POWER -> "Solar Power"
      BATTERY_STORAGE -> "Battery Storage"
      SPEAKER -> "Speaker"
      CASTING_VIDEO_PLAYER -> "Casting Video Player"
      CONTENT_APP -> "Content App"
      MODE_SELECT -> "Mode Select"
      BASIC_VIDEO_PLAYER -> "Basic Video Player"
      CASTING_VIDEO_CLIENT -> "Casting Video Client"
      VIDEO_REMOTE_CONTROL -> "Video Remote Control"
      FAN -> "Fan"
      AIR_QUALITY_SENSOR -> "Air Quality Sensor"
      AIR_PURIFIER -> "Air Purifier"
      WATER_FREEZE_DETECTOR -> "Water Freeze Detector"
      WATER_VALVE -> "Water Valve"
      WATER_LEAK_DETECTOR -> "Water Leak Detector"
      RAIN_SENSOR -> "Rain Sensor"
      REFRIGERATOR -> "Refrigerator"
      ROOM_AIR_CONDITIONER -> "Room Air Conditioner"
      LAUNDRY_WASHER -> "Laundry Washer"
      ROBOTIC_VACUUM_CLEANER -> "Robotic Vacuum Cleaner"
      DISHWASHER -> "Dishwasher"
      SMOKE_CO_ALARM -> "Smoke CO Alarm"
      COOK_SURFACE -> "Cook Surface"
      COOKTOP -> "Cooktop"
      MICROWAVE_OVEN -> "Microwave Oven"
      EXTRACTOR_HOOD -> "Extractor Hood"
      OVEN -> "Oven"
      LAUNDRY_DRYER -> "Laundry Dryer"
      ON_OFF_LIGHT -> "On/Off Light"
      DIMMABLE_LIGHT -> "Dimmable Light"
      ON_OFF_LIGHT_SWITCH -> "On/Off Light Switch"
      DIMMER_SWITCH -> "Dimmer Switch"
      COLOR_DIMMER_SWITCH -> "Color Dimmer Switch"
      LIGHT_SENSOR -> "Light Sensor"
      OCCUPANCY_SENSOR -> "Occupancy Sensor"
      ON_OFF_PLUG_IN_UNIT -> "On/Off Plug-in Unit"
      DIMMABLE_PLUG_IN_UNIT -> "Dimmable Plug-in Unit"
      COLOR_TEMPERATURE_LIGHT -> "Color Temperature Light"
      EXTENDED_COLOR_LIGHT -> "Extended Color Light"
      WINDOW_COVERING -> "Window Covering"
      WINDOW_COVERING_CONTROLLER -> "Window Covering Controller"
      HEATING_COOLING_UNIT -> "Heating/Cooling Unit"
      THERMOSTAT -> "Thermostat"
      TEMPERATURE_SENSOR -> "Temperature Sensor"
      PUMP -> "Pump"
      PUMP_CONTROLLER -> "Pump Controller"
      PRESSURE_SENSOR -> "Pressure Sensor"
      FLOW_SENSOR -> "Flow Sensor"
      HUMIDITY_SENSOR -> "Humidity Sensor"
      THERMOSTAT_CONTROLLER -> "Thermostat Controller"
      EVSE -> "EVSE"
      DEVICE_ENERGY_MANAGEMENT -> "Device Energy Management"
      WATER_HEATER -> "Water Heater"
      ELECTRICAL_SENSOR -> "Electrical Sensor"
      else -> String.format(java.util.Locale.US, "Device Type 0x%04X", deviceTypeId)
    }
}

/** Represents an endpoint on a commissioned Matter Node. */
data class CommissionedEndpoint(
  val endpointId: Int,
  val deviceTypeId: Long,
  val deviceTypeName: String,
  val serverClusters: Set<Long> = emptySet(),
  val clientClusters: Set<Long> = emptySet(),
) {
  fun supportsCluster(clusterId: Long): Boolean = serverClusters.contains(clusterId)
}

/** Represents a commissioned Matter Node in the local fabric. */
data class CommissionedNode(
  val nodeId: Long,
  val nodeLabel: String,
  val roomName: String = "Default Room",
  val vendorName: String = "Standard Matter Vendor",
  val productName: String = "Matter Device",
  val endpoints: List<CommissionedEndpoint> = emptyList(),
  val isOnline: Boolean = true,
  val aliases: List<String> = emptyList(),
  val batteryPercent: Int? = null,
  val liveStates: Map<String, Any?> = emptyMap(),
  val descriptorVerified: Boolean = false,
) {
  fun getEndpointForCluster(clusterId: Long): CommissionedEndpoint? {
    return endpoints.firstOrNull { it.supportsCluster(clusterId) }
      ?: HomeStateCache.findAll(clusterId, 0x0000L)
        .firstOrNull { it.nodeId == nodeId }
        ?.let {
          CommissionedEndpoint(
            endpointId = it.endpointId,
            deviceTypeId = 0L,
            deviceTypeName = MatterDeviceTypes.nameForId(0L),
            serverClusters = setOf(clusterId),
          )
        }
  }

  fun getAllSupportedClusterIds(): Set<Long> {
    return endpoints.flatMap { it.serverClusters }.toSet()
  }

  fun getSupportedClusterMetas(): List<MatterClusterMeta> {
    return getAllSupportedClusterIds().mapNotNull { MatterClusterMetaRegistry.getCluster(it) }
  }
}

/**
 * Fabric-aware registry that manages active commissioned Matter nodes and their discovered
 * endpoints, clusters, and room placement for the Room-Centric Device Center UI.
 */
class CommissionedNodeRegistry(context: Context? = null) {
  private val appContext: Context? = context?.applicationContext ?: context
  private val nodes = ConcurrentHashMap<Long, CommissionedNode>()
  private val listeners = mutableListOf<(CommissionedNode) -> Unit>()
  private val gson = Gson()

  init {
    context?.let { loadFabricFromPreferences(it) }
  }

  fun addNodeRegistryChangeListener(listener: (CommissionedNode) -> Unit) {
    synchronized(listeners) { listeners.add(listener) }
  }

  fun removeNodeRegistryChangeListener(listener: (CommissionedNode) -> Unit) {
    synchronized(listeners) { listeners.remove(listener) }
  }

  private fun notifyNodeUpdated(node: CommissionedNode) {
    val snapshot = synchronized(listeners) { listeners.toList() }
    snapshot.forEach { it.invoke(node) }
  }

  companion object {
    val STANDARD_ROOT_NODE_CLUSTERS: Set<Long> =
      setOf(
        MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR, // 0x001DL
        MatterClusterMetaRegistry.CLUSTER_ACCESS_CONTROL, // 0x001FL
        MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION, // 0x0028L
        MatterClusterMetaRegistry.CLUSTER_GENERAL_COMMISSIONING, // 0x0030L
        MatterClusterMetaRegistry.CLUSTER_NETWORK_COMMISSIONING, // 0x0031L
        MatterClusterMetaRegistry.CLUSTER_GENERAL_DIAGNOSTICS, // 0x0033L
        MatterClusterMetaRegistry.CLUSTER_ADMINISTRATOR_COMMISSIONING, // 0x003CL
        MatterClusterMetaRegistry.CLUSTER_OPERATIONAL_CREDENTIALS, // 0x003EL
        MatterClusterMetaRegistry.CLUSTER_GROUP_KEY_MANAGEMENT, // 0x003FL
      )

    fun sanitizeMatterString(raw: String?): String {
      if (raw == null) return ""
      val cleaned =
        raw
          .filter { ch -> ch >= ' ' && ch != '\u007F' && ch != '\uFFFD' && ch.code != 0xFFFF }
          .trim()
      return if (cleaned.any { it.isLetterOrDigit() }) cleaned else ""
    }

    fun resolveDisplayLabel(
      nodeId: Long,
      rawLabel: String?,
      vendorName: String?,
      productName: String?,
    ): String {
      val cleanLabel = sanitizeMatterString(rawLabel)
      if (cleanLabel.isNotEmpty()) return cleanLabel
      val cleanVendor =
        sanitizeMatterString(vendorName).takeIf { it != "Standard Matter Vendor" }.orEmpty()
      val cleanProduct =
        sanitizeMatterString(productName).takeIf { it != "Matter Device" }.orEmpty()
      val combined = "$cleanVendor $cleanProduct".trim()
      return if (combined.isNotEmpty()) combined else "Device $nodeId"
    }
  }

  private fun normalizeRootEndpoint(ep: CommissionedEndpoint): CommissionedEndpoint {
    if (ep.endpointId != 0) return ep
    if (ep.serverClusters.size > 1 && ep.serverClusters.containsAll(STANDARD_ROOT_NODE_CLUSTERS)) {
      return ep
    }
    return ep.copy(serverClusters = ep.serverClusters + STANDARD_ROOT_NODE_CLUSTERS)
  }

  private fun defaultRootEndpoint(): CommissionedEndpoint =
    CommissionedEndpoint(
      endpointId = 0,
      deviceTypeId = 0x0016L,
      deviceTypeName = "Root Node",
      serverClusters = STANDARD_ROOT_NODE_CLUSTERS,
    )

  private fun putNormalizedNode(node: CommissionedNode): CommissionedNode {
    val normalizedNode = node.copy(endpoints = node.endpoints.map(::normalizeRootEndpoint))
    nodes[normalizedNode.nodeId] = normalizedNode
    return normalizedNode
  }

  fun registerNode(node: CommissionedNode) {
    val normalizedNode = putNormalizedNode(node)
    notifyNodeUpdated(normalizedNode)
  }

  fun unregisterNode(nodeId: Long): CommissionedNode? {
    val removed = nodes.remove(nodeId)
    if (removed != null) {
      notifyNodeUpdated(removed.copy(isOnline = false))
    }
    return removed
  }

  fun getNode(nodeId: Long): CommissionedNode? = nodes[nodeId]

  fun getAllNodes(): List<CommissionedNode> = nodes.values.toList()

  fun getAllRooms(): List<String> {
    return nodes.values.map { it.roomName }.distinct().sorted()
  }

  fun getNodesInRoom(roomName: String): List<CommissionedNode> {
    return nodes.values.filter { it.roomName.equals(roomName, ignoreCase = true) }
  }

  fun autoRegisterCommissionedNode(
    nodeId: Long,
    defaultLabel: String? = null,
    productName: String? = null,
    vendorName: String? = null,
    roomName: String = "Unassigned",
  ): CommissionedNode {
    val existing = nodes[nodeId]
    if (existing != null) {
      return existing
    }
    val label = defaultLabel ?: productName ?: "Device $nodeId"
    val node =
      CommissionedNode(
        nodeId = nodeId,
        nodeLabel = label,
        roomName = roomName.ifBlank { "Unassigned" },
        vendorName = vendorName ?: "Standard Matter Vendor",
        productName = productName ?: "Matter Device",
        endpoints =
          listOf(
            CommissionedEndpoint(
              endpointId = 0,
              deviceTypeId = 0x0016L,
              deviceTypeName = "Root Node",
              serverClusters = STANDARD_ROOT_NODE_CLUSTERS,
            )
          ),
        isOnline = true,
        aliases = emptyList(),
        descriptorVerified = false,
      )
    registerNode(node)
    return node
  }

  fun updateNodeMetadata(
    nodeId: Long,
    nodeLabel: String,
    roomName: String,
    aliases: List<String>,
  ): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing ->
        existing.copy(
          nodeLabel = nodeLabel.trim(),
          roomName = roomName.trim().ifEmpty { "Unassigned" },
          aliases = aliases.map { it.trim() }.filter { it.isNotEmpty() },
        )
      }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeRoom(nodeId: Long, newRoomName: String): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing -> existing.copy(roomName = newRoomName.trim()) }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeLabel(nodeId: Long, newLabel: String): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing -> existing.copy(nodeLabel = newLabel.trim()) }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeAliases(nodeId: Long, aliases: List<String>): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing ->
        existing.copy(aliases = aliases.map { it.trim() }.filter { it.isNotEmpty() })
      }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeOnlineStatus(nodeId: Long, isOnline: Boolean): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing -> existing.copy(isOnline = isOnline) }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeBattery(nodeId: Long, batteryPercent: Int?): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing ->
        existing.copy(batteryPercent = batteryPercent)
      }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun updateNodeLiveState(nodeId: Long, key: String, value: Any?): CommissionedNode? {
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing ->
        val newLiveStates = existing.liveStates.toMutableMap()
        newLiveStates[key] = value
        existing.copy(liveStates = newLiveStates)
      }
        ?: return null
    notifyNodeUpdated(updated)
    return updated
  }

  fun clear() {
    nodes.clear()
  }

  fun clearAllNodes(context: Context? = null) {
    val existingIds = nodes.keys.toList()
    nodes.clear()
    for (nodeId in existingIds) {
      try {
        CenterConnectionManager.onNodeRemoved(nodeId)
      } catch (_: Throwable) {}
    }
    try {
      HomeStateCache.clear()
    } catch (_: Throwable) {}
    context?.let { ctx ->
      val prefs = ctx.getSharedPreferences("matter_device_center_fabric", Context.MODE_PRIVATE)
      prefs.edit().remove("commissioned_nodes").apply()
    }
    notifyNodeUpdated(CommissionedNode(nodeId = -1L, nodeLabel = "", isOnline = false))
  }

  fun resetFabric(context: Context? = null) {
    clearAllNodes(context)
  }

  fun count(): Int = nodes.size

  /** Returns all unique server cluster IDs across all commissioned nodes in this fabric. */
  fun getFabricClusterIds(): Set<Long> {
    return nodes.values.flatMap { it.getAllSupportedClusterIds() }.toSet()
  }

  /** Returns all clusters supported on this fabric. */
  fun getFabricClusters(): List<MatterClusterMeta> {
    return getFabricClusterIds().mapNotNull { MatterClusterMetaRegistry.getCluster(it) }
  }

  /** Finds nodes supporting a specific cluster ID. */
  fun findNodesSupportingCluster(clusterId: Long): List<CommissionedNode> {
    return nodes.values.filter { it.getEndpointForCluster(clusterId) != null }
  }

  private val lightDeviceTypes =
    setOf(
      MatterDeviceTypes.ON_OFF_LIGHT,
      MatterDeviceTypes.DIMMABLE_LIGHT,
      MatterDeviceTypes.COLOR_TEMPERATURE_LIGHT,
      MatterDeviceTypes.EXTENDED_COLOR_LIGHT,
    )

  fun isLightNode(node: CommissionedNode): Boolean {
    return node.endpoints.any { it.deviceTypeId in lightDeviceTypes }
  }

  /**
   * Finds nodes matching a target text (matches nodeLabel, roomName, aliases, or deviceTypeName).
   */
  fun findNodesByQuery(query: String): List<CommissionedNode> {
    val clean =
      query
        .trim()
        .lowercase()
        .split(' ', '\t', '\n', '\r')
        .filter { it.isNotEmpty() && it != "the" && it != "a" && it != "an" && it != "my" }
        .joinToString(" ")
        .trim()
    if (clean.isEmpty()) return emptyList()
    val directMatches =
      nodes.values.filter { node ->
        node.nodeLabel.lowercase().contains(clean) ||
          clean.contains(node.nodeLabel.lowercase()) ||
          node.roomName.lowercase().contains(clean) ||
          clean.contains(node.roomName.lowercase()) ||
          "${node.roomName} ${node.nodeLabel}".lowercase().contains(clean) ||
          node.aliases.any { it.lowercase().contains(clean) || clean.contains(it.lowercase()) } ||
          node.endpoints.any { it.deviceTypeName.lowercase().contains(clean) }
      }
    if (directMatches.isNotEmpty()) return directMatches
    if (
      clean == "light" ||
        clean == "lights" ||
        clean == "bulb" ||
        clean == "bulbs" ||
        clean == "lamp" ||
        clean == "lamps"
    ) {
      return nodes.values.filter { isLightNode(it) }
    }
    return emptyList()
  }

  /** Resolves target node and endpoint given a query and cluster ID. */
  fun resolveTarget(
    query: String?,
    clusterId: Long,
    includeOffline: Boolean = false,
  ): Pair<CommissionedNode, CommissionedEndpoint>? {
    if (query.isNullOrBlank() || findNodesSupportingCluster(clusterId).isEmpty()) return null
    return pairWithClusterEndpoint(filterScopedClusterNodes(query, clusterId), clusterId)
      .firstOrNull()
  }

  fun resolveTargets(
    scope: String?,
    clusterId: Long,
    includeOffline: Boolean = false,
  ): List<Pair<CommissionedNode, CommissionedEndpoint>> {
    val candidates = findNodesSupportingCluster(clusterId)
    if (candidates.isEmpty()) return emptyList()
    if (scope.isNullOrBlank() || isHomeWideScope(scope)) {
      return pairWithClusterEndpoint(candidates, clusterId)
    }
    return pairWithClusterEndpoint(filterScopedClusterNodes(scope, clusterId), clusterId)
  }

  private fun isHomeWideScope(scope: String): Boolean =
    scope.trim().lowercase() in setOf("home", "house", "all", "every", "everywhere", "each room")

  private fun filterScopedClusterNodes(query: String, clusterId: Long): List<CommissionedNode> {
    val clean = query.trim().lowercase()
    val matched = findNodesByQuery(query).filter { it.getEndpointForCluster(clusterId) != null }
    if (!clean.contains("light") && !clean.contains("bulb") && !clean.contains("lamp")) {
      return matched
    }
    return matched.filter { isLightNode(it) }.ifEmpty { matched }
  }

  private fun pairWithClusterEndpoint(
    candidates: List<CommissionedNode>,
    clusterId: Long,
  ): List<Pair<CommissionedNode, CommissionedEndpoint>> =
    candidates.mapNotNull { node -> node.getEndpointForCluster(clusterId)?.let { node to it } }

  /** Serializes current fabric nodes to JSON. */
  fun toJson(): String {
    return gson.toJson(nodes.values)
  }

  /** Loads nodes from JSON. */
  fun loadFromJson(json: String) {
    val listType = object : TypeToken<List<CommissionedNode>>() {}.type
    val loaded: List<CommissionedNode> = gson.fromJson(json, listType) ?: emptyList()
    nodes.clear()
    var lastNode: CommissionedNode? = null
    for (node in loaded) {
      lastNode = putNormalizedNode(node)
    }
    if (lastNode != null) {
      notifyNodeUpdated(lastNode)
    }
  }

  fun saveFabricToPreferences(context: Context) {
    try {
      val prefs =
        context.getSharedPreferences("matter_device_center_fabric", Context.MODE_PRIVATE) ?: return
      val persistableNodes =
        nodes.values.filter { it.vendorName != "SyntheticVendor" && it.vendorName != "BenchVendor" }
      val json = gson.toJson(persistableNodes)
      prefs.edit().putString("commissioned_nodes", json).apply()
    } catch (e: Exception) {
      // Safe fallback for unit test mock contexts
    }
  }

  fun loadFabricFromPreferences(context: Context) {
    try {
      val prefs = context.getSharedPreferences("matter_device_center_fabric", Context.MODE_PRIVATE)
      val json = prefs?.getString("commissioned_nodes", null)
      var upgradedNames = false
      val type = object : TypeToken<List<CommissionedNode>>() {}.type
      val loadedNodes: List<CommissionedNode>? =
        if (json != null) gson.fromJson(json, type) else null
      if (loadedNodes != null) {
        nodes.clear()
        for (node in loadedNodes) {
          val normalized = normalizeLoadedNode(node)
          if (normalized != node) upgradedNames = true
          nodes[node.nodeId] = normalized
        }
      }
      if (upgradedNames) {
        saveFabricToPreferences(context)
      }
      syncWithDeviceIdUtil(context)
    } catch (e: Exception) {
      // Fallback
    }
  }

  private fun normalizeLoadedEndpoint(ep: CommissionedEndpoint): CommissionedEndpoint {
    val sdkName = MatterDeviceTypes.nameForId(ep.deviceTypeId)
    val withSdkName = if (ep.deviceTypeName != sdkName) ep.copy(deviceTypeName = sdkName) else ep
    return normalizeRootEndpoint(withSdkName)
  }

  private fun normalizeLoadedNode(node: CommissionedNode): CommissionedNode {
    val mapped = node.endpoints.map(::normalizeLoadedEndpoint)
    val finalEndpoints =
      if (mapped.none { it.endpointId == 0 }) listOf(defaultRootEndpoint()) + mapped else mapped
    val resolvedLabel =
      resolveDisplayLabel(node.nodeId, node.nodeLabel, node.vendorName, node.productName)
    return node.copy(nodeLabel = resolvedLabel, endpoints = finalEndpoints)
  }

  fun upgradeEp0Clusters(nodeId: Long): CommissionedNode? {
    var changed = false
    val updated =
      nodes.computeIfPresent(nodeId) { _, node ->
        val next = node.copy(endpoints = ensureRootAndNormalizeEndpoints(node.endpoints))
        changed = (next != node)
        next
      }
        ?: return null
    if (changed) {
      notifyNodeUpdated(updated)
    }
    return updated
  }

  private fun ensureRootAndNormalizeEndpoints(
    endpoints: List<CommissionedEndpoint>
  ): List<CommissionedEndpoint> {
    if (endpoints.none { it.endpointId == 0 }) {
      return listOf(defaultRootEndpoint()) + endpoints
    }
    return endpoints.map(::normalizeRootEndpoint)
  }

  fun recordDiscoveredEndpointClusterList(
    context: Context?,
    nodeId: Long,
    endpointId: Int,
    clusterIds: Set<Long>,
  ): CommissionedNode? {
    var changed = false
    val updatedNode =
      nodes.computeIfPresent(nodeId) { _, existing ->
        val next =
          existing.copy(
            endpoints = mergeEndpointClusters(existing.endpoints, endpointId, clusterIds)
          )
        changed = (next != existing)
        next
      }
        ?: return null
    if (changed) {
      context?.let { saveFabricToPreferences(it) }
      notifyNodeUpdated(updatedNode)
    }
    return updatedNode
  }

  private fun mergeEndpointClusters(
    endpoints: List<CommissionedEndpoint>,
    endpointId: Int,
    clusterIds: Set<Long>,
  ): List<CommissionedEndpoint> {
    val existingEp = endpoints.firstOrNull { it.endpointId == endpointId }
    if (existingEp != null) {
      val updatedEp = existingEp.copy(serverClusters = existingEp.serverClusters + clusterIds)
      return endpoints.map { if (it.endpointId == endpointId) updatedEp else it }
    }
    val devTypeId = if (endpointId == 0) 0x0016L else 0L
    val newEp =
      CommissionedEndpoint(
        endpointId = endpointId,
        deviceTypeId = devTypeId,
        deviceTypeName = MatterDeviceTypes.nameForId(devTypeId),
        serverClusters = clusterIds,
      )
    return (endpoints + newEp).sortedBy { it.endpointId }
  }

  /**
   * Returns a 1-indexed button label ("Button 1", "Button 2", ...) for a Switch (0x003B) endpoint
   * ordered by ascending endpointId among Switch endpoints on the node.
   */
  fun getSwitchButtonLabel(nodeId: Long, endpointId: Int): String {
    val node = nodes[nodeId]
    val switchEndpoints =
      node
        ?.endpoints
        ?.filter {
          it.serverClusters.contains(0x003BL) || it.deviceTypeId == MatterDeviceTypes.GENERIC_SWITCH
        }
        ?.map { it.endpointId }
        ?.sorted()
        ?: emptyList()
    val index = switchEndpoints.indexOf(endpointId)
    return if (index >= 0) {
      "Button ${index + 1}"
    } else {
      "Button $endpointId"
    }
  }

  fun removeUnsupportedCluster(
    context: Context?,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
  ): CommissionedNode? {
    var modified = false
    val updated =
      nodes.computeIfPresent(nodeId) { _, existing ->
        val updatedEndpoints =
          existing.endpoints.map { ep ->
            if (
              (ep.endpointId == endpointId || endpointId < 0) &&
                ep.serverClusters.contains(clusterId)
            ) {
              modified = true
              ep.copy(serverClusters = ep.serverClusters - clusterId)
            } else {
              ep
            }
          }
        if (modified) existing.copy(endpoints = updatedEndpoints) else existing
      }
        ?: return null
    if (!modified) return updated
    notifyNodeUpdated(updated)
    if (context != null) {
      saveFabricToPreferences(context)
    }
    return updated
  }

  fun syncWithDeviceIds(commissionedNodeIds: Set<Long>): Boolean {
    var changed = false
    for (existingNode in nodes.values.toList()) {
      if (
        existingNode.nodeId !in commissionedNodeIds ||
          existingNode.vendorName == "SyntheticVendor" ||
          existingNode.vendorName == "BenchVendor"
      ) {
        nodes.remove(existingNode.nodeId)
        try {
          CenterConnectionManager.onNodeRemoved(existingNode.nodeId)
        } catch (_: Throwable) {}
        notifyNodeUpdated(existingNode.copy(isOnline = false))
        changed = true
      }
    }
    for (nodeId in commissionedNodeIds) {
      if (nodeId > 0L && !nodes.containsKey(nodeId)) {
        autoRegisterCommissionedNode(nodeId)
        changed = true
      }
    }
    return changed
  }

  fun syncWithDeviceIdUtil(context: Context) {
    try {
      val commissionedHexIds =
        com.google.chip.chiptool.util.DeviceIdUtil.getCommissionedNodeId(context)
      val commissionedNodeIds =
        commissionedHexIds
          .mapNotNull { hexStr ->
            hexStr.trim().removePrefix("0x").removePrefix("0X").toULongOrNull(16)?.toLong()
          }
          .filter { it > 0L }
          .toSet()
      val changed = syncWithDeviceIds(commissionedNodeIds)
      if (changed) {
        saveFabricToPreferences(context)
      }
    } catch (e: Exception) {
      // Safe fallback when DeviceIdUtil preferences are unavailable in unit tests
    }
  }

  private fun decodeUIntListAttribute(attrState: AttributeState?): List<Long> {
    if (attrState == null) return emptyList()
    val rawVal = attrState.value
    if (rawVal is Collection<*>) {
      val parsed = rawVal.mapNotNull { (it as? Number)?.toLong() }
      if (parsed.isNotEmpty()) return parsed
    }
    val tlv = attrState.tlv ?: return emptyList()
    return try {
      val reader = TlvReader(tlv)
      val list = mutableListOf<Long>()
      reader.enterArray(AnonymousTag)
      while (!reader.isEndOfContainer()) {
        list.add(reader.getUInt(AnonymousTag).toLong())
      }
      reader.exitContainer()
      list
    } catch (_: Throwable) {
      try {
        val reader = TlvReader(tlv)
        val list = mutableListOf<Long>()
        reader.enterArray(AnonymousTag)
        while (!reader.isEndOfContainer()) {
          list.add(reader.getUShort(AnonymousTag).toLong())
        }
        reader.exitContainer()
        list
      } catch (_: Throwable) {
        emptyList()
      }
    }
  }

  private fun decodeDeviceTypeListAttribute(attrState: AttributeState?): List<Long> {
    if (attrState == null) return emptyList()
    val rawVal = attrState.value
    if (rawVal is Collection<*>) {
      val parsed =
        rawVal.mapNotNull { item ->
          when (item) {
            is ChipStructs.DescriptorClusterDeviceTypeStruct -> item.deviceType
            is Number -> item.toLong()
            else ->
              try {
                (item?.javaClass?.getField("deviceType")?.get(item) as? Number)?.toLong()
              } catch (_: Throwable) {
                null
              }
          }
        }
      if (parsed.isNotEmpty()) return parsed
    }
    val tlv = attrState.tlv ?: return emptyList()
    return try {
      val reader = TlvReader(tlv)
      val list = mutableListOf<Long>()
      reader.enterArray(AnonymousTag)
      while (!reader.isEndOfContainer()) {
        reader.enterStructure(AnonymousTag)
        val devType = reader.getUInt(ContextSpecificTag(0)).toLong()
        reader.getUShort(ContextSpecificTag(1))
        reader.exitContainer()
        list.add(devType)
      }
      reader.exitContainer()
      list
    } catch (_: Throwable) {
      emptyList()
    }
  }

  /**
   * Replaces a node's endpoints (serverClusters, deviceTypeId, deviceTypeName) using a live Matter
   * Descriptor cluster (0x001D) report while preserving user-edited fields (nodeLabel, roomName,
   * aliases). Returns true if valid Descriptor data was applied; returns false and keeps existing
   * data if the report contained no valid ServerList.
   */
  fun applyDescriptorReport(context: Context?, nodeId: Long, nodeState: NodeState): Boolean {
    val existing = nodes[nodeId] ?: return false
    var partsList: Set<Int>? = null
    val discoveredByEndpoint = mutableMapOf<Int, Pair<List<Long>, Set<Long>>>()

    for ((epId, epState) in nodeState.endpointStates) {
      val descCluster = epState.getClusterState(0x001DL) ?: continue
      if (epId == 0) {
        val parts =
          decodeUIntListAttribute(descCluster.getAttributeState(0x0003L)).map { it.toInt() }
        if (parts.isNotEmpty()) {
          partsList = (setOf(0) + parts)
        }
      }
      val serverList = decodeUIntListAttribute(descCluster.getAttributeState(0x0001L)).toSet()
      val deviceTypes = decodeDeviceTypeListAttribute(descCluster.getAttributeState(0x0000L))
      if (serverList.isNotEmpty() || deviceTypes.isNotEmpty()) {
        discoveredByEndpoint[epId] = deviceTypes to serverList
      }
    }

    val nonEmptyServerEndpoints = discoveredByEndpoint.filterValues { it.second.isNotEmpty() }
    if (nonEmptyServerEndpoints.isEmpty()) {
      return false
    }

    val targetEndpointIds =
      if (partsList != null) {
        (partsList intersect nonEmptyServerEndpoints.keys).ifEmpty { nonEmptyServerEndpoints.keys }
      } else {
        nonEmptyServerEndpoints.keys
      }

    val allTargetIds = (targetEndpointIds + 0).sorted()
    val replacedEndpoints =
      allTargetIds.map { epId -> buildDiscoveredEndpoint(epId, nonEmptyServerEndpoints[epId]) }

    val updatedNode =
      nodes.compute(nodeId) { _, current ->
        current?.copy(endpoints = replacedEndpoints, descriptorVerified = true, isOnline = true)
      }
        ?: return false
    try {
      val epSummary = replacedEndpoints.map(::formatEndpointSummary)
      android.util.Log.i(
        "DeviceCenter",
        "DescriptorDiscovery node=$nodeId verified=true endpoints=$epSummary",
      )
    } catch (_: Throwable) {}
    context?.let { saveFabricToPreferences(it) }
    notifyNodeUpdated(updatedNode)
    return true
  }

  private fun buildDiscoveredEndpoint(
    epId: Int,
    endpointData: Pair<List<Long>, Set<Long>>?,
  ): CommissionedEndpoint {
    val devTypes = endpointData?.first ?: emptyList()
    val reportedClusters = endpointData?.second ?: emptySet()
    val serverClusters =
      if (epId == 0) {
        (reportedClusters + STANDARD_ROOT_NODE_CLUSTERS).ifEmpty { STANDARD_ROOT_NODE_CLUSTERS }
      } else {
        reportedClusters
      }
    val primaryDevTypeId =
      if (devTypes.isNotEmpty()) {
        MatterDeviceTypes.selectPrimaryDeviceType(devTypes)
      } else if (epId == 0) {
        0x0016L
      } else {
        0L
      }
    val devTypeName = MatterDeviceTypes.nameForId(primaryDevTypeId)
    return CommissionedEndpoint(
      endpointId = epId,
      deviceTypeId = primaryDevTypeId,
      deviceTypeName = devTypeName,
      serverClusters = serverClusters,
    )
  }

  private fun formatEndpointSummary(ep: CommissionedEndpoint): String {
    val hexType = ep.deviceTypeId.toString(16)
    return "ep${ep.endpointId}:type=0x$hexType(${ep.deviceTypeName}):clusters=${ep.serverClusters}"
  }

  /**
   * Records a cluster (and optionally an attribute value) reported by a node. Returns null if the
   * node was removed concurrently, for example by a reset on the main thread while a report is
   * being processed.
   */
  fun recordDiscoveredEndpointCluster(
    context: Context?,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attrId: Long? = null,
    decodedValue: Any? = null,
  ): CommissionedNode? {
    var changed = false
    val updatedNode =
      nodes.computeIfPresent(nodeId) { _, existing ->
        val updatedEndpoints =
          if (existing.descriptorVerified) {
            existing.endpoints
          } else {
            mergeEndpointClusters(existing.endpoints, endpointId, setOf(clusterId))
          }
        val candidate =
          applyDiscoveredNodeMetadata(existing, updatedEndpoints, clusterId, attrId, decodedValue)
        changed = (candidate != existing)
        candidate
      }
        ?: return null

    if (changed) {
      context?.let { saveFabricToPreferences(it) }
      notifyNodeUpdated(updatedNode)
    }
    return updatedNode
  }

  private fun isGenericNodeLabel(nodeId: Long, label: String): Boolean {
    val cleanLabel = sanitizeMatterString(label)
    return cleanLabel.isEmpty() ||
      cleanLabel == "Device $nodeId" ||
      cleanLabel.equals("Device 0x${nodeId.toString(16)}", ignoreCase = true) ||
      cleanLabel == "Matter Device" ||
      cleanLabel.startsWith("Matter Device (")
  }

  private fun applyDiscoveredNodeMetadata(
    existing: CommissionedNode,
    updatedEndpoints: List<CommissionedEndpoint>,
    clusterId: Long,
    attrId: Long?,
    decodedValue: Any?,
  ): CommissionedNode {
    var newLabel = existing.nodeLabel
    var newVendor = existing.vendorName
    var newProduct = existing.productName
    var newBattery = existing.batteryPercent
    val isGenericLabel = isGenericNodeLabel(existing.nodeId, newLabel)

    val cleanBasicString =
      if (
        clusterId == MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION && decodedValue is String
      ) {
        sanitizeMatterString(decodedValue)
      } else {
        ""
      }

    if (cleanBasicString.isNotEmpty()) {
      when (attrId) {
        0x0001L -> newVendor = cleanBasicString
        0x0003L -> {
          newProduct = cleanBasicString
          if (isGenericLabel) newLabel = cleanBasicString
        }
        0x0005L -> if (isGenericLabel) newLabel = cleanBasicString
      }
    } else if (clusterId == 0x002FL && attrId == 0x000CL && decodedValue is Number) {
      newBattery = (decodedValue.toInt() / 2).coerceIn(0, 100)
    }

    newLabel = resolveDisplayLabel(existing.nodeId, newLabel, newVendor, newProduct)
    return existing.copy(
      nodeLabel = newLabel,
      vendorName = newVendor,
      productName = newProduct,
      batteryPercent = newBattery,
      endpoints = updatedEndpoints,
      isOnline = true,
    )
  }
}
