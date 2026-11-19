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
import android.util.Log
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import kotlin.coroutines.resume
import kotlinx.coroutines.suspendCancellableCoroutine

/** Result of dispatching a Matter Interaction Model operation. */
data class MatterDispatchResult(
  val isSuccess: Boolean,
  val responseCode: Long = 0,
  val message: String = "",
  val feedbackMessage: String = "",
  val decodedAttribute: DecodedAttributeValue? = null,
  val rawTlv: ByteArray? = null,
)

/**
 * Universal Matter Interaction Model Dispatcher. Unifies command invocations, attribute writes, and
 * attribute reads into a generic polymorphic execution engine on top of [ChipDeviceController],
 * handling device connection retrieval, TLV serialization/deserialization, and user-facing feedback
 * generation.
 */
open class UniversalMatterDispatcher(private val nodeRegistry: CommissionedNodeRegistry) {
  companion object {
    private const val TAG = "MatterDispatcher"
    private const val DEFAULT_IM_TIMEOUT_MS = 10_000
    private const val DEFAULT_TIMED_REQUEST_TIMEOUT_MS = 5_000
  }

  private fun offlineWithoutCache(nodeId: Long, targetDescription: String): MatterDispatchResult? {
    val link = CenterConnectionManager.linkState(nodeId)
    if (link.phase != LinkPhase.OFFLINE) return null
    val label = nodeRegistry.getNode(nodeId)?.nodeLabel ?: targetDescription
    return MatterDispatchResult(
      isSuccess = false,
      message = "$label is currently offline (link=${link.phase}, reason=${link.reason})",
      feedbackMessage = "$label is currently offline.",
    )
  }

  private fun isSyntheticNode(nodeId: Long): Boolean =
    nodeRegistry.getNode(nodeId)?.vendorName in
      setOf("SyntheticVendor", "BenchVendor", "VirtualVendor")

