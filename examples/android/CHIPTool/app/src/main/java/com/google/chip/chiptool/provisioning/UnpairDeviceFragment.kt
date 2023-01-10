package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.UnpairDeviceCallback
import com.google.chip.chiptool.ChipClient
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
      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

      unpairDeviceBtn.setOnClickListener { scope.launch { unpairDeviceClick() } }
    }
  }

  inner class ChipUnpairDeviceCallback : UnpairDeviceCallback {
    override fun onError(status: Int, remoteDeviceId: Long) {
      Log.d(TAG, "onError : $remoteDeviceId, $status")
    }

    override fun onSuccess(remoteDeviceId: Long) {
      Log.d(TAG, "onSuccess : $remoteDeviceId")
    }
  }

  private  fun unpairDeviceClick() {
    deviceController.unpairDeviceCallback(addressUpdateFragment.deviceId, ChipUnpairDeviceCallback())
  }


  companion object {
    private const val TAG = "UnpairDeviceFragment"
    fun newInstance(): UnpairDeviceFragment = UnpairDeviceFragment()
  }
}
