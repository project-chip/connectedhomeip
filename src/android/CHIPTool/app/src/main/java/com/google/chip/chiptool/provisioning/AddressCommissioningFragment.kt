package com.google.chip.chiptool.provisioning

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.address_commissioning_fragment.addressEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.commissionBtn
import kotlinx.android.synthetic.main.address_commissioning_fragment.discriminatorEditText
import kotlinx.android.synthetic.main.address_commissioning_fragment.pincodeEditText

class AddressCommissioningFragment : Fragment() {
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
  }

  companion object {
    private const val TAG = "AddressCommissioningFragment"

    fun newInstance(): AddressCommissioningFragment = AddressCommissioningFragment()
  }
}