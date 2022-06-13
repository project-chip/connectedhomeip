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
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.multi_admin_client_fragment.discriminatorEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClusterCommandStatus
import kotlinx.android.synthetic.main.multi_admin_client_fragment.setupPinCodeEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.basicCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.enhancedCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.revokeBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.*
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
    discriminatorEd.setText(testDiscriminator)
    setupPinCodeEd.setText(testSetupPinCode.toString())
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
    val testDuration = 100
    deviceController.openPairingWindow(
      ChipClient.getConnectedDevicePointer(
        requireContext(),
        addressUpdateFragment.deviceId
      ), testDuration
    )
  }

  private suspend fun sendEnhancedCommissioningCommandClick() {
    val testDuration = 100
    val testIteration = 1000
    val devicePointer =
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
    deviceController.openPairingWindowWithPIN(
      devicePointer, testDuration, testIteration.toLong(),
      discriminatorEd.text.toString().toInt(), setupPinCodeEd.text.toString().toULong().toLong()
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
