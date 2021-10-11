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
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.OnOffCluster
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale
import kotlinx.android.synthetic.main.on_off_client_fragment.commandStatusTv
import kotlinx.android.synthetic.main.on_off_client_fragment.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.reportStatusTv
import kotlinx.android.synthetic.main.on_off_client_fragment.view.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.view.offBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.onBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.readBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.showSubscribeDialogBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.toggleBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.on_off_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

      onBtn.setOnClickListener { scope.launch { sendOnCommandClick() } }
      offBtn.setOnClickListener { scope.launch { sendOffCommandClick() } }
      toggleBtn.setOnClickListener { scope.launch { sendToggleCommandClick() } }
      readBtn.setOnClickListener { scope.launch { sendReadOnOffClick() } }
      showSubscribeDialogBtn.setOnClickListener { showSubscribeDialog() }

      levelBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
        override fun onProgressChanged(seekBar: SeekBar, i: Int, b: Boolean) {

        }

        override fun onStartTrackingTouch(seekBar: SeekBar?) {
        }

        override fun onStopTrackingTouch(seekBar: SeekBar?) {
          Toast.makeText(
            requireContext(),
            "Level is: " + levelBar.progress,
            Toast.LENGTH_SHORT
          ).show()
          scope.launch { sendLevelCommandClick() }
        }
      })
    }
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
        dialog.dismiss()
      }
    }
    dialog.show()
  }

  private suspend fun sendSubscribeOnOffClick(minInterval: Int, maxInterval: Int) {
    val onOffCluster = getOnOffClusterForDevice()

    val subscribeCallback = object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        val message = "Subscribe on/off success"
        Log.v(TAG, message)
        showMessage(message)

        onOffCluster.reportOnOffAttribute(object : ChipClusters.BooleanAttributeCallback {
          override fun onSuccess(on: Boolean) {
            Log.v(TAG, "Report on/off attribute value: $on")

            val formatter = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
            val time = formatter.format(Calendar.getInstance(Locale.getDefault()).time)
            showReportMessage("Report on/off at $time: ${if (on) "ON" else "OFF"}")
          }

          override fun onError(ex: Exception) {
            Log.e(TAG, "Error reporting on/off attribute", ex)
            showReportMessage("Error reporting on/off attribute: $ex")
          }
        })
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

  override fun onStop() {
    super.onStop()
    scope.cancel()
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

    }, levelBar.progress, 0, 0, 0)
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
      commandStatusTv.text = msg
    }
  }

  private fun showReportMessage(msg: String) {
    requireActivity().runOnUiThread {
      reportStatusTv.text = msg
    }
  }

  companion object {
    private const val TAG = "OnOffClientFragment"

    private const val ON_OFF_CLUSTER_ENDPOINT = 1
    private const val LEVEL_CONTROL_CLUSTER_ENDPOINT = 1

    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