  private fun dispatchSyntheticCommand(
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    commandId: Long,
    params: Map<String, Any?>,
    targetDescription: String,
  ): MatterDispatchResult {
    val clusterMeta = MatterClusterMetaRegistry.getCluster(clusterId)
    val commandMeta = clusterMeta?.commands?.get(commandId)
    val commandName = commandMeta?.name ?: "Command 0x${commandId.toString(16)}"
    val now = System.currentTimeMillis()

    when (clusterId) {
      MatterClusterMetaRegistry.CLUSTER_ON_OFF -> {
        when (
          commandMeta?.name
            ?: if (commandId == 0x01L) "On" else if (commandId == 0x00L) "Off" else "Toggle"
        ) {
          "On" ->
            HomeStateCache.put(nodeId, endpointId, 0x0006L, 0x0000L, true, now, Source.UNICAST_READ)
          "Off" ->
            HomeStateCache.put(
              nodeId,
              endpointId,
              0x0006L,
              0x0000L,
              false,
              now,
              Source.UNICAST_READ,
            )
          "Toggle" -> {
            val cur =
              (HomeStateCache.get(nodeId, endpointId, 0x0006L, 0x0000L)?.value as? Boolean) ?: false
            HomeStateCache.put(nodeId, endpointId, 0x0006L, 0x0000L, !cur, now, Source.UNICAST_READ)
          }
        }
      }
      MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL -> {
        val level =
          (params["level"] as? Number)?.toInt()
            ?: (params["brightnessPercentage"] as? Number)?.let {
              MatterClusterMetaRegistry.percentageToLevel(it.toDouble()).toInt()
            }
              ?: 254
        HomeStateCache.put(nodeId, endpointId, 0x0008L, 0x0000L, level, now, Source.UNICAST_READ)
        if (level > 0) {
          HomeStateCache.put(nodeId, endpointId, 0x0006L, 0x0000L, true, now, Source.UNICAST_READ)
        }
      }
      MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL -> {
        if (params.containsKey("colorTemperatureMireds") || params.containsKey("kelvin")) {
          val mireds =
            (params["colorTemperatureMireds"] as? Number)?.toInt()
              ?: (params["kelvin"] as? Number)?.let {
                MatterClusterMetaRegistry.kelvinToMireds(it.toInt())
              }
                ?: 250
          HomeStateCache.put(nodeId, endpointId, 0x0300L, 0x0007L, mireds, now, Source.UNICAST_READ)
        }
        params["hue"]?.let {
          (it as? Number)?.toInt()?.let { h ->
            HomeStateCache.put(nodeId, endpointId, 0x0300L, 0x0000L, h, now, Source.UNICAST_READ)
          }
        }
        params["saturation"]?.let {
          (it as? Number)?.toInt()?.let { s ->
            HomeStateCache.put(nodeId, endpointId, 0x0300L, 0x0001L, s, now, Source.UNICAST_READ)
          }
        }
      }
      MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK -> {
        val lockState =
          if (
            commandMeta?.name == "UnlockDoor" ||
              commandId == 0x01L ||
              commandMeta?.name == "UnlockWithTimeout"
          )
            2
          else 1
        HomeStateCache.put(
          nodeId,
          endpointId,
          0x0101L,
          0x0000L,
          lockState,
          now,
          Source.UNICAST_READ,
        )
      }
      MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING -> {
        val pos =
          when (commandMeta?.name) {
            "UpOrOpen" -> 0
            "DownOrClose" -> 10000
            "GoToLiftPercentage" -> (params["liftPercent100thsValue"] as? Number)?.toInt() ?: 5000
            else -> 0
          }
        HomeStateCache.put(nodeId, endpointId, 0x0102L, 0x0008L, pos, now, Source.UNICAST_READ)
      }
      MatterClusterMetaRegistry.CLUSTER_THERMOSTAT -> {
        val cur =
          (HomeStateCache.get(nodeId, endpointId, 0x0201L, 0x0012L)?.value as? Number)?.toDouble()
            ?: 2100.0
        val delta = (params["amount"] as? Number)?.toDouble()?.times(10.0) ?: 100.0
        val newTemp = (cur + delta).toInt().coerceIn(1000, 3500)
        HomeStateCache.put(nodeId, endpointId, 0x0201L, 0x0012L, newTemp, now, Source.UNICAST_READ)
      }
    }

    val feedbackMsg = synthesizeCommandFeedback(clusterMeta, commandMeta, params, targetDescription)
    return MatterDispatchResult(
      isSuccess = true,
      message = "Executed $commandName on $targetDescription.",
      feedbackMessage = feedbackMsg,
    )
  }

  private fun dispatchSyntheticWrite(
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    value: Any?,
    targetDescription: String,
  ): MatterDispatchResult {
    val clusterMeta = MatterClusterMetaRegistry.getCluster(clusterId)
    val attrMeta = clusterMeta?.attributes?.get(attributeId)
    val attrName = attrMeta?.name ?: "Attribute 0x${attributeId.toString(16)}"
    HomeStateCache.put(
      nodeId,
      endpointId,
      clusterId,
      attributeId,
      value,
      System.currentTimeMillis(),
      Source.UNICAST_READ,
    )
    return MatterDispatchResult(
      isSuccess = true,
      message = "Wrote $value to $attrName on $targetDescription",
      feedbackMessage = "Updated $attrName to $value on $targetDescription",
    )
  }

  /** Appends "(as of N min ago)" when the cached value is not vouched for by a live link. */
  private fun withCacheAge(text: String, nodeId: Long, updatedAtMs: Long): String {
    val ageMs = CenterConnectionManager.cacheAgeMs(nodeId, updatedAtMs)
    if (ageMs < 60_000L) return text
    val note = "(as of ${ageMs / 60_000L} min ago)"
    return if (text.endsWith(".")) "${text.dropLast(1)} $note." else "$text $note"
  }

