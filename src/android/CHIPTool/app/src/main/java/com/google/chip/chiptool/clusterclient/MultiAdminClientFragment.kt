package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClusterFabricIdEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClusterDeviceIdEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.discriminatorEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.setupPinCodeEd
import kotlinx.android.synthetic.main.multi_admin_client_fragment.multiAdminClusterCommandStatus
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.multiAdminClusterUpdateAddressBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.basicCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.enhancedCommissioningMethodBtn
import kotlinx.android.synthetic.main.multi_admin_client_fragment.view.revokeBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.*
import kotlinx.coroutines.*

class MultiAdminClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.multi_admin_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      multiAdminClusterUpdateAddressBtn.setOnClickListener { updateAddressClick() }
      basicCommissioningMethodBtn.setOnClickListener { scope.launch { sendBasicCommissioningCommandClick() } }
      enhancedCommissioningMethodBtn.setOnClickListener { scope.launch { sendEnhancedCommissioningCommandClick() } }
      revokeBtn.setOnClickListener { scope.launch { sendRevokeCommandClick() } }
    }
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    val testDiscriminator = "3840"
    val testSetupPinCode = 20202021L
    multiAdminClusterFabricIdEd.setText(testFabricId)
    multiAdminClusterDeviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
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

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  private fun updateAddressClick() {
    try{
      deviceController.updateDevice(
              multiAdminClusterFabricIdEd.text.toString().toULong().toLong(),
              multiAdminClusterDeviceIdEd.text.toString().toULong().toLong()
      )
      showMessage("Address update started")
    } catch (ex: Exception) {
      showMessage("Address update failed: $ex")
    }
  }

  private suspend fun sendBasicCommissioningCommandClick() {
    val testDuration = 100
    deviceController.openPairingWindow(multiAdminClusterDeviceIdEd.text.toString().toULong().toLong(), testDuration)
  }

  private suspend fun sendEnhancedCommissioningCommandClick() {
    val testDuration = 100
    val testIteration = 800
    deviceController.openPairingWindowWithPIN(multiAdminClusterDeviceIdEd.text.toString().toULong().toLong(), testDuration, testIteration,
            discriminatorEd.text.toString().toInt(),  setupPinCodeEd.text.toString().toULong().toLong())
  }

  private suspend fun sendRevokeCommandClick() {
    getAdministratorCommissioningClusterForDevice().revokeCommissioning(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Revoke Commissioning success")
      }

      override fun onError(ex: Exception) {
        showMessage("Revoke Commissioning  failure $ex")
        Log.e(TAG, "Revoke Commissioning  failure", ex)
      }
    })
  }

  private suspend fun getAdministratorCommissioningClusterForDevice(): ChipClusters.AdministratorCommissioningCluster {
    return ChipClusters.AdministratorCommissioningCluster(
            ChipClient.getConnectedDevicePointer(requireContext(), multiAdminClusterDeviceIdEd.text.toString().toLong()), 0
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
