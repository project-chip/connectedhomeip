package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.SeekBar
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.OnOffCluster
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OnOffClientFragmentBinding
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OnOffClientFragmentBinding? = null
  private val binding get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = OnOffClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.onBtn.setOnClickListener { scope.launch { sendOnCommandClick() } }
    binding.offBtn.setOnClickListener { scope.launch { sendOffCommandClick() } }
    binding.toggleBtn.setOnClickListener { scope.launch { sendToggleCommandClick() } }
    binding.readBtn.setOnClickListener { scope.launch { sendReadOnOffClick() } }
    binding.showSubscribeDialogBtn.setOnClickListener { showSubscribeDialog() }

    binding.levelBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
      override fun onProgressChanged(seekBar: SeekBar, i: Int, b: Boolean) {

      }

      override fun onStartTrackingTouch(seekBar: SeekBar?) {
      }

      override fun onStopTrackingTouch(seekBar: SeekBar?) {
        Toast.makeText(
          requireContext(),
          "Level is: " + binding.levelBar.progress,
          Toast.LENGTH_SHORT
        ).show()
        scope.launch { sendLevelCommandClick() }
      }
    })

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private suspend fun sendReadOnOffClick() {
    getOnOffClusterForDevice().readOnOffAttribute(object : ChipClusters.BooleanAttributeCallback {
      override fun onSuccess(on: Boolean) {
        Log.v(TAG, "On/Off attribute value: $on")
        showMessage("On/Off attribute value: $on")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error reading onOff attribute", ex)
      }
    })
  }

  private fun showSubscribeDialog() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        sendSubscribeOnOffClick(
          minIntervalEd.text.toString().toInt(),
          maxIntervalEd.text.toString().toInt()
        )
        requireActivity().runOnUiThread {
          dialog.dismiss()
        }
      }
    }
    dialog.show()
  }

  private suspend fun sendSubscribeOnOffClick(minInterval: Int, maxInterval: Int) {
    val onOffCluster = getOnOffClusterForDevice()

    val subscribeCallback = object : ChipClusters.BooleanAttributeCallback {
      override fun onSuccess(value: Boolean) {
        val formatter = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
        val time = formatter.format(Calendar.getInstance(Locale.getDefault()).time)
        val message = "Subscribed on/off value at $time: ${if (value) "ON" else "OFF"}"

        Log.v(TAG, message)
        showReportMessage(message)
      }

      override fun onSubscriptionEstablished() {
        val message = "Subscription for on/off established"
        Log.v(TAG, message)
        showMessage(message)
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error configuring on/off attribute", ex)
      }
    }
    onOffCluster.subscribeOnOffAttribute(subscribeCallback, minInterval, maxInterval)
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

  private suspend fun sendLevelCommandClick() {
    val cluster = ChipClusters.LevelControlCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      LEVEL_CONTROL_CLUSTER_ENDPOINT
    )
    cluster.moveToLevel(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("MoveToLevel command success")
      }

      override fun onError(ex: Exception) {
        showMessage("MoveToLevel command failure $ex")
        Log.e(TAG, "MoveToLevel command failure", ex)
      }

    }, binding.levelBar.progress, 0, 0, 0)
  }

  private suspend fun sendOnCommandClick() {
    getOnOffClusterForDevice().on(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("ON command success")
      }

      override fun onError(ex: Exception) {
        showMessage("ON command failure $ex")
        Log.e(TAG, "ON command failure", ex)
      }

    })
  }

  private suspend fun sendOffCommandClick() {
    getOnOffClusterForDevice().off(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("OFF command success")
      }

      override fun onError(ex: Exception) {
        showMessage("OFF command failure $ex")
        Log.e(TAG, "OFF command failure", ex)
      }
    })
  }

  private suspend fun sendToggleCommandClick() {
    getOnOffClusterForDevice().toggle(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("TOGGLE command success")
      }

      override fun onError(ex: Exception) {
        showMessage("TOGGLE command failure $ex")
        Log.e(TAG, "TOGGLE command failure", ex)
      }
    })
  }

  private suspend fun getOnOffClusterForDevice(): OnOffCluster {
    return OnOffCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId),
      ON_OFF_CLUSTER_ENDPOINT
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      binding.commandStatusTv.text = msg
    }
  }

  private fun showReportMessage(msg: String) {
    requireActivity().runOnUiThread {
      binding.reportStatusTv.text = msg
    }
  }

  companion object {
    private const val TAG = "OnOffClientFragment"

    private const val ON_OFF_CLUSTER_ENDPOINT = 1
    private const val LEVEL_CONTROL_CLUSTER_ENDPOINT = 1

    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
