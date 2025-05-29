package com.google.chip.chiptool.clusterclient

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import androidx.core.content.FileProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.DiagnosticLogType
import chip.devicecontroller.DownloadLogCallback
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.DiagnosticLogFragmentBinding
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class DiagnosticLogFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: DiagnosticLogFragmentBinding? = null
  private val binding
    get() = _binding!!

  private val timeout: Long
    get() = binding.timeoutEd.text.toString().toULongOrNull()?.toLong() ?: 0L

  private val diagnosticLogTypeList = DiagnosticLogType.values()
  private val diagnosticLogType: DiagnosticLogType
    get() = diagnosticLogTypeList[binding.diagnosticTypeSp.selectedItemPosition]

  private var mDownloadFile: File? = null
  private var mDownloadFileOutputStream: FileOutputStream? = null

  private var mReceiveFileLen = 0U

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = DiagnosticLogFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.getDiagnosticLogBtn.setOnClickListener { scope.launch { getDiagnosticLogClick() } }

    binding.diagnosticTypeSp.adapter =
      ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        diagnosticLogTypeList
      )

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  inner class ChipDownloadLogCallback : DownloadLogCallback {
    override fun onError(fabricIndex: Int, nodeId: Long, errorCode: Long) {
      Log.d(TAG, "onError: $fabricIndex, ${nodeId.toULong()}, $errorCode")
      showMessage("Downloading Failed")
      mDownloadFileOutputStream?.flush() ?: return
    }

    override fun onSuccess(fabricIndex: Int, nodeId: Long) {
      Log.d(TAG, "onSuccess: $fabricIndex, ${nodeId.toULong()}")
      mDownloadFileOutputStream?.flush() ?: return
      showMessage("Downloading Completed")
      mDownloadFile?.let { showNotification(it) } ?: return
    }

    override fun onTransferData(fabricIndex: Int, nodeId: Long, data: ByteArray): Boolean {
      Log.d(TAG, "onTransferData : ${data.size}")
      if (mDownloadFileOutputStream == null) {
        Log.d(TAG, "mDownloadFileOutputStream or mDownloadFile is null")
        return false
      }
      return addData(mDownloadFileOutputStream!!, data)
    }

    private fun addData(outputStream: FileOutputStream, data: ByteArray): Boolean {
      try {
        outputStream.write(data)
      } catch (e: IOException) {
        Log.d(TAG, "IOException", e)
        return false
      }
      mReceiveFileLen += data.size.toUInt()
      showMessage("Receive Data Size : $mReceiveFileLen")
      return true
    }
  }

  private fun getDiagnosticLogClick() {
    mDownloadFile =
      createLogFile(
        deviceController.fabricIndex.toUInt(),
        addressUpdateFragment.deviceId.toULong(),
        diagnosticLogType
      )
    mDownloadFileOutputStream = FileOutputStream(mDownloadFile)
    deviceController.downloadLogFromNode(
      addressUpdateFragment.deviceId,
      diagnosticLogType,
      timeout,
      ChipDownloadLogCallback()
    )
  }

  private fun isExternalStorageWritable(): Boolean {
    return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED
  }

  private fun createLogFile(fabricIndex: UInt, nodeId: ULong, type: DiagnosticLogType): File? {
    if (!isExternalStorageWritable()) {
      return null
    }
    val now = System.currentTimeMillis()
    val fileName = "${type}_${fabricIndex}_${nodeId}_$now.txt"
    mReceiveFileLen = 0U
    return File(requireContext().getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS), fileName)
  }

  private fun showNotification(file: File) {
    val intent =
      Intent(Intent.ACTION_VIEW).apply {
        setDataAndType(getFileUri(file), "text/plain")
        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
      }

    requireActivity().startActivity(intent)
  }

  private fun getFileUri(file: File): Uri {
    return FileProvider.getUriForFile(
      requireContext(),
      "${requireContext().packageName}.provider",
      file
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.diagnosticLogTv.text = msg }
  }

  companion object {
    private const val TAG = "DiagnosticLogFragment"

    fun newInstance(): DiagnosticLogFragment = DiagnosticLogFragment()
  }
}