  /** Invokes an arbitrary Matter cluster command with dynamic TLV argument encoding. */
  suspend fun invokeCommand(
    context: Context,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    commandId: Long,
    params: Map<String, Any?> = emptyMap(),
    targetDescription: String = "Node $nodeId",
  ): MatterDispatchResult {
    offlineWithoutCache(nodeId, targetDescription)?.let {
      return it
    }

    if (isSyntheticNode(nodeId)) {
      return dispatchSyntheticCommand(
        nodeId,
        endpointId,
        clusterId,
        commandId,
        params,
        targetDescription,
      )
    }

    val clusterMeta = MatterClusterMetaRegistry.getCluster(clusterId)
    val commandMeta = clusterMeta?.commands?.get(commandId)
    val commandName = commandMeta?.name ?: "Command 0x${commandId.toString(16)}"
    val clusterName = clusterMeta?.name ?: "Cluster 0x${clusterId.toString(16)}"

    // 1. Dynamic TLV encoding
    val jsonString =
      try {
        if (commandMeta != null) {
          UniversalTlvEncoder.encodeCommandJson(commandMeta, params)
        } else {
          UniversalTlvEncoder.encodeCommandJson(
            MatterCommandMeta(commandId, commandName, ""),
            params,
          )
        }
      } catch (e: Exception) {
        Log.e(TAG, "JSON Encoding error for $clusterName::$commandName", e)
        return MatterDispatchResult(
          isSuccess = false,
          message = "JSON Encoding failed: ${e.message}",
          feedbackMessage = "Failed to encode parameters for $commandName on $targetDescription.",
        )
      }

    // 2. Get device pointer
    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(context, nodeId)
      } catch (e: kotlinx.coroutines.CancellationException) {
        throw e
      } catch (e: Exception) {
        Log.e(TAG, "Failed to get connected device pointer for node $nodeId", e)
        return MatterDispatchResult(
          isSuccess = false,
          message = "Failed to connect to node $nodeId: ${e.message}",
          feedbackMessage = "Unable to connect to $targetDescription.",
        )
      }

    val timedTimeout = if (commandMeta?.isTimed == true) DEFAULT_TIMED_REQUEST_TIMEOUT_MS else 0
    val invokeElement =
      InvokeElement.newInstance(endpointId, clusterId, commandId, null, jsonString)

    val controller = ChipClient.getDeviceController(context)

    return suspendCancellableCoroutine { continuation ->
      controller.invoke(
        object : InvokeCallback {
          override fun onError(ex: Exception?) {
            val errorMsg = ex?.message ?: "Unknown invoke error"
            Log.e(TAG, "Invoke command error on $targetDescription: $errorMsg", ex)
            if (
              errorMsg.contains("0x5c3", ignoreCase = true) ||
                errorMsg.contains("0xc3", ignoreCase = true) ||
                errorMsg.contains("UNSUPPORTED_CLUSTER", ignoreCase = true)
            ) {
              nodeRegistry.removeUnsupportedCluster(context, nodeId, endpointId, clusterId)
            }
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = false,
                  message = errorMsg,
                  feedbackMessage = "Error executing $commandName on $targetDescription: $errorMsg",
                )
              )
            }
          }

