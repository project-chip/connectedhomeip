package com.google.chip.chiptool.clusterclient

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.database.Cursor
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.DocumentsContract
import android.provider.MediaStore
import android.provider.Settings
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.app.ActivityCompat
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
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class OtaProviderClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OtaProviderClientFragmentBinding? = null

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

    deviceController.startOTAProvider(otaProviderCallback)
    return binding.root
  }

  override fun onStart() {
    super.onStart()
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R && !Environment.isExternalStorageManager()) {
      Toast.makeText(
          requireContext(),
          "Require to Allow management of all files permission",
          Toast.LENGTH_LONG
        )
        .show()
      startActivity(
        Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION).apply {
          addCategory("android.intent.category.DEFAULT")
          data = Uri.parse("package:${requireContext().packageName}")
        }
      )
    } else if (!checkPermissionForReadExternalStorage()) {
      requestPermissionForReadExternalStorage()
    }
  }

  private fun selectFirmwareFileBtnClick() {
    val intent = Intent(Intent.ACTION_GET_CONTENT)
    val uri = Uri.parse(Environment.getExternalStorageDirectory().path + "/Download/")
    intent.setDataAndType(uri, "*/*")
    startActivityForResult(intent, 0)
  }

  override fun onActivityResult(requestCode: Int, resultCode: Int, intent: Intent?) {
    super.onActivityResult(requestCode, resultCode, intent)
    if (intent == null) {
      return
    }
    val uri = intent.data
    if (uri == null) {
      Log.d(TAG, "onActivityResult : null")
      return
    }

    val filename = getRealPathFromURI(uri)

    requireActivity().runOnUiThread { binding.firmwareFileTv.text = filename }
  }

  private fun getRealPathFromURI(contentUri: Uri): String? {
    var path = contentUri.path
    if (path == null || path.startsWith("/storage")) {
      return path
    }
    val id =
      DocumentsContract.getDocumentId(contentUri)
        .split(":".toRegex())
        .dropLastWhile { it.isEmpty() }
        .toTypedArray()[1]
    val columns = arrayOf(MediaStore.Files.FileColumns.DATA)
    val selection = MediaStore.Files.FileColumns._ID + " = " + id
    val cursor: Cursor? =
      requireContext()
        .contentResolver
        .query(MediaStore.Files.getContentUri("external"), columns, selection, null, null)
    cursor.use { c ->
      val columnIndex = c?.getColumnIndex(columns[0]) ?: 0
      if (c!!.moveToFirst()) {
        path = c.getString(columnIndex)
      }
    }
    return path
  }

  private suspend fun sendAnnounceOTAProviderBtnClick() {
    requireActivity().runOnUiThread {
      val version = 2L
      val versionString = "2.0"

      val path = binding.firmwareFileTv.text.toString()
      val filename = path.substringAfterLast("/")
      Log.d(TAG, "sendAnnounceOTAProviderBtnClick : $filename")
      otaProviderCallback.setOTAFile(version, versionString, filename, path)
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
      ChipClient.VENDOR_ID.toUInt().toInt(),
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

  private fun checkPermissionForReadExternalStorage(): Boolean {
    val result = requireContext().checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
    return result == PackageManager.PERMISSION_GRANTED
  }

  private fun requestPermissionForReadExternalStorage() {
    ActivityCompat.requestPermissions(
      requireActivity(),
      arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
      READ_STORAGE_PERMISSION_REQUEST_CODE
    )
  }

  inner class OtaProviderCallback : OTAProviderDelegate {
    private var fileName: String? = null
    private var version: Long = 0
    private var versionString: String? = null
    private var path: String? = null

    private var fileInputStream: FileInputStream? = null
    private var bufferedInputStream: BufferedInputStream? = null

    fun setOTAFile(version: Long, versionString: String, fileName: String, path: String) {
      this.version = version
      this.versionString = versionString
      this.fileName = fileName
      this.path = path
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
      if (path == null) {
        Log.d(TAG, "path is null")
        return
      }
      try {
        val file = File(path!!)
        fileInputStream = FileInputStream(file)
        bufferedInputStream = BufferedInputStream(fileInputStream)
      } catch (e: IOException) {
        Log.d(TAG, "exception", e)
        fileInputStream?.close()
        bufferedInputStream?.close()
        fileInputStream = null
        bufferedInputStream = null
        return
      }
    }

    override fun handleBDXTransferSessionEnd(errorCode: Long, nodeId: Long) {
      Log.d(TAG, "handleBDXTransferSessionEnd, $errorCode, $nodeId")
      fileInputStream?.close()
      bufferedInputStream?.close()
      fileInputStream = null
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

    private const val READ_STORAGE_PERMISSION_REQUEST_CODE = 41

    private const val APPLY_WAITING_TIME = 10L
    private const val OTA_PROVIDER_ENDPOINT_ID = 0
    private const val OTA_REQUESTER_ENDPOINT_ID = 0
  }
}
