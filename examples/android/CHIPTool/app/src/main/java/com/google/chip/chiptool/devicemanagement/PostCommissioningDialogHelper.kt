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

import android.app.Activity
import android.content.Context
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import com.google.chip.chiptool.databinding.DialogPostCommissioningSetupBinding
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

/**
 * Helper to display the Material Post-Commissioning Device Setup and In-Place Metadata Editor
 * Dialogs.
 */
object PostCommissioningDialogHelper {

  private const val TAG = "PostCommissioningDialog"

  val STANDARD_ROOMS =
    listOf(
      "Unassigned",
      "Living Room",
      "Kitchen",
      "Entryway",
      "Bedroom",
      "Bathroom",
      "Garage",
      "Hallway",
      "Office",
      "+ New Room",
    )

  /**
   * Automatically registers the newly commissioned node ID into the registry if not already
   * present.
   */
  fun autoRegisterNode(
    registry: CommissionedNodeRegistry,
    nodeId: Long,
    deviceInfo: CHIPDeviceInfo? = null,
    context: Context? = null,
  ): CommissionedNode {
    val defaultLabel =
      deviceInfo?.let { "Matter Device (VID:${it.vendorId} PID:${it.productId})" }
        ?: "Device 0x${nodeId.toString(16)}"
    val productName = deviceInfo?.let { "Matter Product ${it.productId}" } ?: "Matter Device"

    return registry
      .autoRegisterCommissionedNode(
        nodeId = nodeId,
        defaultLabel = defaultLabel,
        productName = productName,
        roomName = "Unassigned",
      )
      .also { context?.let { ctx -> registry.saveFabricToPreferences(ctx) } }
  }

  /**
   * Displays the Material Post-Commissioning Setup Dialog immediately after commissioning
   * completes.
   */
  fun showPostCommissioningDialog(
    activity: Activity,
    nodeId: Long,
    registry: CommissionedNodeRegistry,
    deviceInfo: CHIPDeviceInfo? = null,
    onComplete: ((CommissionedNode) -> Unit)? = null,
  ) {
    val node = autoRegisterNode(registry, nodeId, deviceInfo, activity)
    registry.saveFabricToPreferences(activity)
    val binding = DialogPostCommissioningSetupBinding.inflate(LayoutInflater.from(activity))

    // Pre-populate default device name
    val defaultName = node.nodeLabel.ifBlank { "Device 0x${nodeId.toString(16)}" }
    binding.deviceNameEditText.setText(defaultName)
    bindRoomDropdown(activity, binding, registry, "Unassigned")

    AlertDialog.Builder(activity)
      .setTitle("Device Commissioned Successfully")
      .setMessage("Node 0x${nodeId.toString(16)} ($nodeId) is now on your Matter fabric.")
      .setView(binding.root)
      .setCancelable(false)
      .setNeutralButton("Skip (Save as Unassigned)") { _, _ ->
        val finalName = resolveEnteredText(binding.deviceNameEditText.text, defaultName)
        val updated =
          saveMetadataAndPreferences(
            activity,
            registry,
            node,
            finalName,
            "Unassigned",
            node.aliases,
          )
        CenterConnectionManager.onNodeCommissioned(nodeId)
        Toast.makeText(activity, "Saved $finalName to Unassigned", Toast.LENGTH_SHORT).show()
        onComplete?.invoke(updated)
      }
      .setPositiveButton("Save & Finish") { _, _ ->
        val updated =
          saveFromDialogInputs(activity, binding, registry, node, defaultName, "Unassigned")
        CenterConnectionManager.onNodeCommissioned(nodeId)
        Toast.makeText(
            activity,
            "Configured ${updated.nodeLabel} in ${updated.roomName}",
            Toast.LENGTH_SHORT,
          )
          .show()
        onComplete?.invoke(updated)
      }
      .show()
  }

