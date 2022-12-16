package com.google.chip.chiptool.provisioning

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
import com.google.chip.chiptool.clusterclient.AddressUpdateFragment
import kotlinx.android.synthetic.main.unpair_device_fragment.view.unpairDeviceBtn
import kotlinx.coroutines.*

class UnpairDeviceFragment : Fragment() {
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

    return inflater.inflate(R.layout.unpair_device_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

      unpairDeviceBtn.setOnClickListener { scope.launch { unpairDeviceClick() } }
    }
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onPairingDeleted(code: Int) {
      Log.d(TAG, "onPairingDeleted : $code")
    }
  }

  private  fun unpairDeviceClick() {
    deviceController.unpairDevice(addressUpdateFragment.deviceId)
  }


  companion object {
    private const val TAG = "UnpairDeviceFragment"
    fun newInstance(): UnpairDeviceFragment = UnpairDeviceFragment()
  }
}
