package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ICDFragmentBinding
import kotlinx.coroutines.CoroutineScope

class ICDFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: ICDFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ICDFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    val clientInfo = deviceController.icdClientInfo
    var msg = ""

    for (info in clientInfo) {
      msg += "$info\n"
    }
    showMessage(msg)

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.icdTv.text = msg }
  }

  companion object {
    fun newInstance(): ICDFragment = ICDFragment()
  }
}