  /** Displays the In-Place Device Metadata Editor Dialog from the Device Center. */
  fun showEditDeviceDialog(
    context: Context,
    node: CommissionedNode,
    registry: CommissionedNodeRegistry,
    onSaved: ((CommissionedNode) -> Unit)? = null,
  ) {
    val binding = DialogPostCommissioningSetupBinding.inflate(LayoutInflater.from(context))

    binding.dialogSubtitleText.text =
      "Editing metadata for Node 0x${node.nodeId.toString(16)} (${node.productName})"
    binding.deviceNameEditText.setText(node.nodeLabel)
    bindRoomDropdown(context, binding, registry, node.roomName.ifEmpty { "Unassigned" })

    AlertDialog.Builder(context)
      .setTitle("Edit Name & Room")
      .setView(binding.root)
      .setNegativeButton("Cancel", null)
      .setPositiveButton("Save Changes") { _, _ ->
        val updated =
          saveFromDialogInputs(context, binding, registry, node, node.nodeLabel, node.roomName)
        Toast.makeText(context, "Updated ${updated.nodeLabel}", Toast.LENGTH_SHORT).show()
        onSaved?.invoke(updated)
      }
      .show()
  }

  private fun bindRoomDropdown(
    context: Context,
    binding: DialogPostCommissioningSetupBinding,
    registry: CommissionedNodeRegistry,
    initialRoom: String,
  ) {
    val existingRooms = registry.getAllRooms()
    val allRoomOptions =
      (STANDARD_ROOMS.dropLast(1) + existingRooms).distinct() + listOf("+ New Room")
    val roomAdapter =
      ArrayAdapter(context, android.R.layout.simple_dropdown_item_1line, allRoomOptions)
    binding.roomAutoComplete.setAdapter(roomAdapter)
    binding.roomAutoComplete.setText(initialRoom, false)

    binding.roomAutoComplete.setOnItemClickListener { _, _, position, _ ->
      val isNewRoom = roomAdapter.getItem(position) == "+ New Room"
      binding.customRoomLayout.visibility = if (isNewRoom) View.VISIBLE else View.GONE
      if (isNewRoom) binding.customRoomEditText.requestFocus()
    }
  }

  private fun resolveEnteredText(raw: CharSequence?, fallback: String): String =
    raw?.toString()?.trim()?.takeIf { it.isNotEmpty() } ?: fallback

  private fun saveFromDialogInputs(
    context: Context,
    binding: DialogPostCommissioningSetupBinding,
    registry: CommissionedNodeRegistry,
    node: CommissionedNode,
    fallbackName: String,
    fallbackRoom: String,
  ): CommissionedNode {
    val finalName = resolveEnteredText(binding.deviceNameEditText.text, fallbackName)
    val selectedRoom = resolveEnteredText(binding.roomAutoComplete.text, fallbackRoom)
    val finalRoom =
      if (selectedRoom == "+ New Room") {
        resolveEnteredText(binding.customRoomEditText.text, fallbackRoom.ifEmpty { "Unassigned" })
      } else {
        selectedRoom.ifEmpty { "Unassigned" }
      }
    val updated =
      saveMetadataAndPreferences(context, registry, node, finalName, finalRoom, node.aliases)
    if (binding.syncNodeLabelCheckbox.isChecked) {
      syncNodeLabelToDevice(context, node.nodeId, finalName, registry)
    }
    return updated
  }

  private fun saveMetadataAndPreferences(
    context: Context,
    registry: CommissionedNodeRegistry,
    node: CommissionedNode,
    finalName: String,
    finalRoom: String,
    aliasesList: List<String>,
  ): CommissionedNode {
    val updated =
      registry.updateNodeMetadata(
        nodeId = node.nodeId,
        nodeLabel = finalName,
        roomName = finalRoom,
        aliases = aliasesList,
      )
        ?: node
    registry.saveFabricToPreferences(context)
    return updated
  }

  private fun syncNodeLabelToDevice(
    context: Context,
    nodeId: Long,
    nodeLabel: String,
    registry: CommissionedNodeRegistry,
  ) {
    CoroutineScope(Dispatchers.IO).launch {
      try {
        val dispatcher = UniversalMatterDispatcher(registry)
        val result =
          dispatcher.writeAttribute(
            context = context,
            nodeId = nodeId,
            endpointId = 0,
            clusterId = MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION,
            attributeId = 0x0005L, // NodeLabel
            value = nodeLabel,
            targetDescription = "Node $nodeId (Basic Information)",
          )
        Log.d(TAG, "Sync NodeLabel result: ${result.message}")
      } catch (e: Exception) {
        Log.w(TAG, "Optional NodeLabel write to device failed: ${e.message}")
      }
    }
  }
}
