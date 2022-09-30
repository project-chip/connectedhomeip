package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.OpenCommissioningCallback
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.multi_admin_client_fragment.discriminatorEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.timeoutEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClusterCommandStatus
import kotlinx.android.synthetic.main.multi_admin_client_fragment.setupPinCodeEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.basicCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.enhancedCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.revokeBtn
import kotlinx.coroutines.*

class MultiAdminClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    scope = viewLifecycleOwner.lifecycleScope

    return inflater.inflate(R.layout.multi_admin_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

      basicCommissioningMethodBtn.setOnClickListener { scope.launch { sendBasicCommissioningCommandClick() } }
      enhancedCommissioningMethodBtn.setOnClickListener { scope.launch { sendEnhancedCommissioningCommandClick() } }
      revokeBtn.setOnClickListener { scope.launch { sendRevokeCommandClick() } }
    }
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the discriminator and setupPinCode that was used to commission the device
    val testDiscriminator = "3840"
    val testSetupPinCode = 20202021L
    val testDuration = 180
    discriminatorEd.setText(testDiscriminator)
    setupPinCodeEd.setText(testSetupPinCode.toString())
    timeoutEd.setText(testDuration.toString())
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
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

  private suspend fun sendBasicCommissioningCommandClick() {
    val testDuration = timeoutEd.text.toString().toInt()
    deviceController.openPairingWindowCallback(
      ChipClient.getConnectedDevicePointer(
        requireContext(),
        addressUpdateFragment.deviceId
      ), testDuration,
      object:OpenCommissioningCallback {
        override fun onError(status: Int, deviceId: Long) {
          showMessage("OpenBasicCommissioning Fail! \nDevice ID : $deviceId\nErrorCode : $status")
        }

        override fun onSuccess(deviceId: Long, manualPairingCode: String?, qrCode: String?) {
          showMessage("OpenBasicCommissioning Success! \n Node ID: $deviceId")
        }
      }
    )
  }

  private suspend fun sendEnhancedCommissioningCommandClick() {
    val testDuration = timeoutEd.text.toString().toInt()
    val testIteration = 1000
    val devicePointer =
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
    deviceController.openPairingWindowWithPINCallback(
      devicePointer, testDuration, testIteration.toLong(),
      discriminatorEd.text.toString().toInt(), setupPinCodeEd.text.toString().toULong().toLong(),
      object:OpenCommissioningCallback {
        override fun onError(status: Int, deviceId: Long) {
          showMessage("OpenCommissioning Fail! \nDevice ID : $deviceId\nErrorCode : $status")
        }

        override fun onSuccess(deviceId: Long, manualPairingCode: String?, qrCode: String?) {
          showMessage("OpenCommissioning Success! \n Node ID: $deviceId\n\tManual : $manualPairingCode\n\tQRCode : $qrCode")
        }
      }
    )
  }

  private suspend fun sendRevokeCommandClick() {
    val timedInvokeTimeout = 10000
    getAdministratorCommissioningClusterForDevice().revokeCommissioning(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Revoke Commissioning success")
      }

      override fun onError(ex: Exception) {
        showMessage("Revoke Commissioning  failure $ex")
        Log.e(TAG, "Revoke Commissioning  failure", ex)
      }
    }, timedInvokeTimeout)
  }

  private suspend fun getAdministratorCommissioningClusterForDevice(): ChipClusters.AdministratorCommissioningCluster {
    return ChipClusters.AdministratorCommissioningCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId), 0
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      multiAdminClusterCommandStatus.text = msg
    }
  }

  companion object {
    private const val TAG = "MultiAdminClientFragment"
    fun newInstance(): MultiAdminClientFragment = MultiAdminClientFragment()
  }
}
