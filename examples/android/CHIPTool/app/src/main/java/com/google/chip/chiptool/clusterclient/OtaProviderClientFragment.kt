package com.google.chip.chiptool.clusterclient

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.net.Uri
import android.net.wifi.WifiManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.PowerManager
import android.os.SystemClock
import android.provider.OpenableColumns
import android.util.Log
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.DefaultClusterCallback
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping
import chip.devicecontroller.OTAProviderDelegate
import chip.devicecontroller.OTAProviderDelegate.QueryImageResponseStatusEnum
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlEntryStruct
import chip.devicecontroller.cluster.structs.OtaSoftwareUpdateRequestorClusterProviderLocation
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.Status
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OtaProviderClientFragmentBinding
import com.google.chip.chiptool.util.toAny
import java.io.BufferedInputStream
import java.io.IOException
import java.io.InputStream
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OtaProviderClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OtaProviderClientFragmentBinding? = null

  private var fileUri: Uri? = null
  @Volatile private var cachedFirmwareFileSize: Long = 0L
  @Volatile private var cachedTargetDeviceIds: List<Long> = emptyList()

  private val vendorId: Int
    get() = binding.vendorIdEd.text.toString().toInt()

  private val softwareVersion: Long
    get() = binding.softwareVersionEd.text.toString().toLongOrNull() ?: 0L

  private val otaProviderCallback = OtaProviderCallback()
  private val binding
    get() = _binding!!

  private val attributeList = ClusterIDMapping.OtaSoftwareUpdateRequestor.Attribute.values()

  // Power/radio locks are held while at least one BDX transfer is active. They are acquired in
  // handleBDXTransferSessionBegin, which the native layer pairs with handleBDXTransferSessionEnd.
  // OtaTransferService is started proactively when the user taps Announce while in the foreground
  // so Android 12+ (API 31+) does not block FGS startup if BDX Init arrives after the user
  // backgrounds the app. Active transfers and announced-waiting nodes are tracked as sets of node
  // IDs so duplicate callbacks cannot skew the reference count.
  private val transferLockGuard = Any()
  private val activeTransferNodes = HashSet<Long>()
  private val announcedWaitingNodes = HashSet<Long>()
  private var wakeLock: PowerManager.WakeLock? = null
  private var wifiLock: WifiManager.WifiLock? = null
  private var lastLockRenewalMs = 0L
  // Application context used to start OtaTransferService, kept so it can be stopped after detach.
  private var transferServiceContext: Context? = null
  private val lockHandler = Handler(Looper.getMainLooper())
  // Safety net in case an end callback is ever missed: the native BDX layer ends a stalled session
  // after 5 minutes (and BDX Init timeout is 2 minutes), so if no progress occurs within
  // LOCK_SAFETY_TIMEOUT_MS the locks and foreground service are dropped.
  private val lockSafetyReleaser = Runnable {
    Log.w(TAG, "No OTA transfer progress for $LOCK_SAFETY_TIMEOUT_MS ms, releasing transfer locks")
    releaseAllTransferLocks()
    renderMultiDeviceStatus()
  }

  private fun onNodesAnnounced(nodeIds: Collection<Long>) {
    synchronized(transferLockGuard) {
      for (nodeId in nodeIds) {
        if (!activeTransferNodes.contains(nodeId)) {
          announcedWaitingNodes.add(nodeId)
        }
      }
      if (announcedWaitingNodes.isNotEmpty() || activeTransferNodes.isNotEmpty()) {
        val appContext = context?.applicationContext
        if (appContext != null && transferServiceContext == null) {
          val summary =
            "Waiting for OTA QueryImage (${announcedWaitingNodes.size} node(s) announced)"
          if (OtaTransferService.start(appContext, summary)) {
            transferServiceContext = appContext
          }
        }
        armSafetyTimeoutLocked()
      }
    }
  }

  private fun onNodeQueryImageAccepted(nodeId: Long) {
    synchronized(transferLockGuard) {
      announcedWaitingNodes.add(nodeId)
      if (activeTransferNodes.remove(nodeId) && activeTransferNodes.isEmpty()) {
        releasePowerAndWifiLocksLocked()
      }
      val appContext = context?.applicationContext
      if (appContext != null && transferServiceContext == null) {
        val summary = "Waiting for OTA BDX Init (${announcedWaitingNodes.size} node(s) waiting)"
        if (OtaTransferService.start(appContext, summary)) {
          transferServiceContext = appContext
        }
      }
      armSafetyTimeoutLocked()
    }
  }

  private fun onNodeAnnounceFinished(nodeId: Long) {
    synchronized(transferLockGuard) {
      if (
        announcedWaitingNodes.remove(nodeId) &&
          activeTransferNodes.isEmpty() &&
          announcedWaitingNodes.isEmpty()
      ) {
        releaseTransferLocksLocked()
      }
    }
  }

  private fun onTransferSessionStarted(nodeId: Long) {
    synchronized(transferLockGuard) {
      announcedWaitingNodes.remove(nodeId)
      if (activeTransferNodes.add(nodeId) && activeTransferNodes.size == 1) {
        acquireTransferLocksLocked()
      }
      renewTransferLocksLocked()
    }
  }

  private fun onTransferSessionEnded(nodeId: Long) {
    synchronized(transferLockGuard) {
      announcedWaitingNodes.remove(nodeId)
      val removedActive = activeTransferNodes.remove(nodeId)
      if (removedActive && activeTransferNodes.isEmpty()) {
        releasePowerAndWifiLocksLocked()
      }
      if (activeTransferNodes.isEmpty() && announcedWaitingNodes.isEmpty()) {
        releaseTransferLocksLocked()
      }
    }
  }

  private fun onTransferProgress() {
    synchronized(transferLockGuard) {
      if (
        activeTransferNodes.isNotEmpty() &&
          SystemClock.elapsedRealtime() - lastLockRenewalMs >= LOCK_RENEW_INTERVAL_MS
      ) {
        renewTransferLocksLocked()
      }
    }
  }

  private fun releaseAllTransferLocks() {
    synchronized(transferLockGuard) {
      activeTransferNodes.clear()
      announcedWaitingNodes.clear()
      releaseTransferLocksLocked()
    }
  }

  private fun acquireTransferLocksLocked() {
    val context = context?.applicationContext ?: return
    if (wakeLock == null) {
      val powerManager = context.getSystemService(Context.POWER_SERVICE) as PowerManager
      wakeLock =
        powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOCK_TAG).apply {
          setReferenceCounted(false)
        }
    }
    if (wifiLock == null) {
      val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager
      // Note: WIFI_MODE_FULL_LOW_LATENCY is effective while the screen is on and the app is in the
      // foreground; when backgrounded or screen-off, OtaTransferService + PARTIAL_WAKE_LOCK keep
      // the BDX transfer session alive and exempt from Doze network restrictions.
      val mode =
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
          WifiManager.WIFI_MODE_FULL_LOW_LATENCY
        } else {
          @Suppress("DEPRECATION") WifiManager.WIFI_MODE_FULL_HIGH_PERF
        }
      wifiLock = wifiManager.createWifiLock(mode, LOCK_TAG).apply { setReferenceCounted(false) }
    }
    wifiLock?.acquire()
    Log.d(TAG, "Acquired OTA transfer wake lock and Wi-Fi lock")
    // Doze ignores wake locks of apps that are not in the foreground-service state.
    if (transferServiceContext == null && OtaTransferService.start(context)) {
      transferServiceContext = context
    }
  }

  private fun armSafetyTimeoutLocked() {
    lockHandler.removeCallbacks(lockSafetyReleaser)
    lockHandler.postDelayed(lockSafetyReleaser, LOCK_SAFETY_TIMEOUT_MS)
  }

  private fun renewTransferLocksLocked() {
    lastLockRenewalMs = SystemClock.elapsedRealtime()
    // Re-acquiring a non-reference-counted wake lock just restarts its timeout.
    wakeLock?.acquire(LOCK_SAFETY_TIMEOUT_MS)
    armSafetyTimeoutLocked()
  }

  private fun releasePowerAndWifiLocksLocked() {
    wakeLock?.let { if (it.isHeld) it.release() }
    wifiLock?.let { if (it.isHeld) it.release() }
    Log.d(TAG, "Released OTA transfer wake lock and Wi-Fi lock")
  }

  private fun releaseTransferLocksLocked() {
    lockHandler.removeCallbacks(lockSafetyReleaser)
    releasePowerAndWifiLocksLocked()
    transferServiceContext?.let { OtaTransferService.stop(it) }
    transferServiceContext = null
    lastNotificationUpdateElapsedMs = 0L
  }

  private fun ensureNotificationPermission() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      val ctx = context ?: return
      if (
        ContextCompat.checkSelfPermission(ctx, Manifest.permission.POST_NOTIFICATIONS) !=
          PackageManager.PERMISSION_GRANTED
      ) {
        requestPermissions(
          arrayOf(Manifest.permission.POST_NOTIFICATIONS),
          REQUEST_CODE_POST_NOTIFICATIONS,
        )
      }
    }
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    _binding = OtaProviderClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    // Keep the screen on while this page is visible. CPU/Wi-Fi locks are taken per BDX transfer.
    requireActivity().window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.selectFirmwareFileBtn.setOnClickListener { selectFirmwareFileBtnClick() }
    binding.updateOTAStatusBtn.setOnClickListener { updateOTAStatusBtnClick() }
    binding.announceOTAProviderBtn.setOnClickListener {
      refreshCachedTargetDeviceIds()
      scope.launch { sendAnnounceOTAProviderBtnClick() }
    }
    binding.retryFailedOtaBtn.setOnClickListener {
      refreshCachedTargetDeviceIds()
      scope.launch { retryFailedOTAUpdatesBtnClick() }
    }

    binding.writeAclBtn.setOnClickListener { scope.launch { sendAclBtnClick() } }

    binding.readAttributeBtn.setOnClickListener { scope.launch { readAttributeBtnClick() } }

    binding.writeAttributeBtn.setOnClickListener { scope.launch { writeAttributeBtnClick() } }

    setQueryImageSpinnerListener()

    val attributeNames = attributeList.map { it.name }

    binding.attributeSp.adapter =
      ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, attributeNames)

    binding.vendorIdEd.setText(ChipClient.VENDOR_ID.toString())
    binding.delayActionTimeEd.setText("0")

    renderMultiDeviceStatus()
    deviceController.startOTAProvider(otaProviderCallback)
    return binding.root
  }

  private suspend fun sendAclBtnClick() {
    val endpointId = 0
    val clusterId = ClusterIDMapping.AccessControl.ID
    val attributeId = ClusterIDMapping.AccessControl.Attribute.Acl.id

    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    deviceController.readAttributePath(
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          e: Exception,
        ) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : $e")
        }

        override fun onReport(nodeState: NodeState?) {
          Log.d(TAG, "onResponse")
          val tlv =
            nodeState
              ?.getEndpointState(endpointId)
              ?.getClusterState(clusterId)
              ?.getAttributeState(attributeId)
              ?.tlv
          requireActivity().runOnUiThread { showAddAccessControlDialog(tlv) }
        }
      },
      devicePtr,
      listOf(attributePath),
      0,
    )
  }

  private fun showAddAccessControlDialog(tlv: ByteArray?) {
    if (tlv == null) {
      Log.d(TAG, "Access Control read fail")
      showMessage("Access Control read fail")
      return
    }

    val dialogView =
      requireActivity().layoutInflater.inflate(R.layout.add_access_control_dialog, null)
    val groupIdEd = dialogView.findViewById<EditText>(R.id.groupIdEd)
    groupIdEd.visibility = View.GONE
    val nodeIdEd = dialogView.findViewById<EditText>(R.id.nodeIdEd)
    nodeIdEd.visibility = View.VISIBLE
    val accessControlEntrySp = dialogView.findViewById<Spinner>(R.id.accessControlEntrySp)
    accessControlEntrySp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        GroupSettingFragment.Companion.AccessControlEntry.values(),
      )

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()
    dialogView.findViewById<Button>(R.id.addAccessControlBtn).setOnClickListener {
      scope.launch {
        sendAccessControl(
          tlv,
          nodeIdEd.text.toString().toULong(),
          GroupSettingFragment.Companion.AccessControlEntry.valueOf(
              accessControlEntrySp.selectedItem.toString()
            )
            .id,
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendAccessControl(tlv: ByteArray, nodeId: ULong, privilege: UInt) {
    val tlvWriter = TlvWriter().startArray(AnonymousTag)
    var entryStructList: List<AccessControlClusterAccessControlEntryStruct>
    TlvReader(tlv).also {
      entryStructList = buildList {
        it.enterArray(AnonymousTag)
        while (!it.isEndOfContainer()) {
          add(AccessControlClusterAccessControlEntryStruct.fromTlv(AnonymousTag, it))
        }
        it.exitContainer()
      }
    }

    // If GroupID is already added to AccessControl, do not add it.
    for (entry in entryStructList) {
      if (
        entry.authMode == 2U /* CASE */ &&
          entry.subjects != null &&
          entry.subjects!!.contains(nodeId)
      ) {
        continue
      }

      entry.toTlv(AnonymousTag, tlvWriter)
    }

    val newEntry =
      AccessControlClusterAccessControlEntryStruct(
        privilege,
        2U /* CASE */,
        listOf(nodeId), /* single nodeId as subject. */
        null, /* null (empty) targets: wildcard access to everything */
        Optional.empty(), /* no auxiliaryType (does not apply to writes) */
        deviceController.fabricIndex.toUInt(),
      )
    newEntry.toTlv(AnonymousTag, tlvWriter)
    tlvWriter.endArray()

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    deviceController.write(
      object : WriteAttributesCallback {
        override fun onError(attributePath: ChipAttributePath?, e: Exception?) {
          Log.d(TAG, "onError : ", e)
          showMessage("Error : ${e.toString()}")
        }

        override fun onResponse(attributePath: ChipAttributePath, status: Status) {
          Log.d(TAG, "onResponse")
          showMessage("$attributePath : Write response: $status")
        }
      },
      devicePtr,
      listOf(
        AttributeWriteRequest.newInstance(
          0,
          ClusterIDMapping.AccessControl.ID,
          ClusterIDMapping.AccessControl.Attribute.Acl.id,
          tlvWriter.getEncoded(),
        )
      ),
      0,
      0,
    )
  }

  private suspend fun readAttributeBtnClick() {
    val attribute = attributeList[binding.attributeSp.selectedItemPosition]
    val endpointId = OTA_REQUESTER_ENDPOINT_ID
    val clusterId = ClusterIDMapping.OtaSoftwareUpdateRequestor.ID
    val attributeId = attribute.id
    val path = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    deviceController.readAttributePath(
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          e: Exception,
        ) {
          requireActivity().runOnUiThread {
            Toast.makeText(
                requireActivity(),
                R.string.ota_provider_invalid_attribute,
                Toast.LENGTH_SHORT,
              )
              .show()
          }
        }

        override fun onReport(nodeState: NodeState?) {
          val tlv =
            nodeState
              ?.getEndpointState(endpointId)
              ?.getClusterState(clusterId)
              ?.getAttributeState(attributeId)
              ?.tlv

          val value = tlv?.let { TlvReader(it).toAny() }
          Log.i(TAG, "OtaSoftwareUpdateRequestor ${attribute.name} value: $value")
          showMessage("OtaSoftwareUpdateRequestor ${attribute.name} value: $value")
        }
      },
      devicePtr,
      listOf<ChipAttributePath>(path),
      0,
    )
  }

  private fun writeAttributeBtnClick() {
    val attribute = attributeList[binding.attributeSp.selectedItemPosition]
    if (attribute != ClusterIDMapping.OtaSoftwareUpdateRequestor.Attribute.DefaultOTAProviders) {
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            R.string.ota_provider_invalid_attribute,
            Toast.LENGTH_SHORT,
          )
          .show()
      }
      return
    }
    val dialogView =
      requireActivity().layoutInflater.inflate(R.layout.write_default_otaproviders_dialog, null)
    val fabricIndexEd = dialogView.findViewById<EditText>(R.id.fabricIndexEd)
    val providerNodeIdEd = dialogView.findViewById<EditText>(R.id.providerNodeIdEd)
    val endpointIdEd = dialogView.findViewById<EditText>(R.id.endpointIdEd)

    fabricIndexEd.setText(deviceController.fabricIndex.toUInt().toString())
    providerNodeIdEd.setText(deviceController.controllerNodeId.toULong().toString())
    endpointIdEd.setText(OTA_PROVIDER_ENDPOINT_ID.toUInt().toString())

    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    dialogView.findViewById<Button>(R.id.writeDefaultOtaProvidersBtn).setOnClickListener {
      scope.launch {
        sendWriteDefaultOTAProviders(
          providerNodeIdEd.text.toString().toULong(),
          endpointIdEd.text.toString().toUInt(),
          fabricIndexEd.text.toString().toUInt(),
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendWriteDefaultOTAProviders(
    providerNodeId: ULong,
    endpointId: UInt,
    fabricIndex: UInt,
  ) {
    val endpoint = OTA_REQUESTER_ENDPOINT_ID
    val clusterId = ClusterIDMapping.OtaSoftwareUpdateRequestor.ID
    val attributeId = ClusterIDMapping.OtaSoftwareUpdateRequestor.Attribute.DefaultOTAProviders.id

    val tlv =
      TlvWriter()
        .apply {
          startArray(AnonymousTag)
          OtaSoftwareUpdateRequestorClusterProviderLocation(providerNodeId, endpointId, fabricIndex)
            .toTlv(AnonymousTag, this)
          endArray()
        }
        .getEncoded()

    val writeRequest = AttributeWriteRequest.newInstance(endpoint, clusterId, attributeId, tlv)

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    deviceController.write(
      object : WriteAttributesCallback {
        override fun onError(attributePath: ChipAttributePath?, e: Exception?) {
          Log.d(TAG, "onError")
          showMessage("error : ${e.toString()}")
        }

        override fun onResponse(attributePath: ChipAttributePath, status: Status) {
          Log.d(TAG, "onResponse")
          showMessage("$attributePath : Write response: $status")
        }
      },
      devicePtr,
      listOf<AttributeWriteRequest>(writeRequest),
      0,
      0,
    )
  }

  private fun setQueryImageSpinnerListener() {
    val statusList = QueryImageResponseStatusEnum.values()
    binding.titleStatusSp.adapter =
      ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, statusList)
    binding.titleStatusSp.onItemSelectedListener =
      object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
          val isBusy = statusList[position] == QueryImageResponseStatusEnum.Busy

          requireActivity().runOnUiThread {
            binding.delayActionTimeTv.visibility =
              if (isBusy) {
                View.VISIBLE
              } else {
                View.GONE
              }
            binding.delayActionTimeEd.visibility = binding.delayActionTimeTv.visibility
          }
        }

        override fun onNothingSelected(parent: AdapterView<*>?) {
          Log.d(TAG, "onNothingSelected")
        }
      }
  }

  private fun selectFirmwareFileBtnClick() {
    startActivityForResult(
      Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
        addCategory(Intent.CATEGORY_OPENABLE)
        type = "*/*"
      },
      REQUEST_CODE_OPEN_SAF,
    )
  }

  private fun getInputStream(uri: Uri?): InputStream? {
    if (uri == null) {
      return null
    }
    return requireContext().contentResolver.openInputStream(uri)
  }

  /** Skips [count] bytes, stopping early only at end of stream. */
  private fun skipFully(stream: InputStream, count: Long) {
    var remaining = count
    while (remaining > 0) {
      val skipped = stream.skip(remaining)
      if (skipped > 0) {
        remaining -= skipped
      } else if (stream.read() >= 0) {
        // skip() may return 0 before end of stream; read() distinguishes the two cases.
        remaining--
      } else {
        return
      }
    }
  }

  /** Fills [buffer] unless end of stream is reached first; returns the number of bytes read. */
  private fun readFully(stream: InputStream, buffer: ByteArray): Int {
    var total = 0
    while (total < buffer.size) {
      val read = stream.read(buffer, total, buffer.size - total)
      if (read < 0) {
        break
      }
      total += read
    }
    return total
  }

  private fun InputStream.closeQuietly() {
    try {
      close()
    } catch (e: IOException) {
      Log.d(TAG, "Failed to close firmware stream", e)
    }
  }

  private fun queryName(uri: Uri?): String? {
    if (uri == null) {
      return null
    }
    val cursor = requireContext().contentResolver.query(uri, null, null, null, null)
    return cursor?.let { c ->
      val nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME)
      c.moveToFirst()

      val name = cursor.getString(nameIndex)
      c.close()
      name
    }
  }

  override fun onActivityResult(requestCode: Int, resultCode: Int, intent: Intent?) {
    super.onActivityResult(requestCode, resultCode, intent)
    if (resultCode != Activity.RESULT_OK || intent == null) {
      return
    }
    val uri = intent.data
    if (uri == null) {
      Log.d(TAG, "onActivityResult : null")
      return
    }
    val filename = queryName(uri)
    fileUri = uri
    cachedFirmwareFileSize = getFileSize(uri)
    requireActivity().runOnUiThread { binding.firmwareFileTv.text = filename }
  }

  private fun updateOTAStatusBtnClick() {
    val version = softwareVersion
    val versionString = softwareVersion.toString()

    val filename = binding.firmwareFileTv.text.toString()
    Log.d(TAG, "updateOTAStatusBtnClick : $filename")

    when (binding.titleStatusSp.selectedItem.toString()) {
      QueryImageResponseStatusEnum.UpdateAvailable.name ->
        otaProviderCallback.setOTAFile(
          version,
          versionString,
          filename,
          fileUri,
          cachedFirmwareFileSize,
        )
      QueryImageResponseStatusEnum.Busy.name ->
        otaProviderCallback.setOTABusyError(
          binding.delayActionTimeEd.text.toString().toUInt(),
          binding.titleUserConsentNeededSp.selectedItem.toString().toBoolean(),
        )
      QueryImageResponseStatusEnum.NotAvailable.name ->
        otaProviderCallback.setOTANotAvailableError(
          binding.titleUserConsentNeededSp.selectedItem.toString().toBoolean()
        )
    }
  }

  private fun getFileSize(uri: Uri?): Long {
    if (uri == null) return 0L
    try {
      val cursor = requireContext().contentResolver.query(uri, null, null, null, null)
      cursor?.use { c ->
        val sizeIndex = c.getColumnIndex(OpenableColumns.SIZE)
        if (sizeIndex != -1 && c.moveToFirst()) {
          return c.getLong(sizeIndex)
        }
      }
    } catch (e: Exception) {
      Log.d(TAG, "Failed to query size from contentResolver cursor", e)
    }
    try {
      requireContext().contentResolver.openFileDescriptor(uri, "r")?.use { pfd ->
        return pfd.statSize
      }
    } catch (e: Exception) {
      Log.d(TAG, "Failed to get statSize from fileDescriptor", e)
    }
    return 0L
  }

  private fun formatTimeDuration(durationMs: Long): String {
    val totalSec = durationMs / 1000
    val hours = totalSec / 3600
    val minutes = (totalSec % 3600) / 60
    val seconds = totalSec % 60
    return if (hours > 0) {
      String.format("%02d:%02d:%02d", hours, minutes, seconds)
    } else {
      String.format("%02d:%02d", minutes, seconds)
    }
  }

  data class DeviceOtaStatus(
    val nodeId: Long,
    var status: String = "Idle",
    var blockIndex: Long = 0L,
    var startTimeMs: Long = 0L,
    var totalBytesTransferred: Long = 0L,
    var totalFileSize: Long = 0L,
    var transferSpeedKBps: Double = 0.0,
    var isComplete: Boolean = false,
    var endTimeMs: Long = 0L,
  )

  private class NodeCardViewHolder(
    val rootView: LinearLayout,
    val titleTv: TextView,
    val badgeTv: TextView,
    val progressBar: ProgressBar,
    val metricsTv: TextView,
    val statusTv: TextView,
    val retryBtn: Button,
  )

  private val deviceStatusMap = java.util.concurrent.ConcurrentHashMap<Long, DeviceOtaStatus>()
  private val deviceInputStreams =
    java.util.concurrent.ConcurrentHashMap<Long, BufferedInputStream>()
  private val nodeCardViews = HashMap<Long, NodeCardViewHolder>()
  private val badgeBackgroundCache = HashMap<String, GradientDrawable>()
  @Volatile private var lastUiRenderElapsedMs = 0L
  @Volatile private var lastNotificationUpdateElapsedMs = 0L

  private fun dpToPx(context: Context, dp: Int): Int =
    (dp * context.resources.displayMetrics.density).toInt()

  private fun createRoundedBackground(
    fillColor: Int,
    strokeColor: Int,
    radiusDp: Int,
    ctx: Context,
  ) =
    GradientDrawable().apply {
      shape = GradientDrawable.RECTANGLE
      cornerRadius = dpToPx(ctx, radiusDp).toFloat()
      setColor(fillColor)
      if (strokeColor != Color.TRANSPARENT) {
        setStroke(dpToPx(ctx, 1), strokeColor)
      }
    }

  private fun getOrCreateBadgeBackground(badgeBgHex: String, ctx: Context): GradientDrawable =
    badgeBackgroundCache.getOrPut(badgeBgHex) {
      createRoundedBackground(Color.parseColor(badgeBgHex), Color.TRANSPARENT, 12, ctx)
    }

  private fun getOrCreateNodeCard(
    context: Context,
    container: LinearLayout,
    nodeId: Long,
  ): NodeCardViewHolder {
    nodeCardViews[nodeId]?.let {
      return it
    }

    val pad12 = dpToPx(context, 12)
    val pad8 = dpToPx(context, 8)
    val pad4 = dpToPx(context, 4)

    val cardLayout =
      LinearLayout(context).apply {
        orientation = LinearLayout.VERTICAL
        setPadding(pad12, pad12, pad12, pad12)
        background =
          createRoundedBackground(
            Color.parseColor("#FFFFFF"),
            Color.parseColor("#CBD5E1"),
            8,
            context,
          )
        layoutParams =
          LinearLayout.LayoutParams(
              ViewGroup.LayoutParams.MATCH_PARENT,
              ViewGroup.LayoutParams.WRAP_CONTENT,
            )
            .apply { bottomMargin = pad8 }
      }

    val headerRow =
      LinearLayout(context).apply {
        orientation = LinearLayout.HORIZONTAL
        gravity = Gravity.CENTER_VERTICAL
        layoutParams =
          LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
          )
      }

    val titleTv =
      TextView(context).apply {
        text = "Node $nodeId"
        textSize = 15f
        setTypeface(typeface, Typeface.BOLD)
        setTextColor(Color.parseColor("#0F172A"))
        layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
      }

    val badgeTv =
      TextView(context).apply {
        textSize = 12f
        setTypeface(typeface, Typeface.BOLD)
        setPadding(pad8, pad4, pad8, pad4)
      }

    headerRow.addView(titleTv)
    headerRow.addView(badgeTv)

    val progressBar =
      ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal).apply {
        max = 100
        progress = 0
        isIndeterminate = false
        layoutParams =
          LinearLayout.LayoutParams(
              ViewGroup.LayoutParams.MATCH_PARENT,
              ViewGroup.LayoutParams.WRAP_CONTENT,
            )
            .apply {
              topMargin = pad8
              bottomMargin = pad4
            }
      }

    val metricsTv =
      TextView(context).apply {
        textSize = 12f
        typeface = Typeface.MONOSPACE
        setTextColor(Color.parseColor("#334155"))
        visibility = View.GONE
      }

    val footerRow =
      LinearLayout(context).apply {
        orientation = LinearLayout.HORIZONTAL
        gravity = Gravity.CENTER_VERTICAL
        layoutParams =
          LinearLayout.LayoutParams(
              ViewGroup.LayoutParams.MATCH_PARENT,
              ViewGroup.LayoutParams.WRAP_CONTENT,
            )
            .apply { topMargin = pad4 }
      }

    val statusTv =
      TextView(context).apply {
        textSize = 13f
        setTextColor(Color.parseColor("#475569"))
        layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
      }

    val retryBtn =
      Button(context).apply {
        setText(R.string.ota_provider_retry_node_btn_text)
        isAllCaps = false
        textSize = 12f
        minHeight = dpToPx(context, 48)
        visibility = View.GONE
        setOnClickListener { scope.launch { sendAnnounceOTAProviderBtnClick(listOf(nodeId)) } }
      }

    footerRow.addView(statusTv)
    footerRow.addView(retryBtn)

    cardLayout.addView(headerRow)
    cardLayout.addView(progressBar)
    cardLayout.addView(metricsTv)
    cardLayout.addView(footerRow)

    container.addView(cardLayout)
    return NodeCardViewHolder(
        cardLayout,
        titleTv,
        badgeTv,
        progressBar,
        metricsTv,
        statusTv,
        retryBtn,
      )
      .also { nodeCardViews[nodeId] = it }
  }

  private fun refreshCachedTargetDeviceIds(): List<Long> {
    val input = _binding?.targetDeviceIdsEd?.text?.toString()?.trim().orEmpty()
    val ids =
      if (input.isNotBlank()) {
        input
          .split(",")
          .mapNotNull { it.trim().toLongOrNull() }
          .ifEmpty { listOf(addressUpdateFragment.deviceId) }
      } else {
        listOf(addressUpdateFragment.deviceId)
      }
    cachedTargetDeviceIds = ids
    return ids
  }

  private fun getTargetDeviceIds(): List<Long> {
    if (Looper.myLooper() == Looper.getMainLooper()) {
      return refreshCachedTargetDeviceIds()
    }
    return cachedTargetDeviceIds
  }

  private fun isNodeStatusFailed(statusText: String): Boolean =
    statusText.contains("fail", ignoreCase = true) ||
      statusText.contains("error", ignoreCase = true)

  private fun isNodeEligibleForRetry(nodeId: Long): Boolean {
    val status = deviceStatusMap[nodeId]?.status.orEmpty()
    return isNodeStatusFailed(status) || status.isEmpty() || status == "Announcing OTA Provider..."
  }

  private fun updateDeviceStatus(
    nodeId: Long,
    status: String,
    blockIndex: Long = 0L,
    throttleUi: Boolean = false,
  ) {
    val deviceStatus = deviceStatusMap.getOrPut(nodeId) { DeviceOtaStatus(nodeId) }
    deviceStatus.status = status
    if (blockIndex > 0) {
      deviceStatus.blockIndex = blockIndex
    }
    val nowElapsed = SystemClock.elapsedRealtime()
    if (throttleUi && nowElapsed - lastUiRenderElapsedMs < UI_UPDATE_THROTTLE_MS) {
      return
    }
    lastUiRenderElapsedMs = nowElapsed
    renderMultiDeviceStatus()
  }

  private fun renderMultiDeviceStatus() {
    val activity = activity ?: return
    activity.runOnUiThread {
      val currentBinding = _binding ?: return@runOnUiThread
      val ctx = context ?: return@runOnUiThread

      val activeCount: Int
      val waitingCount: Int
      val wakeHeld: Boolean
      val wifiHeld: Boolean
      val fgsActive: Boolean
      synchronized(transferLockGuard) {
        activeCount = activeTransferNodes.size
        waitingCount = announcedWaitingNodes.size
        wakeHeld = wakeLock?.isHeld == true
        wifiHeld = wifiLock?.isHeld == true
        fgsActive = transferServiceContext != null
      }

      var aggregateSpeedKBps = 0.0
      var completedCount = 0
      var failedCount = 0
      var activePercentSum = 0.0
      var activePercentNodes = 0

      val sortedEntries = deviceStatusMap.entries.sortedBy { it.key }
      if (sortedEntries.isEmpty() && nodeCardViews.isNotEmpty()) {
        currentBinding.deviceProgressContainer.removeAllViews()
        nodeCardViews.clear()
      }

      for ((nodeId, status) in sortedEntries) {
        val isFailed = isNodeStatusFailed(status.status)
        val isDone =
          status.isComplete ||
            status.status.contains("Completed", ignoreCase = true) ||
            status.status.contains("Finished", ignoreCase = true)
        val isApplying = status.status.contains("Applying", ignoreCase = true)
        val isTransferring =
          !isDone &&
            !isFailed &&
            (status.blockIndex > 0 ||
              status.status.contains("Sending", ignoreCase = true) ||
              status.status.contains("Started", ignoreCase = true))

        if (isDone) completedCount++
        if (isFailed) failedCount++
        if (isTransferring) {
          aggregateSpeedKBps += status.transferSpeedKBps
          if (status.totalFileSize > 0) {
            val nodePct =
              ((status.totalBytesTransferred.toDouble() / status.totalFileSize.toDouble()) * 100.0)
                .coerceIn(0.0, 100.0)
            activePercentSum += nodePct
            activePercentNodes++
          }
        }

        val holder = getOrCreateNodeCard(ctx, currentBinding.deviceProgressContainer, nodeId)
        holder.titleTv.text = "Node $nodeId"

        val (badgeLabel, badgeTextHex, badgeBgHex) =
          when {
            isDone -> Triple("✓ Completed", "#166534", "#DCFCE7")
            isFailed -> Triple("✕ Failed", "#991B1B", "#FEE2E2")
            isApplying -> Triple("⚙ Applying", "#5B21B6", "#EDE9FE")
            isTransferring -> Triple("↑ Transferring", "#1E40AF", "#DBEAFE")
            status.status.contains("Announc", ignoreCase = true) ||
              status.status.contains("QueryImage", ignoreCase = true) ->
              Triple("⏳ Waiting", "#92400E", "#FEF3C7")
            else -> Triple("• Idle", "#334155", "#F1F5F9")
          }
        holder.badgeTv.text = badgeLabel
        holder.badgeTv.setTextColor(Color.parseColor(badgeTextHex))
        holder.badgeTv.background = getOrCreateBadgeBackground(badgeBgHex, ctx)

        val percentDouble =
          if (status.totalFileSize > 0) {
            ((status.totalBytesTransferred.toDouble() / status.totalFileSize.toDouble()) * 100.0)
              .coerceIn(0.0, 100.0)
          } else if (isDone) {
            100.0
          } else {
            0.0
          }
        holder.progressBar.progress = percentDouble.toInt()

        if (status.blockIndex > 0 || status.totalFileSize > 0) {
          val transferredKB = status.totalBytesTransferred / 1024.0
          val totalKB = status.totalFileSize / 1024.0
          val metricsBuilder = StringBuilder()
          if (status.totalFileSize > 0) {
            metricsBuilder.append(
              String.format("%.1f / %.1f KB (%.1f%%)", transferredKB, totalKB, percentDouble)
            )
          } else if (transferredKB > 0) {
            metricsBuilder.append(String.format("%.1f KB", transferredKB))
          }
          if (status.blockIndex > 0) {
            if (metricsBuilder.isNotEmpty()) metricsBuilder.append(" · ")
            metricsBuilder.append("Block #").append(status.blockIndex)
          }
          if (status.transferSpeedKBps > 0) {
            if (metricsBuilder.isNotEmpty()) metricsBuilder.append(" · ")
            metricsBuilder.append(String.format("%.1f KB/s", status.transferSpeedKBps))
          }
          holder.metricsTv.text = metricsBuilder.toString()
          holder.metricsTv.visibility = View.VISIBLE
        } else {
          holder.metricsTv.visibility = View.GONE
        }

        holder.statusTv.text = status.status
        holder.retryBtn.visibility = if (isFailed) View.VISIBLE else View.GONE
      }

      currentBinding.otaProviderSummaryTv.text =
        String.format(
          "Serving %d / %d concurrent · Aggregate: %.1f KB/s · Waiting: %d · Done: %d · Failed: %d",
          activeCount,
          MAX_CONCURRENT_BDX_SESSIONS,
          aggregateSpeedKBps,
          waitingCount,
          completedCount,
          failedCount,
        )
      currentBinding.otaLockStatusTv.text =
        String.format(
          "CPU WakeLock: %s · Wi-Fi Lock: %s · Foreground Service: %s",
          if (wakeHeld) "HELD" else "IDLE",
          if (wifiHeld) "HELD" else "IDLE",
          if (fgsActive) "ACTIVE" else "IDLE",
        )

      if (fgsActive && activeCount > 0) {
        val avgProgress =
          if (activePercentNodes > 0) (activePercentSum / activePercentNodes).toInt() else 0
        val nowElapsed = SystemClock.elapsedRealtime()
        if (
          avgProgress >= 100 ||
            nowElapsed - lastNotificationUpdateElapsedMs >= NOTIFICATION_UPDATE_THROTTLE_MS
        ) {
          lastNotificationUpdateElapsedMs = nowElapsed
          synchronized(transferLockGuard) {
            val liveActiveCount = activeTransferNodes.size
            val svcContext = transferServiceContext
            if (svcContext != null && liveActiveCount > 0) {
              val notifSummary =
                String.format(
                  "Serving %d node(s) · %d%% (%.1f KB/s)",
                  liveActiveCount,
                  avgProgress,
                  aggregateSpeedKBps,
                )
              OtaTransferService.updateProgress(
                svcContext,
                liveActiveCount,
                avgProgress,
                notifSummary,
              )
            }
          }
        }
      }
    }
  }

  private suspend fun sendAnnounceOTAProviderBtnClick(nodesToAnnounce: List<Long>? = null) {
    ensureNotificationPermission()
    requireActivity().runOnUiThread {
      refreshCachedTargetDeviceIds()
      updateOTAStatusBtnClick()
    }

    val activeSnapshot = synchronized(transferLockGuard) { activeTransferNodes.toSet() }
    val requestedNodeIds = nodesToAnnounce ?: getTargetDeviceIds()
    val targetNodeIds =
      if (nodesToAnnounce == null) {
        requestedNodeIds.filterNot { activeSnapshot.contains(it) }
      } else {
        requestedNodeIds
      }
    if (nodesToAnnounce == null) {
      val staleKeys = deviceStatusMap.keys.filterNot { activeSnapshot.contains(it) }
      for (staleKey in staleKeys) {
        deviceStatusMap.remove(staleKey)
      }
      activity?.runOnUiThread {
        val container = _binding?.deviceProgressContainer
        val iter = nodeCardViews.entries.iterator()
        while (iter.hasNext()) {
          val entry = iter.next()
          if (!activeSnapshot.contains(entry.key)) {
            container?.removeView(entry.value.rootView)
            iter.remove()
          }
        }
      }
    }
    for (nodeId in targetNodeIds) {
      deviceStatusMap[nodeId] = DeviceOtaStatus(nodeId, "Announcing OTA Provider...")
    }
    onNodesAnnounced(targetNodeIds)
    renderMultiDeviceStatus()

    for (nodeId in targetNodeIds) {
      val devicePtr =
        try {
          ChipClient.getConnectedDevicePointer(requireContext(), nodeId)
        } catch (e: IllegalStateException) {
          Log.d(TAG, "getConnectedDevicePointer exception for Node $nodeId", e)
          onNodeAnnounceFinished(nodeId)
          updateDeviceStatus(nodeId, "Connection failed: ${e.message}")
          continue
        }

      val otaRequestCluster =
        ChipClusters.OtaSoftwareUpdateRequestorCluster(devicePtr, OTA_REQUESTER_ENDPOINT_ID)
      otaRequestCluster.announceOTAProvider(
        object : DefaultClusterCallback {
          override fun onSuccess() {
            Log.i(TAG, "announceOTAProvider command success for Node $nodeId")
            updateDeviceStatus(nodeId, "Announced. Waiting for QueryImage...")
          }

          override fun onError(ex: java.lang.Exception?) {
            Log.e(TAG, "announceOTAProvider command failure for Node $nodeId", ex)
            onNodeAnnounceFinished(nodeId)
            updateDeviceStatus(nodeId, "Announce failed: ${ex?.message}")
          }
        },
        deviceController.controllerNodeId.toULong().toLong(),
        vendorId,
        0 /* AnnounceReason */,
        Optional.empty(),
        OTA_PROVIDER_ENDPOINT_ID,
      )
    }
  }

  private suspend fun retryFailedOTAUpdatesBtnClick() {
    val failedNodeIds = getTargetDeviceIds().filter { isNodeEligibleForRetry(it) }

    if (failedNodeIds.isEmpty()) {
      requireActivity().runOnUiThread {
        Toast.makeText(requireContext(), "No failed OTA nodes to retry", Toast.LENGTH_SHORT).show()
      }
      return
    }

    sendAnnounceOTAProviderBtnClick(failedNodeIds)
  }

  override fun onDestroyView() {
    super.onDestroyView()
    // Finish the provider first: it ends every active BDX session (delivering
    // handleBDXTransferSessionEnd synchronously) and guarantees no further delegate callbacks.
    deviceController.finishOTAProvider()
    for ((_, stream) in deviceInputStreams) {
      stream.closeQuietly()
    }
    deviceInputStreams.clear()
    nodeCardViews.clear()
    badgeBackgroundCache.clear()
    releaseAllTransferLocks()
    activity?.window?.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
    _binding = null
  }

  inner class OtaProviderCallback : OTAProviderDelegate {
    private var fileName: String? = null
    private var version: Long = 0
    private var versionString: String? = null
    private var uri: Uri? = null
    private var fileSize: Long = 0L

    private var status: QueryImageResponseStatusEnum? = null
    private var delayedTime: UInt? = null
    private var userConsentNeeded: Boolean? = null

    fun setOTAFile(
      version: Long,
      versionString: String,
      fileName: String,
      uri: Uri?,
      fileSize: Long = 0L,
      userConsentNeeded: Boolean? = null,
    ) {
      this.status = QueryImageResponseStatusEnum.UpdateAvailable
      this.version = version
      this.versionString = versionString
      this.fileName = fileName
      this.uri = uri
      this.fileSize = fileSize
      this.delayedTime = null
      this.userConsentNeeded = userConsentNeeded
    }

    fun setOTABusyError(delayedTime: UInt, userConsentNeeded: Boolean? = null) {
      this.status = QueryImageResponseStatusEnum.Busy
      this.delayedTime = delayedTime
      this.userConsentNeeded = userConsentNeeded
    }

    fun setOTANotAvailableError(userConsentNeeded: Boolean? = null) {
      this.status = QueryImageResponseStatusEnum.NotAvailable
      this.delayedTime = null
      this.userConsentNeeded = userConsentNeeded
    }

    override fun handleQueryImage(
      nodeId: Long,
      vendorId: Int,
      productId: Int,
      softwareVersion: Long,
      hardwareVersion: Int?,
      location: String?,
      requestorCanConsent: Boolean?,
      metadataForProvider: ByteArray?,
    ): OTAProviderDelegate.QueryImageResponse? {
      Log.d(
        TAG,
        "handleQueryImage, nodeId=$nodeId, $vendorId, $productId, " +
          "$softwareVersion, $hardwareVersion, $location",
      )
      val resolvedNodeId =
        if (nodeId != 0L) {
          nodeId
        } else {
          val announcedNodeId =
            getTargetDeviceIds().firstOrNull {
              deviceStatusMap[it]?.status?.startsWith("Announced") == true
            }
          announcedNodeId ?: 0L
        }

      if (resolvedNodeId != 0L) {
        deviceInputStreams.remove(resolvedNodeId)?.closeQuietly()
        deviceStatusMap[resolvedNodeId]?.apply {
          blockIndex = 0L
          totalBytesTransferred = 0L
          transferSpeedKBps = 0.0
          isComplete = false
          startTimeMs = 0L
          endTimeMs = 0L
        }
        if (status == QueryImageResponseStatusEnum.UpdateAvailable) {
          onNodeQueryImageAccepted(resolvedNodeId)
          updateDeviceStatus(resolvedNodeId, "QueryImage received (SW Ver: $softwareVersion)")
        } else {
          onNodeAnnounceFinished(resolvedNodeId)
          updateDeviceStatus(
            resolvedNodeId,
            "QueryImage received (SW Ver: $softwareVersion, response: $status)",
          )
        }
      }

      return when (status) {
        QueryImageResponseStatusEnum.UpdateAvailable ->
          OTAProviderDelegate.QueryImageResponse(version, versionString, fileName, null)
        QueryImageResponseStatusEnum.Busy ->
          OTAProviderDelegate.QueryImageResponse(
            status,
            delayedTime?.toLong() ?: 0,
            userConsentNeeded ?: false,
          )
        QueryImageResponseStatusEnum.NotAvailable ->
          OTAProviderDelegate.QueryImageResponse(status, userConsentNeeded ?: false)
        else -> null
      }
    }

    override fun handleOTAQueryFailure(nodeId: Long, error: Int) {
      Log.d(TAG, "handleOTAQueryFailure, nodeId=$nodeId, error=$error")
      if (nodeId != 0L) {
        deviceInputStreams.remove(nodeId)?.closeQuietly()
        onTransferSessionEnded(nodeId)
        updateDeviceStatus(nodeId, "OTA QueryImage failed (status: $error)")
      }
      showMessage("handleOTAQueryFailure (node $nodeId): $error")
    }

    override fun handleApplyUpdateRequest(
      nodeId: Long,
      newVersion: Long,
    ): OTAProviderDelegate.ApplyUpdateResponse {
      Log.d(TAG, "handleApplyUpdateRequest, $nodeId, $newVersion")
      updateDeviceStatus(nodeId, "Applying Update (Target Version: $newVersion)")
      return OTAProviderDelegate.ApplyUpdateResponse(
        OTAProviderDelegate.ApplyUpdateActionEnum.Proceed,
        APPLY_WAITING_TIME,
      )
    }

    override fun handleNotifyUpdateApplied(nodeId: Long) {
      Log.d(TAG, "handleNotifyUpdateApplied, $nodeId")
      updateDeviceStatus(nodeId, "Firmware Update Completed Successfully!")
    }

    override fun handleBDXTransferSessionBegin(
      nodeId: Long,
      fileDesignator: String?,
      offset: Long,
    ) {
      Log.d(TAG, "handleBDXTransferSessionBegin, $nodeId, $fileDesignator, $offset")
      onTransferSessionStarted(nodeId)
      // A redundant QueryImage/BDX Init from the same node supersedes its previous session.
      deviceInputStreams.remove(nodeId)?.closeQuietly()
      // Reset startTimeMs before opening the firmware stream so a failed begin cannot reuse a
      // stale timestamp from a prior run when handleBDXTransferSessionEnd runs.
      deviceStatusMap[nodeId]?.startTimeMs = 0L
      var bufferedInputStream: BufferedInputStream? = null
      try {
        val inputStream = getInputStream(uri) ?: throw IOException("No firmware file selected")
        bufferedInputStream = BufferedInputStream(inputStream)
        skipFully(bufferedInputStream, offset)
        deviceInputStreams[nodeId] = bufferedInputStream
        val deviceStatus = deviceStatusMap.getOrPut(nodeId) { DeviceOtaStatus(nodeId) }
        deviceStatus.startTimeMs = System.currentTimeMillis()
        deviceStatus.totalBytesTransferred = 0L
        deviceStatus.totalFileSize = fileSize
        deviceStatus.transferSpeedKBps = 0.0
        deviceStatus.isComplete = false
        deviceStatus.endTimeMs = 0L
        updateDeviceStatus(nodeId, "BDX Transfer Started (offset: $offset)")
      } catch (e: Exception) {
        // IOException, or SecurityException if access to the selected file was revoked. The first
        // block query will find no stream and abort, which triggers handleBDXTransferSessionEnd
        // after the native layer sends the abort StatusReport.
        Log.d(TAG, "exception for Node $nodeId", e)
        updateDeviceStatus(nodeId, "BDX Session Error: ${e.message}")
        bufferedInputStream?.closeQuietly()
        return
      }
    }

    override fun handleBDXTransferSessionEnd(errorCode: Long, nodeId: Long) {
      Log.d(TAG, "handleBDXTransferSessionEnd, $errorCode, $nodeId")
      deviceInputStreams.remove(nodeId)?.closeQuietly()
      onTransferSessionEnded(nodeId)
      val statusObj = deviceStatusMap[nodeId]
      if (statusObj != null) {
        statusObj.endTimeMs = System.currentTimeMillis()
        statusObj.isComplete = (errorCode == 0L)
        if (statusObj.startTimeMs == 0L) {
          val msg =
            if (errorCode == 0L) statusObj.status else "${statusObj.status} (BDX error $errorCode)"
          updateDeviceStatus(nodeId, msg)
          return
        }
        val totalMs = statusObj.endTimeMs - statusObj.startTimeMs
        val lastingTimeStr = formatTimeDuration(totalMs)
        val finalSpeedMsg =
          if (statusObj.transferSpeedKBps > 0) {
            String.format(
              " (Avg: %.1f KB/s / %.1f kbps, Total OTA Time: %s)",
              statusObj.transferSpeedKBps,
              statusObj.transferSpeedKBps * 8.0,
              lastingTimeStr,
            )
          } else " (Total OTA Time: $lastingTimeStr)"
        val msg =
          if (errorCode == 0L) "BDX Transfer Finished$finalSpeedMsg"
          else "BDX Transfer Ended with error $errorCode (Total OTA Time: $lastingTimeStr)"
        updateDeviceStatus(nodeId, msg)
      } else {
        val msg =
          if (errorCode == 0L) "BDX Transfer Finished"
          else "BDX Transfer Ended with error $errorCode"
        updateDeviceStatus(nodeId, msg)
      }
    }

    override fun handleBDXQuery(
      nodeId: Long,
      blockSize: Int,
      blockIndex: Long,
      bytesToSkip: Long,
    ): OTAProviderDelegate.BDXData? {
      Log.d(TAG, "handleBDXQuery, $nodeId, $blockSize, $blockIndex, $bytesToSkip")
      val bufferedInputStream = deviceInputStreams[nodeId]
      if (bufferedInputStream == null) {
        // Returning null makes the native layer abort the transfer rather than report a bogus EOF.
        Log.e(TAG, "No firmware stream for Node $nodeId, aborting BDX transfer")
        return null
      }
      val packet = ByteArray(blockSize)
      val len =
        try {
          skipFully(bufferedInputStream, bytesToSkip)
          readFully(bufferedInputStream, packet)
        } catch (e: IOException) {
          // Native BdxOTASession aborts on null return and invokes handleBDXTransferSessionEnd
          // once the abort StatusReport is sent, which releases the session's transfer locks.
          Log.e(TAG, "Failed to read firmware for Node $nodeId", e)
          updateDeviceStatus(nodeId, "BDX Read Error: ${e.message}")
          deviceInputStreams.remove(nodeId)?.closeQuietly()
          return null
        }
      onTransferProgress()

      val sendPacket = if (len < blockSize) packet.copyOf(len) else packet

      val isEOF = len < blockSize

      val deviceStatus = deviceStatusMap.getOrPut(nodeId) { DeviceOtaStatus(nodeId) }
      if (sendPacket.isNotEmpty()) {
        deviceStatus.totalBytesTransferred += sendPacket.size
        val elapsedTimeSec = (System.currentTimeMillis() - deviceStatus.startTimeMs) / 1000.0
        if (elapsedTimeSec > 0) {
          deviceStatus.transferSpeedKBps =
            (deviceStatus.totalBytesTransferred / 1024.0) / elapsedTimeSec
        }
      }

      val now = System.currentTimeMillis()
      val lastingMs = if (deviceStatus.startTimeMs > 0) now - deviceStatus.startTimeMs else 0L
      val lastingText = formatTimeDuration(lastingMs)

      var etaText = ""
      if (deviceStatus.totalFileSize > 0 && deviceStatus.transferSpeedKBps > 0) {
        val remainingBytes = deviceStatus.totalFileSize - deviceStatus.totalBytesTransferred
        if (remainingBytes > 0) {
          val speedBytesPerSec = deviceStatus.transferSpeedKBps * 1024.0
          val remainingMs = ((remainingBytes / speedBytesPerSec) * 1000).toLong()
          etaText = ", ETA: ${formatTimeDuration(remainingMs)}"
        } else if (isEOF) {
          etaText = ", ETA: 00:00"
        }
      }

      val speedText =
        if (deviceStatus.transferSpeedKBps > 0) {
          val kbps = deviceStatus.transferSpeedKBps * 8.0
          String.format("%.1f KB/s [%.1f kbps]", deviceStatus.transferSpeedKBps, kbps)
        } else ""

      val statusMsg =
        if (speedText.isNotEmpty()) {
          "Sending firmware... Speed: $speedText (Elapsed: $lastingText$etaText)"
        } else {
          "Sending firmware... (Elapsed: $lastingText)"
        }
      updateDeviceStatus(nodeId, statusMsg, blockIndex, throttleUi = !isEOF)

      return OTAProviderDelegate.BDXData(sendPacket, isEOF)
    }
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onCommissioningComplete(nodeId: Long, errorCode: Long) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
      showMessage("Address update complete for nodeId $nodeId with code $errorCode")
    }

    override fun onNotifyChipConnectionClosed() {
      Log.d(TAG, "onNotifyChipConnectionClosed")
    }

    override fun onCloseBleComplete() {
      Log.d(TAG, "onCloseBleComplete")
    }

    override fun onError(error: Throwable?) {
      Log.d(TAG, "onError: $error")
    }
  }

  private fun showMessage(msg: String) {
    // Also called from the CHIP thread, possibly after the view has been destroyed.
    val activity = activity ?: return
    activity.runOnUiThread { _binding?.commandStatusTv?.text = msg }
  }

  companion object {
    private const val TAG = "OtaProviderClientFragment"

    fun newInstance(): OtaProviderClientFragment = OtaProviderClientFragment()

    private const val REQUEST_CODE_OPEN_SAF = 100
    private const val REQUEST_CODE_POST_NOTIFICATIONS = 101

    private const val APPLY_WAITING_TIME = 10L
    private const val OTA_PROVIDER_ENDPOINT_ID = 0
    private const val OTA_REQUESTER_ENDPOINT_ID = 0

    private const val MAX_CONCURRENT_BDX_SESSIONS = 8
    private const val UI_UPDATE_THROTTLE_MS = 250L
    private const val NOTIFICATION_UPDATE_THROTTLE_MS = 1000L

    private const val LOCK_TAG = "CHIPTool:OtaTransfer"
    // Longer than the 5 minute BDX transfer timeout, so a live session never loses its locks.
    private const val LOCK_SAFETY_TIMEOUT_MS = 10 * 60 * 1000L
    private const val LOCK_RENEW_INTERVAL_MS = 30 * 1000L
  }
}
