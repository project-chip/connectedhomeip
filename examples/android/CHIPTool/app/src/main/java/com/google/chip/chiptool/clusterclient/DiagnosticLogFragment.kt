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
import kotlinx.coroutines.*
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

class DiagnosticLogFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: DiagnosticLogFragmentBinding? = null
  private val binding
    get() = _binding!!

  private val timeout: Int
    get() = binding.timeoutEd.text.toString().toUIntOrNull()?.toInt() ?: 0

  private val diagnosticLogTypeList = DiagnosticLogType.values()
  private val diagnosticLogType: DiagnosticLogType
    get() = diagnosticLogTypeList[binding.diagnosticTypeSp.selectedItemPosition]

  private var mDownloadFile: File? = null
  private var mDownloadFileOutputStream: FileOutputStream? = null

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
      ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, diagnosticLogTypeList)

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  inner class ChipDownloadLogCallback : DownloadLogCallback {
      override fun onError(fabricIndex: Int, nodeId: Long, errorCode: Long) {
        Log.d(TAG, "onError: $fabricIndex, ${nodeId.toULong()}, $errorCode")
      }

      override fun onTransferData(
        fabricIndex: Int,
        nodeId: Long,
        data: ByteArray,
        isEof: Boolean
      ): Boolean {
          Log.d(TAG, "onTransferData : ${data.size}, $isEof")
        if (mDownloadFileOutputStream == null || mDownloadFile == null) {
            Log.d(TAG, "mDownloadFileOutputStream or mDownloadFile is null")
            return false
        }
        try {
            mDownloadFileOutputStream!!.write(data)
            if (isEof) {
                mDownloadFileOutputStream!!.flush()
                showNotification(mDownloadFile!!)
            }
        } catch (e: IOException) {
            Log.d(TAG, "IOException", e)
            return false
        }
        return true
      }
  }

  private fun getDiagnosticLogClick() {
    mDownloadFile = createLogFile(deviceController.fabricIndex.toUInt(), addressUpdateFragment.deviceId.toULong(), diagnosticLogType)
    mDownloadFileOutputStream = FileOutputStream(mDownloadFile)
    deviceController.downloadLogFromNode(addressUpdateFragment.deviceId, diagnosticLogType, timeout, ChipDownloadLogCallback())
  }

  private fun isExternalStorageWritable(): Boolean {
    return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED
  }

  private fun createLogFile(fabricIndex: UInt, nodeId: ULong, type:DiagnosticLogType) : File? {
    if (!isExternalStorageWritable()) {
      return null
    }
    val now = System.currentTimeMillis()
    val fileName = "${type}_${fabricIndex}_${nodeId}_$now.txt"
    return File(requireContext().getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS), fileName)
  }

  private fun showNotification(file: File) {
        val intent = Intent(Intent.ACTION_VIEW).apply {
            setDataAndType(getFileUri(file), "text/plain")
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        }

      requireActivity().startActivity(intent)
    }

    private fun getFileUri(file: File): Uri {
        return FileProvider.getUriForFile(requireContext(), "${requireContext().packageName}.provider", file)
    }

  companion object {
    private const val TAG = "DiagnosticLogFragment"

    fun newInstance(): DiagnosticLogFragment = DiagnosticLogFragment()
  }
}