          override fun onResponse(responseElement: InvokeElement?, successCode: Long) {
            CenterConnectionManager.noteReachable(nodeId)
            val feedbackMsg =
              synthesizeCommandFeedback(clusterMeta, commandMeta, params, targetDescription)
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = true,
                  responseCode = successCode,
                  message = "Command $commandName succeeded on $targetDescription",
                  feedbackMessage = feedbackMsg,
                  rawTlv = responseElement?.tlvByteArray,
                )
              )
            }
          }
        },
        devicePointer,
        invokeElement,
        timedTimeout,
        DEFAULT_IM_TIMEOUT_MS,
      )
    }
  }

  /** Writes an attribute on a target node endpoint. */
  suspend fun writeAttribute(
    context: Context,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    value: Any?,
    targetDescription: String = "Node $nodeId",
  ): MatterDispatchResult {
    offlineWithoutCache(nodeId, targetDescription)?.let {
      return it
    }

    if (isSyntheticNode(nodeId)) {
      return dispatchSyntheticWrite(
        nodeId,
        endpointId,
        clusterId,
        attributeId,
        value,
        targetDescription,
      )
    }

    val clusterMeta = MatterClusterMetaRegistry.getCluster(clusterId)
    val attrMeta = clusterMeta?.attributes?.get(attributeId)
    val attrName = attrMeta?.name ?: "Attribute 0x${attributeId.toString(16)}"

    val jsonString =
      try {
        if (attrMeta != null) {
          UniversalTlvEncoder.encodeAttributeWriteJson(attrMeta, value)
        } else {
          UniversalTlvEncoder.encodeAttributeWriteJson(
            MatterAttributeMeta(attributeId, attrName, MatterDataType.INT64),
            value,
          )
        }
      } catch (e: Exception) {
        return MatterDispatchResult(
          isSuccess = false,
          message = "JSON write payload encoding error: ${e.message}",
          feedbackMessage = "Could not format value for $attrName.",
        )
      }

    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(context, nodeId)
      } catch (e: kotlinx.coroutines.CancellationException) {
        throw e
      } catch (e: Exception) {
        return MatterDispatchResult(
          isSuccess = false,
          message = "Connection failed: ${e.message}",
          feedbackMessage = "Unable to connect to $targetDescription.",
        )
      }

    val writeRequest =
      AttributeWriteRequest.newInstance(
        ChipPathId.forId(endpointId.toLong()),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
        jsonString,
      )

    val controller = ChipClient.getDeviceController(context)

    return suspendCancellableCoroutine { continuation ->
      controller.write(
        object : WriteAttributesCallback {
          override fun onError(attributePath: ChipAttributePath?, ex: Exception) {
            val err = ex.message ?: "Write attribute failed"
            if (
              err.contains("0x5c3", ignoreCase = true) ||
                err.contains("0xc3", ignoreCase = true) ||
                err.contains("UNSUPPORTED_CLUSTER", ignoreCase = true)
            ) {
              nodeRegistry.removeUnsupportedCluster(context, nodeId, endpointId, clusterId)
            }
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = false,
                  message = err,
                  feedbackMessage = "Failed to set $attrName on $targetDescription: $err",
                )
              )
            }
          }

          override fun onResponse(
            attributePath: ChipAttributePath,
            status: chip.devicecontroller.model.Status,
          ) {
            CenterConnectionManager.noteReachable(nodeId)
            if (status.status != chip.devicecontroller.model.Status.Code.Success) {
              if (continuation.isActive) {
                continuation.resume(
                  MatterDispatchResult(
                    isSuccess = false,
                    message = "Write rejected: $status",
                    feedbackMessage = "Failed to set $attrName on $targetDescription: $status",
                  )
                )
              }
              return
            }
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = true,
                  message = "Wrote $value to $attrName on $targetDescription",
                  feedbackMessage = "Updated $attrName to $value on $targetDescription",
                )
              )
            }
          }
        },
        devicePointer,
        listOf(writeRequest),
        0,
        DEFAULT_IM_TIMEOUT_MS,
      )
    }
  }

  /**
   * Reads an attribute from a target node endpoint and parses the TLV response into human-readable
   * text.
   */
  open suspend fun readAttribute(
    context: Context,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    targetDescription: String = "Node $nodeId",
    forceRefresh: Boolean = false,
  ): MatterDispatchResult {
    // Endpoint 0 is the Root Node endpoint and a valid target. Only a negative endpoint means
    // "any endpoint"; a pinned read never falls back to a value cached for another endpoint.
    val cached =
      if (endpointId >= 0) {
        HomeStateCache.get(nodeId, endpointId, clusterId, attributeId)
      } else {
        HomeStateCache.findAll(clusterId, attributeId).firstOrNull { it.nodeId == nodeId }
      }
    val clusterMeta = MatterClusterMetaRegistry.getCluster(clusterId)
    val attrMeta = clusterMeta?.attributes?.get(attributeId)
    val attrName = attrMeta?.name ?: "Attribute 0x${attributeId.toString(16)}"

    if (!forceRefresh && cached != null && cached.value != null) {
      val formatted =
        if (attrMeta != null) {
          UniversalTlvEncoder.formatDecodedValue(cached.value, attrMeta)
        } else {
          val unit = attrMeta?.unit ?: ""
          "${cached.value} $unit".trim()
        }
      val decoded =
        DecodedAttributeValue(
          rawValue = cached.value,
          formattedValue = formatted,
          unit = attrMeta?.unit,
          attributeMeta = attrMeta,
        )
      val feedbackMsg =
        withCacheAge(
          synthesizeStatusReport(targetDescription, attrMeta, decoded),
          nodeId,
          cached.updatedAtMs,
        )
      return MatterDispatchResult(
        isSuccess = true,
        message = "Read $attrName from cache: $formatted",
        feedbackMessage = feedbackMsg,
        decodedAttribute = decoded,
      )
    }

    if (isSyntheticNode(nodeId)) {
      val defaultVal: Any =
        when (clusterId) {
          MatterClusterMetaRegistry.CLUSTER_ON_OFF -> false
          MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL -> 254
          MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL -> 250
          MatterClusterMetaRegistry.CLUSTER_POWER_SOURCE -> 100
          MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK -> 1
          MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING -> 0
          MatterClusterMetaRegistry.CLUSTER_THERMOSTAT -> 2100
          MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT -> 2150
          MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT -> 4500
          MatterClusterMetaRegistry.CLUSTER_OCCUPANCY_SENSING -> 0
          MatterClusterMetaRegistry.CLUSTER_BOOLEAN_STATE -> true
          else -> 0L
        }
      HomeStateCache.put(
        nodeId,
        endpointId,
        clusterId,
        attributeId,
        defaultVal,
        System.currentTimeMillis(),
        Source.UNICAST_READ,
      )
      val formatted =
        if (attrMeta != null) {
          UniversalTlvEncoder.formatDecodedValue(defaultVal, attrMeta)
        } else {
          defaultVal.toString()
        }
      val decoded =
        DecodedAttributeValue(
          rawValue = defaultVal,
          formattedValue = formatted,
          unit = attrMeta?.unit,
          attributeMeta = attrMeta,
        )
      val feedbackMsg = synthesizeStatusReport(targetDescription, attrMeta, decoded)
      return MatterDispatchResult(
        isSuccess = true,
        message = "Read $attrName from synthetic node: $formatted",
        feedbackMessage = feedbackMsg,
        decodedAttribute = decoded,
      )
    }

    offlineWithoutCache(nodeId, targetDescription)?.let {
      return it
    }

    val devicePointer =
      try {
        kotlinx.coroutines.withTimeout(15000L) {
          ChipClient.getConnectedDevicePointer(context, nodeId)
        }
      } catch (e: kotlinx.coroutines.TimeoutCancellationException) {
        Log.e(TAG, "Connection timed out for node $nodeId", e)
        return MatterDispatchResult(
          isSuccess = false,
          message = "Connection failed or timed out: ${e.message}",
          feedbackMessage = "Unable to connect to $targetDescription.",
        )
      } catch (e: kotlinx.coroutines.CancellationException) {
        throw e
      } catch (e: Exception) {
        Log.e(TAG, "Connection failed or timed out for node $nodeId", e)
        return MatterDispatchResult(
          isSuccess = false,
          message = "Connection failed or timed out: ${e.message}",
          feedbackMessage = "Unable to connect to $targetDescription.",
        )
      }

    val attributePath =
      ChipAttributePath.newInstance(
        if (endpointId >= 0) ChipPathId.forId(endpointId.toLong()) else ChipPathId.forWildcard(),
        ChipPathId.forId(clusterId),
        ChipPathId.forId(attributeId),
      )
    val controller = ChipClient.getDeviceController(context)

    return suspendCancellableCoroutine { continuation ->
      controller.readPath(
        object : ReportCallback {
          override fun onError(
            attributePath: ChipAttributePath?,
            eventPath: ChipEventPath?,
            ex: Exception,
          ) {
            Log.e(TAG, "readPath onError for node $nodeId cluster 0x${clusterId.toString(16)}", ex)
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = false,
                  message = "Read error: ${ex.message}",
                  feedbackMessage = "Failed to read $attrName from $targetDescription.",
                )
              )
            }
          }

          override fun onReport(nodeState: NodeState) {
            val result =
              processReadAttributeReport(
                context = context,
                nodeState = nodeState,
                nodeId = nodeId,
                endpointId = endpointId,
                clusterId = clusterId,
                attributeId = attributeId,
                attrMeta = attrMeta,
                attrName = attrName,
                targetDescription = targetDescription,
              )
            if (continuation.isActive) {
              continuation.resume(result)
            }
          }

          override fun onDone() {
            if (continuation.isActive) {
              continuation.resume(
                MatterDispatchResult(
                  isSuccess = false,
                  message = "Read finished without attribute report",
                  feedbackMessage = "$attrName on $targetDescription is currently unavailable.",
                )
              )
            }
          }
        },
        devicePointer,
        listOf(attributePath),
        null,
        false,
        DEFAULT_IM_TIMEOUT_MS,
      )
    }
  }

  private fun processReadAttributeReport(
    context: Context?,
    nodeState: NodeState,
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    attrMeta: MatterAttributeMeta?,
    attrName: String,
    targetDescription: String,
  ): MatterDispatchResult {
    val matched = findAttributeInReport(nodeState, endpointId, clusterId, attributeId)
    if (matched == null) {
      return MatterDispatchResult(
        isSuccess = false,
        message = "No matching attribute state in report",
        feedbackMessage = "$attrName on $targetDescription is currently unavailable.",
      )
    }
    val (actualEpId, attrState) = matched
    val decoded = decodeReportedAttribute(attrState, attrMeta)
    if (decoded == null) {
      return MatterDispatchResult(
        isSuccess = false,
        message = "No TLV or value received in attribute report",
        feedbackMessage = "$attrName on $targetDescription is currently unavailable.",
      )
    }
    CenterConnectionManager.noteReachable(nodeId)
    if (decoded.rawValue != null) {
      HomeStateCache.put(
        nodeId,
        actualEpId,
        clusterId,
        attributeId,
        decoded.rawValue,
        System.currentTimeMillis(),
        Source.UNICAST_READ,
      )
      nodeRegistry.recordDiscoveredEndpointCluster(
        context,
        nodeId,
        actualEpId,
        clusterId,
        attributeId,
        decoded.rawValue,
      )
    }
    val updatedNode = nodeRegistry.getNode(nodeId)
    val effectiveTargetDesc =
      updatedNode?.let { "${it.nodeLabel} in ${it.roomName}" } ?: targetDescription
    val feedbackMsg = synthesizeStatusReport(effectiveTargetDesc, attrMeta, decoded)
    return MatterDispatchResult(
      isSuccess = true,
      message = "Read $attrName: ${decoded.formattedValue}",
      feedbackMessage = feedbackMsg,
      decodedAttribute = decoded,
      rawTlv = attrState.tlv,
    )
  }

  private fun findAttributeInReport(
    nodeState: NodeState,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
  ): Pair<Int, chip.devicecontroller.model.AttributeState>? {
    if (endpointId >= 0) {
      return nodeState
        .getEndpointState(endpointId)
        ?.getClusterState(clusterId)
        ?.getAttributeState(attributeId)
        ?.let { endpointId to it }
    }
    return nodeState.endpointStates.entries.firstNotNullOfOrNull { (epId, epState) ->
      epState.getClusterState(clusterId)?.getAttributeState(attributeId)?.let { epId to it }
    }
  }

  private fun decodeReportedAttribute(
    attrState: chip.devicecontroller.model.AttributeState,
    attrMeta: MatterAttributeMeta?,
  ): DecodedAttributeValue? {
    val rawVal = attrState.value
    if (rawVal != null) {
      val formatted =
        attrMeta?.let { UniversalTlvEncoder.formatDecodedValue(rawVal, it) } ?: rawVal.toString()
      return DecodedAttributeValue(
        rawValue = rawVal,
        formattedValue = formatted,
        unit = attrMeta?.unit,
        attributeMeta = attrMeta,
      )
    }
    val tlvBytes = attrState.tlv ?: return null
    val meta = attrMeta ?: return null
    if (tlvBytes.isEmpty()) return null
    return runCatching {
        UniversalTlvEncoder.decodeAttributeReport(meta, tlvBytes).takeIf { it.rawValue != null }
      }
      .getOrNull()
  }

  // --- Human-Readable Feedback Synthesis ---

  private fun synthesizeCommandFeedback(
    clusterMeta: MatterClusterMeta?,
    commandMeta: MatterCommandMeta?,
    params: Map<String, Any?>,
    targetDesc: String,
  ): String {
    if (clusterMeta == null || commandMeta == null) return "Command sent to $targetDesc."

    return when (clusterMeta.clusterId) {
      MatterClusterMetaRegistry.CLUSTER_ON_OFF -> {
        when (commandMeta.name) {
          "On" -> "Turned on $targetDesc."
          "Off" -> "Turned off $targetDesc."
          "Toggle" -> "Toggled $targetDesc."
          else -> "Executed ${commandMeta.name} on $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL -> {
        val pct =
          params["brightnessPercentage"]
            ?: params["level"]?.let {
              (it as? Number)?.let { l ->
                MatterClusterMetaRegistry.levelToPercentage(l.toLong().toUByte()).toInt()
              }
            }
        if (pct != null) "Set brightness of $targetDesc to $pct%."
        else "Adjusted level of $targetDesc."
      }
      MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL -> {
        if (params.containsKey("kelvin")) {
          "Set $targetDesc color temperature to ${params["kelvin"]}K."
        } else if (params.containsKey("colorTemperatureMireds")) {
          val mireds = (params["colorTemperatureMireds"] as? Number)?.toInt() ?: 250
          val kelvin = MatterClusterMetaRegistry.miredsToKelvin(mireds)
          "Set $targetDesc warmth to ${kelvin}K."
        } else {
          "Changed color of $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK -> {
        when (commandMeta.name) {
          "LockDoor" -> "Locked $targetDesc."
          "UnlockDoor" -> "Unlocked $targetDesc."
          "UnlockWithTimeout" -> "Unlocked $targetDesc for ${params["timeoutSeconds"]} seconds."
          else -> "Updated lock state on $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING -> {
        when (commandMeta.name) {
          "UpOrOpen" -> "Opened $targetDesc."
          "DownOrClose" -> "Closed $targetDesc."
          "StopMotion" -> "Stopped $targetDesc."
          "GoToLiftPercentage" -> {
            val p = (params["liftPercent100thsValue"] as? Number)?.toDouble()?.div(100.0) ?: 50.0
            "Set $targetDesc position to ${p.toInt()}%."
          }
          else -> "Adjusted $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_THERMOSTAT -> {
        if (commandMeta.name == "SetpointRaiseLower") {
          val amount = (params["amount"] as? Number)?.toDouble()?.div(10.0) ?: 1.0
          val direction = if (amount >= 0) "raised" else "lowered"
          "Temperature $direction by ${Math.abs(amount)}°C on $targetDesc."
        } else {
          "Updated thermostat settings on $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_MEDIA_PLAYBACK -> {
        when (commandMeta.name) {
          "Play" -> "Resumed playback on $targetDesc."
          "Pause" -> "Paused playback on $targetDesc."
          "Stop" -> "Stopped playback on $targetDesc."
          "Next" -> "Skipped to next track on $targetDesc."
          "Previous" -> "Playing previous track on $targetDesc."
          else -> "Executed ${commandMeta.name} on $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_AUDIO_OUTPUT -> {
        when (commandMeta.name) {
          "SelectOutput" -> "Selected audio output ${params["index"] ?: 0} on $targetDesc."
          "RenameOutput" -> "Renamed audio output ${params["index"] ?: 0} on $targetDesc."
          else -> "Updated audio output on $targetDesc."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_RVC_RUN_MODE -> {
        when ((params["newMode"] as? Number)?.toInt()) {
          1 -> "Started vacuuming with $targetDesc."
          3 -> "Sending $targetDesc back to the dock."
          else -> "Set $targetDesc run mode."
        }
      }
      MatterClusterMetaRegistry.CLUSTER_ENERGY_EVSE -> {
        when (commandMeta.name) {
          "EnableCharging" -> {
            val maxCurrent = (params["maximumChargeCurrent"] as? Number)?.toDouble()?.div(1000.0)
            if (maxCurrent != null) {
              "Enabled charging up to ${maxCurrent.toInt()}A on $targetDesc."
            } else {
              "Started charging EV on $targetDesc."
            }
          }
          "Disable" -> "Stopped charging EV on $targetDesc."
          "EnableDischarging" -> "Enabled EV discharging on $targetDesc."
          "StartDiagnostics" -> "Started EVSE diagnostics on $targetDesc."
          else -> "Updated EV charger on $targetDesc."
        }
      }
      else -> "Successfully executed ${commandMeta.name} on $targetDesc."
    }
  }

  private fun synthesizeStatusReport(
    targetDesc: String,
    attrMeta: MatterAttributeMeta?,
    decoded: DecodedAttributeValue,
  ): String {
    if (attrMeta == null) return "$targetDesc status: ${decoded.formattedValue}"

    return when (attrMeta.name) {
      "LocalTemperature",
      "MeasuredValue" -> {
        if (attrMeta.unit == "°C") {
          "The temperature at $targetDesc is ${decoded.formattedValue}."
        } else if (attrMeta.unit == "%") {
          "The relative humidity at $targetDesc is ${decoded.formattedValue}."
        } else {
          "$targetDesc ${attrMeta.name} is ${decoded.formattedValue}."
        }
      }
      "LockState" -> "$targetDesc is currently ${decoded.formattedValue}."
      "OnOff" -> "$targetDesc is currently turned ${decoded.formattedValue}."
      "CurrentLevel" -> "$targetDesc brightness is ${decoded.formattedValue}."
      "StateValue" -> "$targetDesc is currently ${decoded.formattedValue}."
      "Occupancy" -> {
        if (
          decoded.formattedValue.contains("1") ||
            decoded.formattedValue.equals("Occupied", ignoreCase = true)
        ) {
          "Motion is detected at $targetDesc."
        } else {
          "No motion detected at $targetDesc."
        }
      }
      "SmokeState" -> "Smoke sensor on $targetDesc is ${decoded.formattedValue}."
      "COState" -> "Carbon monoxide status on $targetDesc is ${decoded.formattedValue}."
      "BatPercentRemaining" -> {
        val cleanTarget = formatBatteryTargetLabel(targetDesc)
        "$cleanTarget battery is ${decoded.formattedValue}."
      }
      else -> "$targetDesc ${attrMeta.name} is ${decoded.formattedValue}."
    }
  }

  private fun formatBatteryTargetLabel(targetDesc: String): String {
    if (!targetDesc.contains(" in ")) return targetDesc
    val parts = targetDesc.split(" in ", limit = 2)
    val label = parts[0].trim()
    val room = parts[1].trim()
    val hasDistinctRoom =
      room.isNotEmpty() &&
        !room.equals("Unassigned", ignoreCase = true) &&
        !room.equals("Default Room", ignoreCase = true) &&
        !label.lowercase().contains(room.lowercase())
    return if (hasDistinctRoom) "$room $label" else label
  }
}
