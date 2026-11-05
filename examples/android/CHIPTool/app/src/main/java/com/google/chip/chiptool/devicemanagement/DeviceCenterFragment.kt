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

import android.content.res.ColorStateList
import android.graphics.Color
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.chip.Chip
import com.google.android.material.slider.Slider
import com.google.android.material.tabs.TabLayout
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.DialogEndpointInspectorBinding
import com.google.chip.chiptool.databinding.FragmentDeviceCenterBinding
import com.google.chip.chiptool.databinding.ItemClusterAttributeEditorBinding
import com.google.chip.chiptool.databinding.ItemDeviceCardBinding
import com.google.chip.chiptool.databinding.ItemRoomSectionBinding
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch

/** Modern Room-Centric Matter Device & Dynamic Endpoint Management Center Fragment. */
class DeviceCenterFragment : Fragment() {

  private var _binding: FragmentDeviceCenterBinding? = null
  private val binding
    get() = checkNotNull(_binding) { "FragmentDeviceCenterBinding accessed outside view lifecycle" }

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var synchronizer: MatterStateSynchronizer
  private lateinit var dispatcher: UniversalMatterDispatcher
  private lateinit var viewModel: DeviceManagementViewModel

  private lateinit var roomHierarchyAdapter: RoomHierarchyAdapter
  private var inspectorDialog: AlertDialog? = null

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    registry = CenterConnectionManager.registry(requireContext())
    synchronizer = MatterStateSynchronizer.getInstance(registry)
    dispatcher = UniversalMatterDispatcher(registry)
    viewModel =
      ViewModelProvider(this, DeviceManagementViewModel.Factory(requireContext()))[
        DeviceManagementViewModel::class.java]
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    _binding = FragmentDeviceCenterBinding.inflate(inflater, container, false)
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    setupSummaryStrip()
    setupSearchAndFilters()
    setupEmptyStateActions()
    setupRecyclerView()
    viewModel.refreshFabric()
  }

  override fun onStart() {
    super.onStart()
    CenterConnectionManager.acquire(requireContext(), CENTER_LEASE_OWNER)
  }

  override fun onStop() {
    // Leaving the Center (navigation or app background) tears down every subscription. A
    // configuration change (rotation) keeps them: the recreated Center re-acquires the same lease.
    if (activity?.isChangingConfigurations != true) {
      CenterConnectionManager.release(CENTER_LEASE_OWNER)
    }
    super.onStop()
  }

  private fun setupSummaryStrip() {
    binding.connectAllButton.setOnClickListener { viewModel.connectAll() }

    binding.cancelConnectAllButton.setOnClickListener { viewModel.cancelConnectAll() }

    viewLifecycleOwner.lifecycleScope.launch {
      viewModel.summaryStripText.collectLatest { text ->
        if (!isAdded || _binding == null) return@collectLatest
        binding.summaryStripText.text = text
      }
    }

    viewLifecycleOwner.lifecycleScope.launch {
      viewModel.connectAllProgress.collectLatest { progress ->
        if (!isAdded || _binding == null) return@collectLatest
        if (progress?.active == true) {
          binding.connectAllButton.visibility = View.GONE
          binding.cancelConnectAllButton.visibility = View.VISIBLE
          binding.connectAllProgressContainer.visibility = View.VISIBLE
          val cur = minOf(progress.settled + 1, progress.total)
          binding.connectAllProgressText.text = "Connecting $cur of ${progress.total}…"
          val pct =
            if (progress.total > 0) ((progress.settled.toFloat() / progress.total) * 100).toInt()
            else 0
          binding.connectAllProgressBar.progress = pct
        } else {
          binding.connectAllButton.visibility = View.VISIBLE
          binding.cancelConnectAllButton.visibility = View.GONE
          binding.connectAllProgressContainer.visibility = View.GONE
        }
      }
    }
  }

  private fun setupSearchAndFilters() {
    binding.searchEditText.addTextChangedListener(
      object : TextWatcher {
        override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}

        override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
          viewModel.setSearchQuery(s?.toString() ?: "")
        }

        override fun afterTextChanged(s: Editable?) {}
      }
    )

    // Setup room filter chips dynamically
    viewLifecycleOwner.lifecycleScope.launch {
      viewModel.availableRooms.collectLatest { rooms ->
        if (!isAdded || _binding == null) return@collectLatest
        binding.roomChipGroup.removeAllViews()

        val allChip =
          Chip(requireContext()).apply {
            text = "All Rooms"
            isCheckable = true
            isChecked = (viewModel.selectedRoomFilter.value == null)
            setOnClickListener { viewModel.setRoomFilter(null) }
          }
        binding.roomChipGroup.addView(allChip)

        for (room in rooms) {
          val chip =
            Chip(requireContext()).apply {
              text = room
              isCheckable = true
              isChecked = (viewModel.selectedRoomFilter.value == room)
              setOnClickListener { viewModel.setRoomFilter(room) }
            }
          binding.roomChipGroup.addView(chip)
        }
      }
    }
  }

  private fun setupEmptyStateActions() {
    binding.commissionDeviceBtn.setOnClickListener {
      parentFragmentManager
        .beginTransaction()
        .replace(
          R.id.nav_host_fragment,
          BarcodeFragment.newInstance(),
          BarcodeFragment::class.java.simpleName,
        )
        .addToBackStack(null)
        .commit()
    }
  }

  private fun withCardViewModel(
    card: DeviceCardState,
    action: (android.content.Context, DeviceCardViewModel) -> Unit,
  ) {
    val ctx = context ?: return
    val node = registry.getNode(card.nodeId) ?: return
    action(ctx, viewModel.getOrCreateCardViewModel(node))
  }

  private fun setupRecyclerView() {
    val cardActions =
      object : DeviceCardActions {
        override fun togglePower(card: DeviceCardState) {
          withCardViewModel(card) { ctx, vm -> vm.togglePower(ctx) }
        }

        override fun toggleLock(card: DeviceCardState) {
          withCardViewModel(card) { ctx, vm -> vm.toggleLock(ctx) }
        }

        override fun changeBrightness(card: DeviceCardState, percent: Int) {
          withCardViewModel(card) { ctx, vm -> vm.setBrightness(ctx, percent) }
        }

        override fun editAliases(card: DeviceCardState) {
          registry.getNode(card.nodeId)?.let(::showEditRoomAndAliasesDialog)
        }

        override fun inspectEndpoints(card: DeviceCardState) {
          showEndpointInspector(card.nodeId)
        }

        override fun connectDevice(card: DeviceCardState) {
          viewModel.connectDevice(card.nodeId)
        }
      }
    roomHierarchyAdapter = RoomHierarchyAdapter(cardActions)

    binding.roomHierarchyRecyclerView.layoutManager = LinearLayoutManager(requireContext())
    (binding.roomHierarchyRecyclerView.itemAnimator
        as? androidx.recyclerview.widget.SimpleItemAnimator)
      ?.supportsChangeAnimations = false
    binding.roomHierarchyRecyclerView.adapter = roomHierarchyAdapter

    viewLifecycleOwner.lifecycleScope.launch {
      viewModel.filteredRoomHierarchy.collectLatest { roomItems ->
        if (!isAdded || _binding == null) return@collectLatest
        if (roomItems.isEmpty()) {
          binding.emptyStateLayout.visibility = View.VISIBLE
          binding.roomHierarchyRecyclerView.visibility = View.GONE
        } else {
          binding.emptyStateLayout.visibility = View.GONE
          binding.roomHierarchyRecyclerView.visibility = View.VISIBLE
          roomHierarchyAdapter.submitList(roomItems)
        }
      }
    }
  }

  fun showEndpointInspector(nodeId: Long) {
    inspectorDialog?.dismiss()
    registry.upgradeEp0Clusters(nodeId)
    val inspectorVm = EndpointInspectorViewModel(nodeId, registry, synchronizer, dispatcher)
    val dialogBinding = DialogEndpointInspectorBinding.inflate(layoutInflater)
    val inspectorJobs = mutableListOf<Job>()

    val dialog = AlertDialog.Builder(requireContext()).setView(dialogBinding.root).create()

    dialog.setOnDismissListener {
      inspectorJobs.forEach { it.cancel() }
      inspectorVm.onCleared()
      if (inspectorDialog === dialog) {
        inspectorDialog = null
      }
    }
    inspectorDialog = dialog

    dialogBinding.inspectorToolbar.setNavigationOnClickListener { dialog.dismiss() }

    val clusterAdapter =
      ClusterAttributeAdapter(
        toggleSwitch = { epId, clusterId, attrId, isChecked ->
          inspectorVm.inPlaceWriteAttribute(requireContext(), epId, clusterId, attrId, isChecked)
        },
        changeSlider = { epId, clusterId, attrId, value ->
          inspectorVm.inPlaceWriteAttribute(requireContext(), epId, clusterId, attrId, value)
        },
        readAttribute = { epId, clusterId, attrId ->
          inspectorVm.readAttribute(requireContext(), epId, clusterId, attrId)
        },
        editAttribute = { epId, clusterId, attrId, attrName, currentVal ->
          showWriteAttributeDialog(inspectorVm, epId, clusterId, attrId, attrName, currentVal)
        },
        invokeCommand = { epId, clusterId, cmd ->
          if (cmd.requestFields.isNotEmpty()) {
            showInvokeCommandDialog(inspectorVm, epId, clusterId, cmd)
          } else {
            inspectorVm.invokeCommand(requireContext(), epId, clusterId, cmd.commandId, emptyMap())
          }
        },
      )

    dialogBinding.clustersRecyclerView.layoutManager = LinearLayoutManager(requireContext())
    dialogBinding.clustersRecyclerView.adapter = clusterAdapter

    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        inspectorVm.node.collectLatest { node ->
          val current = node ?: return@collectLatest
          val nodeHex = current.nodeId.toString(16)
          dialogBinding.inspectorDeviceLabel.text = current.nodeLabel
          dialogBinding.inspectorDeviceMeta.text =
            "Node 0x$nodeHex • ${current.vendorName} • Room: ${current.roomName}"
        }
      }
    )

    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        inspectorVm.operationStatus.collectLatest { status ->
          if (status.isNullOrEmpty()) return@collectLatest
          dialogBinding.inspectorStatusBanner.text = status
        }
      }
    )

    var renderedTabKeys: List<Pair<Int, String>> = emptyList()
    var suppressTabEvents = false

    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        inspectorVm.endpoints.collectLatest { endpoints ->
          val newKeys = endpoints.map { it.endpointId to it.deviceTypeName }
          if (newKeys != renderedTabKeys) {
            renderedTabKeys = newKeys
            suppressTabEvents = true
            try {
              populateEndpointTabs(
                dialogBinding.endpointTabLayout,
                endpoints,
                inspectorVm.currentEndpoint.value?.endpointId,
              )
            } finally {
              suppressTabEvents = false
            }
          }
        }
      }
    )

    dialogBinding.endpointTabLayout.addOnTabSelectedListener(
      object : TabLayout.OnTabSelectedListener {
        override fun onTabSelected(tab: TabLayout.Tab?) {
          if (!suppressTabEvents) {
            tab?.let { inspectorVm.selectEndpoint(it.position) }
          }
        }

        override fun onTabUnselected(tab: TabLayout.Tab?) {}

        override fun onTabReselected(tab: TabLayout.Tab?) {}
      }
    )

    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        inspectorVm.currentEndpoint.collectLatest { ep ->
          val currentEp = ep ?: return@collectLatest
          val hexType = currentEp.deviceTypeId.toString(16)
          dialogBinding.endpointTitleText.text =
            "Endpoint ${currentEp.endpointId}: ${currentEp.deviceTypeName} (0x$hexType)"
          dialogBinding.clusterCountText.text = "${currentEp.serverClusters.size} Server Clusters"
          clusterAdapter.submitClusters(currentEp.endpointId, currentEp.serverClusters)
        }
      }
    )

    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        inspectorVm.recentEvents.collectLatest { events ->
          renderRecentEvents(dialogBinding, events)
        }
      }
    )

    // Trigger background Descriptor ServerList read on ep0 if connected, refreshing inspectorVm
    inspectorJobs.add(
      viewLifecycleOwner.lifecycleScope.launch {
        refreshRootDescriptorClusters(nodeId, inspectorVm)
      }
    )

    dialog.show()
  }

  private fun renderRecentEvents(
    dialogBinding: DialogEndpointInspectorBinding,
    events: List<CachedEvent>,
  ) {
    val ctx = context ?: return
    dialogBinding.recentEventsContainer.removeAllViews()
    if (events.isEmpty()) {
      val emptyView =
        TextView(ctx).apply {
          text = "No recent events"
          textSize = 12f
          setTextColor(ContextCompat.getColor(ctx, R.color.colorOnSurfaceVariant))
        }
      dialogBinding.recentEventsContainer.addView(emptyView)
    } else {
      val timeFormat = java.text.SimpleDateFormat("HH:mm:ss.SSS", java.util.Locale.US)
      for (event in events) {
        dialogBinding.recentEventsContainer.addView(createRecentEventView(ctx, event, timeFormat))
      }
    }
    // The list is newest first; keep the newest event visible as rows are added on top.
    dialogBinding.recentEventsScroll.post { dialogBinding.recentEventsScroll.scrollTo(0, 0) }
  }

  private fun createRecentEventView(
    ctx: android.content.Context,
    event: CachedEvent,
    timeFormat: java.text.SimpleDateFormat,
  ): TextView {
    val clusterName =
      MatterClusterMetaRegistry.getCluster(event.clusterId)?.name
        ?: "0x${event.clusterId.toString(16)}"
    val timeStr = timeFormat.format(java.util.Date(event.timestampMs))
    val rowText =
      if (event.isCheckIn) {
        "[$timeStr] $clusterName · ${event.eventName} · counter ${event.checkInCounter}"
      } else {
        "[$timeStr] EP ${event.endpointId} · $clusterName · " +
          "${event.eventName} (#${event.eventNumber})"
      }
    return TextView(ctx).apply {
      text = rowText
      textSize = 12f
      setTextColor(ContextCompat.getColor(ctx, R.color.colorOnSurface))
    }
  }

  private fun populateEndpointTabs(
    tabLayout: TabLayout,
    endpoints: List<EndpointDescriptor>,
    selectedEndpointId: Int? = null,
  ) {
    tabLayout.removeAllTabs()
    var selectedTab: TabLayout.Tab? = null
    for (ep in endpoints) {
      val tab = tabLayout.newTab().setText("EP ${ep.endpointId}: ${ep.deviceTypeName}")
      tabLayout.addTab(tab, false)
      if (ep.endpointId == selectedEndpointId) {
        selectedTab = tab
      }
    }
    (selectedTab ?: tabLayout.getTabAt(0))?.select()
  }

  private fun showWriteAttributeDialog(
    inspectorVm: EndpointInspectorViewModel,
    epId: Int,
    clusterId: Long,
    attrId: Long,
    attrName: String,
    currentVal: Any?,
  ) {
    val input =
      EditText(requireContext()).apply {
        hint = "New value"
        setText(currentVal?.toString() ?: "")
      }
    AlertDialog.Builder(requireContext())
      .setTitle("Write $attrName")
      .setView(input)
      .setPositiveButton("Write") { _, _ ->
        val text = input.text.toString().trim()
        if (text.isNotEmpty()) {
          inspectorVm.inPlaceWriteAttribute(requireContext(), epId, clusterId, attrId, text)
        }
      }
      .setNegativeButton("Cancel", null)
      .show()
  }

  private fun showInvokeCommandDialog(
    inspectorVm: EndpointInspectorViewModel,
    epId: Int,
    clusterId: Long,
    cmd: MatterCommandMeta,
  ) {
    val layout =
      LinearLayout(requireContext()).apply {
        orientation = LinearLayout.VERTICAL
        setPadding(32, 16, 32, 16)
      }
    val inputs = mutableMapOf<String, EditText>()
    for (field in cmd.requestFields) {
      val input = EditText(requireContext()).apply { hint = "${field.name} (${field.type})" }
      inputs[field.name] = input
      layout.addView(input)
    }

    AlertDialog.Builder(requireContext())
      .setTitle("Invoke ${cmd.name}")
      .setView(layout)
      .setPositiveButton("Invoke") { _, _ ->
        val params = inputs.mapValues { it.value.text.toString() }
        inspectorVm.invokeCommand(requireContext(), epId, clusterId, cmd.commandId, params)
      }
      .setNegativeButton("Cancel", null)
      .show()
  }

  private suspend fun refreshRootDescriptorClusters(
    nodeId: Long,
    inspectorVm: EndpointInspectorViewModel,
  ) {
    val appContext = context?.applicationContext ?: return
    try {
      val result =
        dispatcher.readAttribute(
          context = appContext,
          nodeId = nodeId,
          endpointId = 0,
          clusterId = MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR,
          attributeId = 0x0001L, // ServerList
          targetDescription = "Root Node Descriptor",
        )
      val rawList = result.decodedAttribute?.value as? List<*> ?: return
      if (!result.isSuccess) return
      val serverClusters = rawList.mapNotNull { (it as? Number)?.toLong() }.toSet()
      if (serverClusters.isNotEmpty()) {
        registry.recordDiscoveredEndpointClusterList(appContext, nodeId, 0, serverClusters)
        inspectorVm.reloadEndpoints()
      }
    } catch (e: kotlinx.coroutines.CancellationException) {
      throw e
    } catch (_: Exception) {}
  }

  fun showEditRoomAndAliasesDialog(node: CommissionedNode) {
    PostCommissioningDialogHelper.showEditDeviceDialog(
      context = requireContext(),
      node = node,
      registry = registry,
      onSaved = { viewModel.refreshFabric() },
    )
  }

  override fun onDestroyView() {
    super.onDestroyView()
    inspectorDialog?.dismiss()
    inspectorDialog = null
    _binding = null
  }

  companion object {
    private const val CENTER_LEASE_OWNER = "device_center"

    @JvmStatic fun newInstance() = DeviceCenterFragment()
  }
}

