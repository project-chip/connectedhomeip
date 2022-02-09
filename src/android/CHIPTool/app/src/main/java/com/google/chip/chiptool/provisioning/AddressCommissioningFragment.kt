package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.address_commissioning_fragment.addressEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.commissionBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.discoverBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.discoverListSpinner
import kotlinx.android.synthetic.main.address_commissioning_fragment.discriminatorEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.pincodeEditText
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class AddressCommissioningFragment : Fragment() {
  private val ipAddressList = ArrayList<String>()
  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    return inflater.inflate(R.layout.address_commissioning_fragment, container, false)
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    commissionBtn.setOnClickListener {
      val address = addressEditText.text.toString()
      val discriminator = discriminatorEditText.text.toString()
      val pincode = pincodeEditText.text.toString()

      if (address.isEmpty() || discriminator.isEmpty() || pincode.isEmpty()) {
        Log.e(TAG, "Address, discriminator, or pincode was empty: $address $discriminator $pincode")
        return@setOnClickListener
      }

      FragmentUtil.getHost(this, BarcodeFragment.Callback::class.java)?.onCHIPDeviceInfoReceived(
        CHIPDeviceInfo(
          discriminator = discriminator.toInt(),
          setupPinCode = pincode.toLong(),
          ipAddress = address
        )
      )
    }

    discoverBtn.setOnClickListener { _ ->
      discoverBtn.isEnabled = false
      val deviceController = ChipClient.getDeviceController(requireContext())
      deviceController.discoverCommissionableNodes()
      scope.launch {
        delay(7000)
        updateSpinner()
        discoverBtn.isEnabled = true
      }
    }
  }

  private fun updateSpinner() {
    val deviceController = ChipClient.getDeviceController(requireContext())
    for(i in 0..10) {
      val device = deviceController.getDiscoveredDevice(i) ?: break
      ipAddressList.add("${device.ipAddress}, ${device.discriminator}")
    }
    requireActivity().runOnUiThread {
      discoverListSpinner.adapter =
        ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, ipAddressList)
      discoverListSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>, view: View, position: Int, id: Long) {
          val address = ipAddressList[position].split(",")[0].trim()
          val discriminator = ipAddressList[position].split(",")[1].trim()
          addressEditText.setText(address)
          discriminatorEditText.setText(discriminator)
        }

        override fun onNothingSelected(parent: AdapterView<*>) {}
      }
    }
  }

  companion object {
    private const val TAG = "AddressCommissioningFragment"

    fun newInstance(): AddressCommissioningFragment = AddressCommissioningFragment()
  }
}