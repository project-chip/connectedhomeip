package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.content.Context
import android.net.wifi.WifiManager
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
import com.google.chip.chiptool.util.DeviceIdUtil
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale
import kotlinx.android.synthetic.main.on_off_client_fragment.commandStatusTv
import kotlinx.android.synthetic.main.on_off_client_fragment.deviceIdEd
import kotlinx.android.synthetic.main.on_off_client_fragment.fabricIdEd
import kotlinx.android.synthetic.main.on_off_client_fragment.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.view.cancelReportBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.view.offBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.onBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.readBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.reportBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.toggleBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.updateAddressBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController()

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.on_off_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      updateAddressBtn.setOnClickListener { updateAddressClick() }
      onBtn.setOnClickListener { scope.launch { sendOnCommandClick() } }
      offBtn.setOnClickListener { scope.launch { sendOffCommandClick() } }
      toggleBtn.setOnClickListener { scope.launch { sendToggleCommandClick() } }
      readBtn.setOnClickListener { scope.launch { sendReadOnOffClick() } }
      reportBtn.setOnClickListener { showReportDialog() }
      cancelReportBtn.setOnClickListener { scope.launch { cancelReporting() } }

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

  private suspend fun cancelReporting() {
    val bindingCluster = ChipClusters.BindingCluster(
      ChipClient.getConnectedDevicePointer(
        deviceIdEd.text.toString().toLong()
      ), BINDING_CLUSTER_ENDPOINT
    )
    bindingCluster.unbind(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        Log.v(TAG, "Successfully unbound on/off cluster")
        showMessage("Unbound on/off cluster")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error unbinding on/off cluster", ex)
      }
    }, CONTROLLER_NODE_ID, 0, ON_OFF_CLUSTER_ENDPOINT, OnOffCluster.clusterId())

    // According to spec: max interval set to 0xFFFF cancels reporting.
    getOnOffClusterForDevice().configureOnOffAttribute(object :
                                                         ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        Log.v(TAG, "Successfully configured on/off attribute for reporting cancellation")
        showMessage("Cancelled reporting for on/off cluster")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Failed to configure on/off attribute for reporting cancellation", ex)
      }
    }, 1, REPORT_MAX_INTERVAL_CANCELLATION_VALUE, 1)
  }

  private fun showReportDialog() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.report_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    val changeEd = dialogView.findViewById<EditText>(R.id.changeEd)
    dialogView.findViewById<Button>(R.id.reportBtn).setOnClickListener {
      scope.launch {
        sendReportOnOffClick(
          minIntervalEd.text.toString().toInt(),
          maxIntervalEd.text.toString().toInt(),
          changeEd.text.toString().toInt()
        )
        dialog.dismiss()
      }
    }
    dialog.show()
  }

  private suspend fun sendReportOnOffClick(minInterval: Int, maxInterval: Int, change: Int) {
    val bindingCluster = ChipClusters.BindingCluster(
      ChipClient.getConnectedDevicePointer(
        deviceIdEd.text.toString().toLong()
      ), BINDING_CLUSTER_ENDPOINT
    )
    val onOffCluster = getOnOffClusterForDevice()

    val configureCallback = object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        Log.v(TAG, "Configure on/off success")

        onOffCluster.reportOnOffAttribute(object : ChipClusters.BooleanAttributeCallback {
          override fun onSuccess(on: Boolean) {
            Log.v(TAG, "Report on/off attribute value: $on")

            val formatter = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
            val time = formatter.format(Calendar.getInstance(Locale.getDefault()).time)
            showMessage("Report on/off at $time: ${if (on) "ON" else "OFF"}")
          }

          override fun onError(ex: Exception) {
            Log.e(TAG, "Error reporting on/off attribute", ex)
          }
        })
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error configuring on/off attribute", ex)
      }
    }
    bindingCluster.bind(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        onOffCluster.configureOnOffAttribute(configureCallback, minInterval, maxInterval, change)
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error binding on/off cluster", ex)
      }
    }, CONTROLLER_NODE_ID, 0, ON_OFF_CLUSTER_ENDPOINT, OnOffCluster.clusterId())
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    fabricIdEd.setText(testFabricId)
    deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
    }

    override fun onSendMessageComplete(message: String?) {
      commandStatusTv.text = requireContext().getString(R.string.echo_status_response, message)
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

  private fun updateAddressClick() {
    try {
      deviceController.updateDevice(
        fabricIdEd.text.toString().toULong().toLong(),
        deviceIdEd.text.toString().toULong().toLong()
      )
      showMessage("Address update started")
    } catch (ex: Exception) {
      showMessage("Address update failed: $ex")
    }
  }

  private suspend fun sendLevelCommandClick() {
    val cluster = ChipClusters.LevelControlCluster(
      ChipClient.getConnectedDevicePointer(deviceIdEd.text.toString().toLong()),
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
      ChipClient.getConnectedDevicePointer(deviceIdEd.text.toString().toLong()),
      ON_OFF_CLUSTER_ENDPOINT
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      commandStatusTv.text = msg
    }
  }

  companion object {
    private const val TAG = "OnOffClientFragment"

    private const val ON_OFF_CLUSTER_ENDPOINT = 1
    private const val LEVEL_CONTROL_CLUSTER_ENDPOINT = 1
    private const val BINDING_CLUSTER_ENDPOINT = 1
    private const val CONTROLLER_NODE_ID = 112233L
    private const val REPORT_MAX_INTERVAL_CANCELLATION_VALUE = 0xFFFF

    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