interface DeviceCardActions {
  fun togglePower(card: DeviceCardState)

  fun toggleLock(card: DeviceCardState)

  fun changeBrightness(card: DeviceCardState, percent: Int)

  fun editAliases(card: DeviceCardState)

  fun inspectEndpoints(card: DeviceCardState)

  fun connectDevice(card: DeviceCardState)
}

class RoomHierarchyAdapter(private val cardActions: DeviceCardActions) :
  RecyclerView.Adapter<RoomHierarchyAdapter.RoomViewHolder>() {

  private val items = mutableListOf<RoomHierarchyItem>()

  fun submitList(newItems: List<RoomHierarchyItem>) {
    dispatchDiff(items, newItems, this) { a, b -> a.roomName == b.roomName }
  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RoomViewHolder {
    val binding = ItemRoomSectionBinding.inflate(LayoutInflater.from(parent.context), parent, false)
    return RoomViewHolder(binding)
  }

  override fun onBindViewHolder(holder: RoomViewHolder, position: Int) {
    holder.bind(items[position])
  }

  override fun getItemCount(): Int = items.size

  inner class RoomViewHolder(private val binding: ItemRoomSectionBinding) :
    RecyclerView.ViewHolder(binding.root) {

    private val deviceAdapter = DeviceCardAdapter(cardActions)

    init {
      binding.roomDevicesRecyclerView.layoutManager = LinearLayoutManager(binding.root.context)
      (binding.roomDevicesRecyclerView.itemAnimator
          as? androidx.recyclerview.widget.SimpleItemAnimator)
        ?.supportsChangeAnimations = false
      binding.roomDevicesRecyclerView.adapter = deviceAdapter
    }

    fun bind(item: RoomHierarchyItem) {
      binding.roomNameText.text = item.roomName
      binding.roomCountBadge.text = "${item.totalCount} devices • ${item.activeCount} online"
      deviceAdapter.submitList(item.deviceCards)
    }
  }
}

private fun <T> dispatchDiff(
  items: MutableList<T>,
  newItems: List<T>,
  adapter: RecyclerView.Adapter<*>,
  sameIdentity: (T, T) -> Boolean,
) {
  val oldItems = items.toList()
  val diffResult =
    DiffUtil.calculateDiff(
      object : DiffUtil.Callback() {
        override fun getOldListSize(): Int = oldItems.size

        override fun getNewListSize(): Int = newItems.size

        override fun areItemsTheSame(oldPos: Int, newPos: Int): Boolean =
          sameIdentity(oldItems[oldPos], newItems[newPos])

        override fun areContentsTheSame(oldPos: Int, newPos: Int): Boolean =
          oldItems[oldPos] == newItems[newPos]
      }
    )
  items.clear()
  items.addAll(newItems)
  diffResult.dispatchUpdatesTo(adapter)
}

class DeviceCardAdapter(private val cardActions: DeviceCardActions) :
  RecyclerView.Adapter<DeviceCardAdapter.DeviceViewHolder>() {

  private val items = mutableListOf<DeviceCardState>()

  fun submitList(newItems: List<DeviceCardState>) {
    dispatchDiff(items, newItems, this) { a, b -> a.nodeId == b.nodeId }
  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): DeviceViewHolder {
    val binding = ItemDeviceCardBinding.inflate(LayoutInflater.from(parent.context), parent, false)
    return DeviceViewHolder(binding)
  }

  override fun onBindViewHolder(holder: DeviceViewHolder, position: Int) {
    holder.bind(items[position])
  }

  override fun getItemCount(): Int = items.size

  inner class DeviceViewHolder(private val binding: ItemDeviceCardBinding) :
    RecyclerView.ViewHolder(binding.root) {

    private var boundCard: DeviceCardState? = null

    init {
      binding.quickLevelSlider.addOnChangeListener { _, value, fromUser ->
        if (!fromUser) return@addOnChangeListener
        binding.sliderValueText.text = "${value.toInt()}%"
      }
      binding.quickLevelSlider.addOnSliderTouchListener(
        object : Slider.OnSliderTouchListener {
          override fun onStartTrackingTouch(slider: Slider) {}

          override fun onStopTrackingTouch(slider: Slider) {
            boundCard?.let { cardActions.changeBrightness(it, slider.value.toInt()) }
          }
        }
      )
    }

    fun bind(card: DeviceCardState) {
      boundCard = card
      val context = binding.root.context
      binding.deviceLabelText.text = card.nodeLabel
      binding.deviceSubtitleText.text =
        "${card.vendorName} • ${card.productName} • Node 0x${card.nodeId.toString(16)}"

      val density = context.resources.displayMetrics.density
      binding.deviceCardView.strokeWidth = (1 * density).toInt()
      binding.deviceCardView.strokeColor =
        ContextCompat.getColor(context, R.color.colorOutlineVariant)

      // 5-State Center Link Status Badge
      binding.badgeCenterStatus.visibility = View.VISIBLE
      binding.badgeCenterStatus.text = card.linkStatusText
      binding.badgeCenterStatus.setChipIconResource(card.linkStatusIconRes)
      val linkColor =
        try {
          Color.parseColor(card.linkStatusColorHex)
        } catch (e: Exception) {
          ContextCompat.getColor(context, R.color.centerLive)
        }
      binding.badgeCenterStatus.setTextColor(linkColor)
      binding.badgeCenterStatus.chipIconTint = ColorStateList.valueOf(linkColor)

      // Per-device Connect Button (visible in OFFLINE / NOT_CONNECTED states)
      if (card.showConnectButton) {
        binding.connectDeviceButton.visibility = View.VISIBLE
        binding.connectDeviceButton.setOnClickListener { cardActions.connectDevice(card) }
      } else {
        binding.connectDeviceButton.visibility = View.GONE
      }

      // Legacy Badges
      val isUnassigned =
        card.roomName.equals("Unassigned", ignoreCase = true) ||
          card.badges.any { it.type == DeviceBadgeType.UNASSIGNED }
      binding.badgeUnassigned.visibility = if (isUnassigned) View.VISIBLE else View.GONE

      if (card.batteryPercent != null) {
        binding.badgeBattery.visibility = View.VISIBLE
        binding.badgeBattery.text = "Battery ${card.batteryPercent}%"
      } else {
        binding.badgeBattery.visibility = View.GONE
      }

      val stateBadge =
        card.badges.firstOrNull {
          it.type == DeviceBadgeType.STATE_ON ||
            it.type == DeviceBadgeType.STATE_OFF ||
            it.type == DeviceBadgeType.STATE_LOCKED ||
            it.type == DeviceBadgeType.STATE_UNLOCKED ||
            it.type == DeviceBadgeType.TEMPERATURE
        }
      if (stateBadge != null) {
        binding.badgeState.visibility = View.VISIBLE
        binding.badgeState.text = stateBadge.label
      } else {
        binding.badgeState.visibility = View.GONE
      }

      // Sensor Readings & Switch Events
      if (!card.sensorReadingText.isNullOrBlank()) {
        binding.sensorReadingsText.visibility = View.VISIBLE
        binding.sensorReadingsText.text = card.sensorReadingText
      } else {
        binding.sensorReadingsText.visibility = View.GONE
      }

      if (!card.switchEventText.isNullOrBlank()) {
        binding.switchEventText.visibility = View.VISIBLE
        binding.switchEventText.text = card.switchEventText
      } else {
        binding.switchEventText.visibility = View.GONE
      }

      // Aliases
      if (card.aliases.isNotEmpty()) {
        binding.aliasesText.visibility = View.VISIBLE
        binding.aliasesText.text = "Aliases: " + card.aliases.joinToString(", ")
      } else {
        binding.aliasesText.visibility = View.GONE
      }

      // Quick Controls - always remain enabled regardless of link state
      binding.quickToggleSwitch.isEnabled = true
      binding.quickLockButton.isEnabled = true
      binding.quickLevelSlider.isEnabled = true

      when (val qc = card.quickControl) {
        is QuickControlAction.ToggleSwitch -> {
          binding.quickToggleSwitch.visibility = View.VISIBLE
          binding.quickLockButton.visibility = View.GONE
          binding.sliderContainer.visibility = View.GONE
          binding.quickToggleSwitch.setOnCheckedChangeListener(null)
          binding.quickToggleSwitch.isChecked = qc.isOn
          binding.quickToggleSwitch.setOnCheckedChangeListener { _, _ ->
            cardActions.togglePower(card)
          }
        }
        is QuickControlAction.LockToggle -> {
          binding.quickToggleSwitch.setOnCheckedChangeListener(null)
          binding.quickToggleSwitch.visibility = View.GONE
          binding.quickLockButton.visibility = View.VISIBLE
          binding.sliderContainer.visibility = View.GONE
          binding.quickLockButton.text = if (qc.isLocked) "Unlock" else "Lock"
          binding.quickLockButton.setOnClickListener { cardActions.toggleLock(card) }
        }
        is QuickControlAction.BrightnessSlider -> {
          // The power toggle is shown only when the endpoint also has the OnOff cluster.
          binding.quickToggleSwitch.setOnCheckedChangeListener(null)
          binding.quickToggleSwitch.visibility = if (qc.isOn != null) View.VISIBLE else View.GONE
          binding.quickLockButton.visibility = View.GONE
          binding.sliderContainer.visibility = View.VISIBLE
          if (qc.isOn != null) {
            binding.quickToggleSwitch.isChecked = qc.isOn
            binding.quickToggleSwitch.setOnCheckedChangeListener { _, _ ->
              cardActions.togglePower(card)
            }
          }
          binding.quickLevelSlider.value = qc.levelPercent.toFloat().coerceIn(0f, 100f)
          binding.sliderValueText.text = "${qc.levelPercent}%"
        }
        else -> {
          binding.quickToggleSwitch.setOnCheckedChangeListener(null)
          binding.quickToggleSwitch.visibility = View.GONE
          binding.quickLockButton.visibility = View.GONE
          binding.sliderContainer.visibility = View.GONE
        }
      }

      binding.editAliasesButton.setOnClickListener { cardActions.editAliases(card) }
      binding.inspectEndpointsButton.setOnClickListener { cardActions.inspectEndpoints(card) }
    }
  }
}

class ClusterAttributeAdapter(
  private val toggleSwitch:
    (endpointId: Int, clusterId: Long, attributeId: Long, isChecked: Boolean) -> Unit,
  private val changeSlider:
    (endpointId: Int, clusterId: Long, attributeId: Long, value: Any) -> Unit,
  private val readAttribute: (endpointId: Int, clusterId: Long, attributeId: Long) -> Unit,
  private val editAttribute:
    (
      endpointId: Int,
      clusterId: Long,
      attributeId: Long,
      attrName: String,
      currentValue: Any?,
    ) -> Unit,
  private val invokeCommand: (endpointId: Int, clusterId: Long, cmd: MatterCommandMeta) -> Unit,
) : RecyclerView.Adapter<ClusterAttributeAdapter.ClusterViewHolder>() {

  private var endpointId: Int = 1
  private val clusters = mutableListOf<ClusterDescriptor>()

  fun submitClusters(epId: Int, newClusters: List<ClusterDescriptor>) {
    endpointId = epId
    clusters.clear()
    clusters.addAll(newClusters)
    notifyDataSetChanged()
  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ClusterViewHolder {
    val binding =
      ItemClusterAttributeEditorBinding.inflate(LayoutInflater.from(parent.context), parent, false)
    return ClusterViewHolder(binding)
  }

  override fun onBindViewHolder(holder: ClusterViewHolder, position: Int) {
    holder.bind(clusters[position])
  }

  override fun getItemCount(): Int = clusters.size

  inner class ClusterViewHolder(private val binding: ItemClusterAttributeEditorBinding) :
    RecyclerView.ViewHolder(binding.root) {

    fun bind(cluster: ClusterDescriptor) {
      binding.clusterNameText.text = "${cluster.clusterName} (0x${cluster.clusterId.toString(16)})"
      binding.clusterCategoryBadge.text = cluster.category
      binding.clusterDescriptionText.text = cluster.description

      binding.attributesContainer.removeAllViews()

      for (attr in cluster.attributes) {
        val row = createAttributeRow(cluster.clusterId, attr)
        binding.attributesContainer.addView(row)
      }

      if (cluster.commands.isNotEmpty()) {
        val cmdHeader =
          TextView(binding.root.context).apply {
            text = "Commands"
            textSize = 14f
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            setPadding(0, 16, 0, 8)
          }
        binding.attributesContainer.addView(cmdHeader)

        val chipGroup =
          com.google.android.material.chip.ChipGroup(binding.root.context).apply {
            setPadding(0, 0, 0, 8)
          }
        for (cmd in cluster.commands) {
          val chip =
            com.google.android.material.chip.Chip(binding.root.context).apply {
              text = "${cmd.name} (0x${cmd.commandId.toString(16)})"
              setOnClickListener { invokeCommand(endpointId, cluster.clusterId, cmd) }
            }
          chipGroup.addView(chip)
        }
        binding.attributesContainer.addView(chipGroup)
      }
    }

    private fun createAttributeRow(clusterId: Long, attr: WritableAttributeState): View {
      val ctx = binding.root.context
      return when (attr.controlType) {
        AttributeControlType.SWITCH -> createSwitchRow(ctx, clusterId, attr)
        AttributeControlType.SLIDER_COLOR_TEMP,
        AttributeControlType.SLIDER_NUMERIC -> createSliderRow(ctx, clusterId, attr)
        else -> createActionRow(ctx, clusterId, attr)
      }
    }

    private fun createLabeledHorizontalRow(
      ctx: android.content.Context,
      attr: WritableAttributeState,
      includeAccessMode: Boolean,
    ): LinearLayout {
      val layout =
        LinearLayout(ctx).apply {
          orientation = LinearLayout.HORIZONTAL
          gravity = android.view.Gravity.CENTER_VERTICAL
          setPadding(0, 8, 0, 8)
        }
      val textLayout =
        LinearLayout(ctx).apply {
          orientation = LinearLayout.VERTICAL
          layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
        }
      val nameTv =
        TextView(ctx).apply {
          text = "${attr.attributeName} (0x${attr.attributeId.toString(16)})"
          textSize = 13f
          typeface = android.graphics.Typeface.DEFAULT_BOLD
        }
      val accessText =
        if (includeAccessMode) {
          val mode = if (attr.isWritable) "Writable" else "Read-Only"
          "${attr.dataType} • $mode • Value: ${attr.formattedValue}"
        } else {
          "${attr.dataType} • Value: ${attr.formattedValue}"
        }
      val metaTv =
        TextView(ctx).apply {
          text = accessText
          textSize = 10f
        }
      textLayout.addView(nameTv)
      textLayout.addView(metaTv)
      layout.addView(textLayout)
      return layout
    }

    private fun createSwitchRow(
      ctx: android.content.Context,
      clusterId: Long,
      attr: WritableAttributeState,
    ): View {
      val layout = createLabeledHorizontalRow(ctx, attr, includeAccessMode = true)
      val switch =
        com.google.android.material.switchmaterial.SwitchMaterial(ctx).apply {
          showText = false
          textOn = ""
          textOff = ""
          isChecked = (attr.currentValue as? Boolean) == true
          isEnabled = attr.isWritable
          setOnCheckedChangeListener { _, isChecked ->
            toggleSwitch(endpointId, clusterId, attr.attributeId, isChecked)
          }
        }
      layout.addView(switch)
      return layout
    }

    private fun createSliderRow(
      ctx: android.content.Context,
      clusterId: Long,
      attr: WritableAttributeState,
    ): View {
      val layout =
        LinearLayout(ctx).apply {
          orientation = LinearLayout.VERTICAL
          setPadding(0, 8, 0, 8)
        }
      val header = LinearLayout(ctx).apply { orientation = LinearLayout.HORIZONTAL }
      val nameTv =
        TextView(ctx).apply {
          text = "${attr.attributeName} (0x${attr.attributeId.toString(16)})"
          textSize = 13f
          typeface = android.graphics.Typeface.DEFAULT_BOLD
          layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
        }
      val valTv =
        TextView(ctx).apply {
          text = attr.formattedValue
          textSize = 12f
          typeface = android.graphics.Typeface.DEFAULT_BOLD
        }
      header.addView(nameTv)
      header.addView(valTv)
      layout.addView(header)

      val slider =
        Slider(ctx).apply {
          valueFrom = (attr.minValue ?: 0.0).toFloat()
          valueTo = (attr.maxValue ?: 254.0).toFloat()
          val raw = (attr.currentValue as? Number)?.toFloat() ?: valueFrom
          value = raw.coerceIn(valueFrom, valueTo)
          stepSize = (attr.step ?: 1.0).toFloat()
          isEnabled = attr.isWritable
          clearOnChangeListeners()
          clearOnSliderTouchListeners()
          addOnChangeListener { _, v, fromUser ->
            if (!fromUser) return@addOnChangeListener
            valTv.text = "$v"
          }
          addOnSliderTouchListener(
            object : Slider.OnSliderTouchListener {
              override fun onStartTrackingTouch(slider: Slider) {}

              override fun onStopTrackingTouch(slider: Slider) {
                changeSlider(endpointId, clusterId, attr.attributeId, slider.value)
              }
            }
          )
        }
      layout.addView(slider)
      return layout
    }

    private fun createActionRow(
      ctx: android.content.Context,
      clusterId: Long,
      attr: WritableAttributeState,
    ): View {
      val layout = createLabeledHorizontalRow(ctx, attr, includeAccessMode = false)
      val readBtn =
        com.google.android.material.button
          .MaterialButton(ctx, null, com.google.android.material.R.attr.borderlessButtonStyle)
          .apply {
            text = "Read"
            textSize = 11f
            setOnClickListener { readAttribute(endpointId, clusterId, attr.attributeId) }
          }
      layout.addView(readBtn)

      if (attr.isWritable) {
        val editBtn =
          com.google.android.material.button
            .MaterialButton(
              ctx,
              null,
              com.google.android.material.R.attr.materialButtonOutlinedStyle,
            )
            .apply {
              text = "Edit"
              textSize = 11f
              setOnClickListener {
                editAttribute(
                  endpointId,
                  clusterId,
                  attr.attributeId,
                  attr.attributeName,
                  attr.currentValue,
                )
              }
            }
        layout.addView(editBtn)
      }
      return layout
    }
  }
}
