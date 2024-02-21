package com.google.chip.chiptool.clusterclient

import android.app.Activity
import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import android.widget.Toast
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

  private val vendorId: Int
    get() = binding.vendorIdEd.text.toString().toInt()

  private val otaProviderCallback = OtaProviderCallback()
  private val binding
    get() = _binding!!

  private val attributeList = ClusterIDMapping.OtaSoftwareUpdateRequestor.Attribute.values()

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = OtaProviderClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.selectFirmwareFileBtn.setOnClickListener { selectFirmwareFileBtnClick() }
    binding.updateOTAStatusBtn.setOnClickListener { updateOTAStatusBtnClick() }
    binding.announceOTAProviderBtn.setOnClickListener {
      scope.launch { sendAnnounceOTAProviderBtnClick() }
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
          e: Exception
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
      0
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
        GroupSettingFragment.Companion.AccessControlEntry.values()
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
            .id
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
        listOf(nodeId),
        null,
        deviceController.fabricIndex.toUInt()
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
          tlvWriter.getEncoded()
        )
      ),
      0,
      0
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
          e: Exception
        ) {
          requireActivity().runOnUiThread {
            Toast.makeText(
                requireActivity(),
                R.string.ota_provider_invalid_attribute,
                Toast.LENGTH_SHORT
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
      0
    )
  }

  private fun writeAttributeBtnClick() {
    val attribute = attributeList[binding.attributeSp.selectedItemPosition]
    if (attribute != ClusterIDMapping.OtaSoftwareUpdateRequestor.Attribute.DefaultOTAProviders) {
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            R.string.ota_provider_invalid_attribute,
            Toast.LENGTH_SHORT
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
          fabricIndexEd.text.toString().toUInt()
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendWriteDefaultOTAProviders(
    providerNodeId: ULong,
    endpointId: UInt,
    fabricIndex: UInt
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
      0
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
      REQUEST_CODE_OPEN_SAF
    )
  }

  private fun getInputStream(uri: Uri?): InputStream? {
    if (uri == null) {
      return null
    }
    return requireContext().contentResolver.openInputStream(uri)
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
    requireActivity().runOnUiThread { binding.firmwareFileTv.text = filename }
  }

  private fun updateOTAStatusBtnClick() {
    val version = 2L
    val versionString = "2.0"

    val filename = binding.firmwareFileTv.text.toString()
    Log.d(TAG, "updateOTAStatusBtnClick : $filename")

    when (binding.titleStatusSp.selectedItem.toString()) {
      QueryImageResponseStatusEnum.UpdateAvailable.name ->
        otaProviderCallback.setOTAFile(version, versionString, filename, fileUri)
      QueryImageResponseStatusEnum.Busy.name ->
        otaProviderCallback.setOTABusyError(
          binding.delayActionTimeEd.text.toString().toUInt(),
          binding.titleUserConsentNeededSp.selectedItem.toString().toBoolean()
        )
      QueryImageResponseStatusEnum.NotAvailable.name ->
        otaProviderCallback.setOTANotAvailableError(
          binding.titleUserConsentNeededSp.selectedItem.toString().toBoolean()
        )
    }
  }

  private suspend fun sendAnnounceOTAProviderBtnClick() {
    requireActivity().runOnUiThread { updateOTAStatusBtnClick() }

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    val otaRequestCluster =
      ChipClusters.OtaSoftwareUpdateRequestorCluster(devicePtr, OTA_REQUESTER_ENDPOINT_ID)
    otaRequestCluster.announceOTAProvider(
      object : DefaultClusterCallback {
        override fun onSuccess() {
          showMessage("announceOTAProvider command success")
          Log.e(TAG, "announceOTAProvider command success")
        }

        override fun onError(ex: java.lang.Exception?) {
          showMessage("announceOTAProvider command failure $ex")
          Log.e(TAG, "announceOTAProvider command failure", ex)
        }
      },
      deviceController.controllerNodeId.toULong().toLong(),
      vendorId,
      0 /* AnnounceReason */,
      Optional.empty(),
      OTA_PROVIDER_ENDPOINT_ID
    )
  }

  override fun onDestroyView() {
    super.onDestroyView()
    deviceController.finishOTAProvider()
    _binding = null
  }

  inner class OtaProviderCallback : OTAProviderDelegate {
    private var fileName: String? = null
    private var version: Long = 0
    private var versionString: String? = null
    private var uri: Uri? = null

    private var inputStream: InputStream? = null
    private var bufferedInputStream: BufferedInputStream? = null

    private var status: QueryImageResponseStatusEnum? = null
    private var delayedTime: UInt? = null
    private var userConsentNeeded: Boolean? = null

    fun setOTAFile(
      version: Long,
      versionString: String,
      fileName: String,
      uri: Uri?,
      userConsentNeeded: Boolean? = null
    ) {
      this.status = QueryImageResponseStatusEnum.UpdateAvailable
      this.version = version
      this.versionString = versionString
      this.fileName = fileName
      this.uri = uri
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
      vendorId: Int,
      productId: Int,
      softwareVersion: Long,
      hardwareVersion: Int?,
      location: String?,
      requestorCanConsent: Boolean?,
      metadataForProvider: ByteArray?
    ): OTAProviderDelegate.QueryImageResponse? {
      Log.d(
        TAG,
        "handleQueryImage, $vendorId, $productId, $softwareVersion, $hardwareVersion, $location"
      )

      return when (status) {
        QueryImageResponseStatusEnum.UpdateAvailable ->
          OTAProviderDelegate.QueryImageResponse(version, versionString, fileName, null)
        QueryImageResponseStatusEnum.Busy ->
          OTAProviderDelegate.QueryImageResponse(
            status,
            delayedTime?.toLong() ?: 0,
            userConsentNeeded ?: false
          )
        QueryImageResponseStatusEnum.NotAvailable ->
          OTAProviderDelegate.QueryImageResponse(status, userConsentNeeded ?: false)
        else -> null
      }
    }

    override fun handleOTAQueryFailure(error: Int) {
      Log.d(TAG, "handleOTAQueryFailure, $error")
      showMessage("handleOTAQueryFailure : $error")
    }

    override fun handleApplyUpdateRequest(
      nodeId: Long,
      newVersion: Long
    ): OTAProviderDelegate.ApplyUpdateResponse {
      Log.d(TAG, "handleApplyUpdateRequest, $nodeId, $newVersion")
      return OTAProviderDelegate.ApplyUpdateResponse(
        OTAProviderDelegate.ApplyUpdateActionEnum.Proceed,
        APPLY_WAITING_TIME
      )
    }

    override fun handleNotifyUpdateApplied(nodeId: Long) {
      Log.d(TAG, "handleNotifyUpdateApplied, $nodeId")
      showMessage("Finish Firmware Update : $nodeId")
    }

    override fun handleBDXTransferSessionBegin(
      nodeId: Long,
      fileDesignator: String?,
      offset: Long
    ) {
      Log.d(TAG, "handleBDXTransferSessionBegin, $nodeId, $fileDesignator, $offset")
      try {
        inputStream = getInputStream(uri)
        bufferedInputStream = BufferedInputStream(inputStream)
      } catch (e: IOException) {
        Log.d(TAG, "exception", e)
        inputStream?.close()
        bufferedInputStream?.close()
        inputStream = null
        bufferedInputStream = null
        return
      }
    }

    override fun handleBDXTransferSessionEnd(errorCode: Long, nodeId: Long) {
      Log.d(TAG, "handleBDXTransferSessionEnd, $errorCode, $nodeId")
      inputStream?.close()
      bufferedInputStream?.close()
      inputStream = null
      bufferedInputStream = null
    }

    override fun handleBDXQuery(
      nodeId: Long,
      blockSize: Int,
      blockIndex: Long,
      bytesToSkip: Long
    ): OTAProviderDelegate.BDXData? {
      // This code is just example code. This code doesn't check blockIndex and bytesToSkip
      // variable.
      Log.d(TAG, "handleBDXQuery, $nodeId, $blockSize, $blockIndex, $bytesToSkip")
      showMessage("sending.. $blockIndex")
      if (bufferedInputStream == null) {
        return OTAProviderDelegate.BDXData(ByteArray(0), true)
      }
      val packet = ByteArray(blockSize)
      val len = bufferedInputStream!!.read(packet)

      val sendPacket =
        if (len < blockSize) {
          packet.copyOf(len)
        } else if (len < 0) {
          ByteArray(0)
        } else {
          packet.clone()
        }

      val isEOF = len < blockSize

      return OTAProviderDelegate.BDXData(sendPacket, isEOF)
    }
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
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
    requireActivity().runOnUiThread { binding.commandStatusTv.text = msg }
  }

  companion object {
    private const val TAG = "OtaProviderClientFragment"

    fun newInstance(): OtaProviderClientFragment = OtaProviderClientFragment()

    private const val REQUEST_CODE_OPEN_SAF = 100

    private const val APPLY_WAITING_TIME = 10L
    private const val OTA_PROVIDER_ENDPOINT_ID = 0
    private const val OTA_REQUESTER_ENDPOINT_ID = 0
  }
}
