package com.google.chip.chiptool.clusterclient

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.OpenableColumns
import android.provider.Settings
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.app.ActivityCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.OTAProviderDelegate
import chip.devicecontroller.model.InvokeElement
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OtaProviderClientFragmentBinding
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import java.io.BufferedInputStream
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvWriter

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
        binding.announceOTAProviderBtn.setOnClickListener { scope.launch { sendAnnounceOTAProviderBtnClick() } }

        deviceController.setOTAProviderDelegate(otaProviderCallback)

        return binding.root
    }

    override fun onStart() {
        super.onStart()
        if (Build.VERSION.SDK_INT >= 30 && !Environment.isExternalStorageManager()) {
           Toast.makeText(requireContext(), "Require to Allow management of all files permission", Toast.LENGTH_LONG).show()
           startActivity(Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION).apply {
               addCategory("android.intent.category.DEFAULT")
               data = Uri.parse(String.format("package:%s", requireContext().packageName))
           })
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

        val filename = intent.data?.let {
            returnUri -> requireContext().contentResolver.query(returnUri, null, null, null, null)
        }?.use { cursor ->
            val nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
            val sizeIndex = cursor.getColumnIndex(OpenableColumns.SIZE)
            cursor.moveToFirst()
            val name = cursor.getString(nameIndex)
            val size = cursor.getLong(sizeIndex).toString()

            Log.d(TAG, "$name, $size")
            name
        }
        Log.d(TAG, "onActivityResult : $filename")
        Log.d(TAG, intent.toString())

        requireActivity().runOnUiThread {
            binding.firmwareFileTv.text = filename
        }
    }

    private suspend fun sendAnnounceOTAProviderBtnClick() {
        val endpointId = 0
        val clusterId = ClusterIDMapping.OtaSoftwareUpdateRequestor.ID
        val commandId = ClusterIDMapping.OtaSoftwareUpdateRequestor.Command.AnnounceOTAProvider

        val tlvWriter = TlvWriter().apply {
            startStructure(AnonymousTag)
            put(ContextSpecificTag(ClusterIDMapping.OtaSoftwareUpdateRequestor.AnnounceOTAProviderCommandField.ProviderNodeID.id), deviceController.controllerNodeId.toULong())
            put(ContextSpecificTag(ClusterIDMapping.OtaSoftwareUpdateRequestor.AnnounceOTAProviderCommandField.VendorID.id), ChipClient.VENDOR_ID.toUInt())
            put(ContextSpecificTag(ClusterIDMapping.OtaSoftwareUpdateRequestor.AnnounceOTAProviderCommandField.AnnouncementReason.id), 0U)
            put(ContextSpecificTag(ClusterIDMapping.OtaSoftwareUpdateRequestor.AnnounceOTAProviderCommandField.Endpoint.id), 0U)
            endStructure()
        }

        val invokeElement = InvokeElement.newInstance(endpointId, clusterId, commandId.id, tlvWriter.getEncoded(), null)

        requireActivity().runOnUiThread {
            val filename = binding.firmwareFileTv.text.toString()
            otaProviderCallback.setOTAFile(2, "2.0", filename, Environment.getExternalStorageDirectory().path + "/Download/$filename")
        }

        deviceController.invoke(
                object : InvokeCallback {
                    override fun onError(ex: Exception?) {
                        showMessage("${commandId.name} command failure $ex")
                        Log.e(TAG, "${commandId.name} command failure", ex)
                    }

                    override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
                        Log.d(TAG, "onResponse : $invokeElement, Code : $successCode")
                        showMessage("${commandId.name} command success")
                    }
                },
                getConnectedDevicePointer(),
                invokeElement,
                0,
                0
        )
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    fun checkPermissionForReadExternalStorage(): Boolean {
        val result = requireContext().checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
        return result == PackageManager.PERMISSION_GRANTED
    }

    fun requestPermissionForReadExternalStorage() {
        try {
            ActivityCompat.requestPermissions(requireActivity(), arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                    READ_STORAGE_PERMISSION_REQUEST_CODE)
        } catch (e: Exception) {
            e.printStackTrace()
            return
        }
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

        override fun handleQueryImage(vendorId: Int, productId: Int, softwareVersion: Long, hardwareVersion: Int?, location: String?, requestorCanConsent: Boolean?, metadataForProvider: ByteArray?): OTAProviderDelegate.QueryImageResponse {
            Log.d(TAG, "handleQueryImage, $vendorId, $productId, $softwareVersion, $hardwareVersion, $location")
            return OTAProviderDelegate.QueryImageResponse(version, versionString, fileName)
        }

        override fun handleOTAQueryFailure(error: Int) {
            Log.d(TAG, "handleOTAQueryFailure, $error")
        }

        override fun handleApplyUpdateRequest(nodeId: Long, newVersion: Long): OTAProviderDelegate.ApplyUpdateResponse {
            Log.d(TAG, "handleApplyUpdateRequest, $nodeId, $newVersion")
            return OTAProviderDelegate.ApplyUpdateResponse(OTAProviderDelegate.ApplyUpdateActionEnum.Proceed, 0)
        }

        override fun handleNotifyUpdateApplied(nodeId: Long) {
            Log.d(TAG, "handleNotifyUpdateApplied, $nodeId")
        }

        override fun handleBDXTransferSessionBegin(nodeId: Long, fileDesignator: String?, offset: Long) {
            Log.d(TAG, "handleBDXTransferSessionBegin, $nodeId, $fileDesignator, $offset")
            showMessage("BDXTransferSessionBegin : $nodeId")
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
                return
            }
        }

        override fun handleBDXTransferSessionEnd(errorCode: Long, nodeId: Long) {
            Log.d(TAG, "handleBDXTransferSessionEnd, $errorCode, $nodeId")
            showMessage("BDXTransferSessionEnd : $nodeId, $errorCode")
            if (bufferedInputStream != null) {
                bufferedInputStream!!.close()
                bufferedInputStream = null
            }
            if (fileInputStream != null) {
                fileInputStream!!.close()
                fileInputStream = null
            }
        }

        override fun handleBDXQuery(nodeId: Long, blockSize: Int, blockIndex: Long, bytesToSkip: Long): OTAProviderDelegate.BDXData? {
            Log.d(TAG, "handleBDXQuery, $nodeId, $blockSize, $blockIndex, $bytesToSkip")
            showMessage("sending.. $blockIndex")
            if (bufferedInputStream == null) {
                return null
            }
            val packet = ByteArray(blockSize)
            val len = bufferedInputStream!!.read(packet)

            val sendPacket = if (len < blockSize) {
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
        override fun onConnectDeviceComplete() {}

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

    private suspend fun getConnectedDevicePointer(): Long {
        return ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
    }

    private fun showMessage(msg: String) {
        requireActivity().runOnUiThread { binding.commandStatusTv.text = msg }
    }

    companion object {
        private const val TAG = "OtaProviderClientFragment"

        fun newInstance(): OtaProviderClientFragment = OtaProviderClientFragment()

        private const val READ_STORAGE_PERMISSION_REQUEST_CODE = 41
    }
}