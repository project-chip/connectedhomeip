package com.google.chip.chiptool.clusterclient

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.DefaultClusterCallback
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.OTAProviderDelegate
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OtaProviderClientFragmentBinding
import java.io.BufferedInputStream
import java.io.IOException
import java.io.InputStream
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

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
    binding.announceOTAProviderBtn.setOnClickListener {
      scope.launch { sendAnnounceOTAProviderBtnClick() }
    }

    binding.vendorIdEd.setText(ChipClient.VENDOR_ID.toString())

    deviceController.startOTAProvider(otaProviderCallback)
    return binding.root
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

  private suspend fun sendAnnounceOTAProviderBtnClick() {
    requireActivity().runOnUiThread {
      val version = 2L
      val versionString = "2.0"

      val filename = binding.firmwareFileTv.text.toString()
      Log.d(TAG, "sendAnnounceOTAProviderBtnClick : $filename")
      otaProviderCallback.setOTAFile(version, versionString, filename, fileUri)
    }

    val otaRequestCluster =
      ChipClusters.OtaSoftwareUpdateRequestorCluster(
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
        OTA_REQUESTER_ENDPOINT_ID
      )
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

    fun setOTAFile(version: Long, versionString: String, fileName: String, uri: Uri?) {
      this.version = version
      this.versionString = versionString
      this.fileName = fileName
      this.uri = uri
    }

    override fun handleQueryImage(
      vendorId: Int,
      productId: Int,
      softwareVersion: Long,
      hardwareVersion: Int?,
      location: String?,
      requestorCanConsent: Boolean?,
      metadataForProvider: ByteArray?
    ): OTAProviderDelegate.QueryImageResponse {
      Log.d(
        TAG,
        "handleQueryImage, $vendorId, $productId, $softwareVersion, $hardwareVersion, $location"
      )
      return OTAProviderDelegate.QueryImageResponse(version, versionString, fileName)
    }

    override fun handleOTAQueryFailure(error: Int) {
      Log.d(TAG, "handleOTAQueryFailure, $error")
      showMessage("handleOTAQueryFailure : $error")
      deviceController.finishOTAProvider()
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
